// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRANSACTION_SERVICE_H_
#define ORBIT_CORE_TRANSACTION_SERVICE_H_

#include <functional>
#include <utility>

#include "Message.h"
#include "Serialization.h"
#include "TcpServer.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

// See TransactionClient.h for details.

struct TransactionRequestHandler {
  using RequestHandler = std::function<void(const Message&)>;

  TransactionRequestHandler() = default;
  TransactionRequestHandler(RequestHandler request_handler, MessageType type,
                            std::string description)
      : request_handler{std::move(request_handler)},
        type{type},
        description{std::move(description)} {}

  RequestHandler request_handler;
  MessageType type = Msg_Invalid;
  std::string description;
};

class TransactionService {
 public:
  explicit TransactionService(TcpServer* server);

  TransactionService() = delete;
  TransactionService(const TransactionService&) = delete;
  TransactionService& operator=(const TransactionService&) = delete;
  TransactionService(TransactionService&&) = delete;
  TransactionService& operator=(TransactionService&&) = delete;

  void RegisterTransactionRequestHandler(
      const TransactionRequestHandler& handler);

  template <typename T>
  void ReceiveRequest(const Message& message, T* object) {
    DeserializeObjectBinary(message.GetData(), message.GetSize(), *object);
    ReceiveRequestInternal(message);
  }

  template <typename T>
  void SendResponse(MessageType type, const T& object) {
    SendResponseInternal(type, SerializeObjectBinary(object));
  }

  template <typename T>
  void SendResponse(MessageType type, const T* object) = delete;

 private:
  TransactionRequestHandler* GetRequestHandler(MessageType type);
  bool HasRequestHandler(MessageType type);
  void HandleRequest(const Message& message);

  void ReceiveRequestInternal(const Message& message);
  void SendResponseInternal(MessageType type, const std::string& object);

  TcpServer* server_;
  absl::flat_hash_map<MessageType, TransactionRequestHandler>
      transaction_request_handlers_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_CORE_TRANSACTION_SERVICE_H_
