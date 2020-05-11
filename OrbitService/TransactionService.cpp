// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TransactionService.h"

#include <OrbitBase/Logging.h>

TransactionService::TransactionService(TcpServer* server) : server_{server} {
  CHECK(server_ != nullptr);
}

void TransactionService::RegisterTransactionRequestHandler(
    const TransactionRequestHandler& handler) {
  CHECK(!HasRequestHandler(handler.type));
  absl::MutexLock lock(&mutex_);
  transaction_request_handlers_[handler.type] = handler;
  server_->AddMainThreadCallback(
      handler.type, [this](const Message& msg) { HandleRequest(msg); });
}

TransactionRequestHandler* TransactionService::GetRequestHandler(
    MessageType type) {
  CHECK(HasRequestHandler(type));
  absl::MutexLock lock(&mutex_);
  return &transaction_request_handlers_.at(type);
}

bool TransactionService::HasRequestHandler(MessageType type) {
  absl::MutexLock lock(&mutex_);
  return transaction_request_handlers_.contains(type);
}

void TransactionService::HandleRequest(const Message& message) {
  const TransactionRequestHandler::RequestHandler& handler =
      GetRequestHandler(message.GetType())->request_handler;
  if (handler != nullptr) {
    handler(message);
  }
}

void TransactionService::ReceiveRequestInternal(const Message& message) {
  LOG("Receiving transaction request: %s [%u bytes]",
      GetRequestHandler(message.GetType())->description, message.GetSize());
}

void TransactionService::SendResponseInternal(MessageType type,
                                              const std::string& object) {
  LOG("Sending transaction response: %s [%lu bytes]",
      GetRequestHandler(type)->description, object.size());
  server_->Send(type, object);
}
