#include "TransactionManager.h"

#include "ConnectionManager.h"
#include "OrbitBase/Logging.h"
#include "TcpClient.h"
#include "TcpServer.h"

namespace orbit {

TransactionManager::TransactionManager() {
  on_request_ = [this](const Message& msg) { HandleRequest(msg); };
  on_response_ = [this](const Message& msg) { HandleResponse(msg); };
}

TransactionManager& TransactionManager::Get() {
  static TransactionManager manager;
  return manager;
}

void TransactionManager::RegisterTransactionHandler(
    const TransactionHandler& handler) {
  MessageType type = handler.GetType();
  CHECK(!HasHandler(type));
  absl::MutexLock lock(&mutex_);
  transaction_handlers_[type] = std::make_shared<TransactionHandler>(handler);
  if (GTcpClient) GTcpClient->AddMainThreadCallback(type, on_response_);
  if (GTcpServer) GTcpServer->AddMainThreadCallback(type, on_request_);
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

void TransactionManager::EnqueueRequestInternal(MessageType type,
                                                std::string&& object) {
  absl::MutexLock lock(&mutex_);
  auto transaction = std::make_shared<Transaction>();
  transaction->message_type = type;
  transaction->payload = std::move(object);
  transaction_queue_.push(transaction);
}

void TransactionManager::InitiateTransaction(
    std::shared_ptr<Transaction> transaction) {
  transaction->start_time = OrbitTicks();
  SendRequestInternal(transaction->message_type, transaction->payload);
}

void TransactionManager::SendRequestInternal(MessageType type,
                                             const std::string& object) {
  CHECK(ConnectionManager::Get().IsClient());
  std::shared_ptr<TransactionHandler> handler = GetHandler(type);
  const char* desc = handler->description.c_str();
  LOG("Sending transaction request: %s [%lu bytes]", desc, object.size());
  GTcpClient->Send(type, object);
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
  GTcpServer->Send(type, object);
}

void TransactionManager::ReceiveResponseInternal(const Message& message) {
  CHECK(ConnectionManager::Get().IsClient());
  std::shared_ptr<TransactionHandler> handler = GetHandler(message.GetType());
  const char* desc = handler->description.c_str();
  LOG("Receiving transaction response: %s [%u bytes]", desc, message.GetSize());
}

void TransactionManager::OnTransactionCompleted(
    std::shared_ptr<Transaction> transaction) {
  MessageType type = transaction->message_type;
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
  GetHandler(message.GetType())->response_handler(message);
  current_transaction_->end_time = OrbitTicks();
  current_transaction_->completed = true;
}

}  // namespace orbit
