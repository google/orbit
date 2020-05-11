// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TransactionClient.h"

#include <OrbitBase/Logging.h>

#include "Profiling.h"

TransactionClient::TransactionClient(TcpClient* client) : client_{client} {
  CHECK(client_ != nullptr);
}

void TransactionClient::RegisterTransactionResponseHandler(
    const TransactionResponseHandler& handler) {
  CHECK(!HasResponseHandler(handler.type));
  absl::MutexLock lock(&mutex_);
  transaction_response_handlers_[handler.type] = handler;
  client_->AddMainThreadCallback(
      handler.type, [this](const Message& msg) { HandleResponse(msg); });
}

void TransactionClient::Tick() {
  if (current_transaction_ == nullptr) {
    if (!IsTransactionQueueEmpty()) {
      current_transaction_ = PopTransaction();
      InitiateTransaction(current_transaction_.get());
    }
  } else if (current_transaction_->completed) {
    OnTransactionCompleted(current_transaction_.get());
    current_transaction_ = nullptr;
  }
}

bool TransactionClient::IsTransactionQueueEmpty() {
  absl::MutexLock lock(&mutex_);
  return transaction_queue_.empty();
}

std::shared_ptr<Transaction> TransactionClient::PopTransaction() {
  absl::MutexLock lock(&mutex_);
  std::shared_ptr<Transaction> transaction = transaction_queue_.front();
  transaction_queue_.pop();
  return transaction;
}

void TransactionClient::InitiateTransaction(Transaction* transaction) {
  transaction->start_time = OrbitTicks();
  SendRequestInternal(transaction->type, transaction->payload);
}

void TransactionClient::OnTransactionCompleted(Transaction* transaction) {
  LOG("Transaction complete: %s",
      GetResponseHandler(transaction->type)->description);
}

TransactionResponseHandler* TransactionClient::GetResponseHandler(
    MessageType type) {
  CHECK(HasResponseHandler(type));
  absl::MutexLock lock(&mutex_);
  return &transaction_response_handlers_.at(type);
}

bool TransactionClient::HasResponseHandler(MessageType type) {
  absl::MutexLock lock(&mutex_);
  return transaction_response_handlers_.contains(type);
}

void TransactionClient::HandleResponse(const Message& message) {
  CHECK(current_transaction_ != nullptr);
  uint64_t id = current_transaction_->id;
  const TransactionResponseHandler::ResponseHandler& handler =
      GetResponseHandler(message.GetType())->response_handler;
  if (handler != nullptr) {
    handler(message, id);
  }
  current_transaction_->end_time = OrbitTicks();
  current_transaction_->completed = true;
}

uint64_t TransactionClient::EnqueueRequestInternal(MessageType type,
                                                   std::string&& object) {
  absl::MutexLock lock(&mutex_);
  auto transaction =
      std::make_shared<Transaction>(type, std::move(object), request_counter_);
  ++request_counter_;
  transaction_queue_.push(transaction);
  return transaction->id;
}

void TransactionClient::SendRequestInternal(MessageType type,
                                            const std::string& object) {
  LOG("Sending transaction request: %s [%lu bytes]",
      GetResponseHandler(type)->description, object.size());
  client_->Send(type, object);
}

void TransactionClient::ReceiveResponseInternal(const Message& message) {
  LOG("Receiving transaction response: %s [%u bytes]",
      GetResponseHandler(message.GetType())->description, message.GetSize());
}
