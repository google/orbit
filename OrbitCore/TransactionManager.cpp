#include "TransactionManager.h"

#include "ConnectionManager.h"
#include "OrbitBase/Logging.h"
#include "TcpClient.h"
#include "TcpServer.h"

namespace orbit {

TransactionManager::TransactionManager(TcpClient* client, TcpServer* server) {
  client_ = client;
  server_ = server;
  on_request_ = [this](const Message& msg) { HandleRequest(msg); };
  on_response_ = [this](const Message& msg) { HandleResponse(msg); };
}

void TransactionManager::RegisterTransactionHandler(
    const TransactionHandler& handler) {
  CHECK(!HasHandler(handler.type));
  absl::MutexLock lock(&mutex_);
  MessageType type = handler.type;
  transaction_handlers_[type] = std::make_shared<TransactionHandler>(handler);
  if (server_) server_->AddMainThreadCallback(handler.type, on_request_);
  if (client_) client_->AddMainThreadCallback(handler.type, on_response_);
}

void TransactionManager::Tick() {
  if (current_transaction_ == nullptr) {
    if (!IsTransactionQueueEmpty()) {
      current_transaction_ = PopTransaction();
      InitiateTransaction(current_transaction_);
    }
  } else if (current_transaction_->completed) {
    OnTransactionCompleted(current_transaction_);
    current_transaction_ = nullptr;
  }
}

bool TransactionManager::IsTransactionQueueEmpty() const {
  absl::MutexLock lock(&mutex_);
  return transaction_queue_.empty();
};

std::shared_ptr<Transaction> TransactionManager::PopTransaction() {
  absl::MutexLock lock(&mutex_);
  std::shared_ptr<Transaction> transaction = transaction_queue_.front();
  transaction_queue_.pop();
  return transaction;
}

uint32_t TransactionManager::EnqueueRequestInternal(MessageType type,
                                                    std::string&& object) {
  absl::MutexLock lock(&mutex_);
  auto transaction = std::make_shared<Transaction>();
  transaction->type = type;
  transaction->payload = std::move(object);
  transaction->id = request_counter_++;
  transaction_queue_.push(transaction);
  return transaction->id;
}

void TransactionManager::InitiateTransaction(
    std::shared_ptr<Transaction> transaction) {
  transaction->start_time = OrbitTicks();
  SendRequestInternal(transaction->type, transaction->payload);
}

void TransactionManager::SendRequestInternal(MessageType type,
                                             const std::string& object) {
  CHECK(ConnectionManager::Get().IsClient());
  std::shared_ptr<TransactionHandler> handler = GetHandler(type);
  const char* desc = handler->description.c_str();
  LOG("Sending transaction request: %s [%lu bytes]", desc, object.size());
  client_->Send(type, object);
}

void TransactionManager::ReceiveRequestInternal(const Message& message) {
  CHECK(ConnectionManager::Get().IsService());
  std::shared_ptr<TransactionHandler> handler = GetHandler(message.GetType());
  const char* desc = handler->description.c_str();
  LOG("Receiving transaction request: %s [%u bytes]", desc, message.GetSize());
}

void TransactionManager::SendResponseInternal(MessageType type,
                                              const std::string& object) {
  CHECK(ConnectionManager::Get().IsService());
  std::shared_ptr<TransactionHandler> handler = GetHandler(type);
  const char* desc = handler->description.c_str();
  LOG("Sending transaction response: %s [%lu bytes]", desc, object.size());
  server_->Send(type, object);
}

void TransactionManager::ReceiveResponseInternal(const Message& message) {
  CHECK(ConnectionManager::Get().IsClient());
  std::shared_ptr<TransactionHandler> handler = GetHandler(message.GetType());
  const char* desc = handler->description.c_str();
  LOG("Receiving transaction response: %s [%u bytes]", desc, message.GetSize());
}

void TransactionManager::OnTransactionCompleted(
    std::shared_ptr<Transaction> transaction) {
  MessageType type = transaction->type;
  std::shared_ptr<TransactionHandler> handler = GetHandler(type);
  LOG("Transaction %s complete.", handler->description.c_str());
}

bool TransactionManager::HasHandler(MessageType type) const {
  absl::MutexLock lock(&mutex_);
  return transaction_handlers_.find(type) != transaction_handlers_.end();
}

std::shared_ptr<TransactionHandler> TransactionManager::GetHandler(
    MessageType type) {
  CHECK(HasHandler(type));
  absl::MutexLock lock(&mutex_);
  return transaction_handlers_[type];
}

void TransactionManager::HandleRequest(const Message& message) {
  CHECK(ConnectionManager::Get().IsService());
  GetHandler(message.GetType())->request_handler(message);
}

void TransactionManager::HandleResponse(const Message& message) {
  CHECK(ConnectionManager::Get().IsClient());
  CHECK(current_transaction_);
  uint32_t id = current_transaction_->id;
  GetHandler(message.GetType())->response_handler(message, id);
  current_transaction_->end_time = OrbitTicks();
  current_transaction_->completed = true;
}

}  // namespace orbit
