// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_TRANSACTION_CLIENT_H_
#define ORBIT_CORE_TRANSACTION_CLIENT_H_

#include <atomic>
#include <functional>
#include <queue>
#include <utility>

#include "Message.h"
#include "Serialization.h"
#include "TcpClient.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

// The TransactionClient and TransactionService (TransactionService.h) are
// responsible for coordinating requests from the client (UI) to the service and
// responses from the service back to the client. The goal is to centralize
// communications between client and service to ensure that only *one*
// transaction is in flight at at any given time. The user can enqueue
// transactions having a guarantee that they will be executed in order. Note
// that enqueueing requests is thread-safe.
//
// Usage: Register one TransactionResponseHandler per message type on the
// TransactionClient and one TransactionRequestHandler per message type on the
// TransactionService. A TransactionRequestHandler carries a request handler
// that will be executed on the service side and TransactionResponseHandler
// carries a response handler for the client side.
// The steps for issuing requests and receiving a response are:
// 1. The client enqueues a request through "TransactionClient::EnqueueRequest".
// 2. The TransactionClient schedules and sends out the request.
// 3. Then service receives the request in its request handler.
//    The request can be deserialized by calling
//    "TransactionService::ReceiveRequest".
// 4. The service sends a response through "TransactionService::SendResponse".
// 5. The client receives the response in its response handler.
//    The response can be deserialized by calling
//    "TransactionClient::ReceiveResponse".

struct TransactionResponseHandler {
  using ResponseHandler = std::function<void(const Message&, uint64_t /*id*/)>;

  TransactionResponseHandler() = default;
  TransactionResponseHandler(ResponseHandler response_handler, MessageType type,
                             std::string description)
      : response_handler{std::move(response_handler)},
        type{type},
        description{std::move(description)} {}

  ResponseHandler response_handler;
  MessageType type = Msg_Invalid;
  std::string description;
};

struct Transaction {
  Transaction() = default;
  Transaction(MessageType type, std::string payload, uint64_t id)
      : type{type}, payload{std::move(payload)}, id{id} {}

  MessageType type = Msg_Invalid;
  std::string payload;
  uint64_t id = 0;
  uint64_t start_time = 0;
  uint64_t end_time = 0;
  std::atomic<bool> completed = false;
};

class TransactionClient {
 public:
  explicit TransactionClient(TcpClient* client);

  TransactionClient() = delete;
  TransactionClient(const TransactionClient&) = delete;
  TransactionClient& operator=(const TransactionClient&) = delete;
  TransactionClient(TransactionClient&&) = delete;
  TransactionClient& operator=(TransactionClient&&) = delete;

  void RegisterTransactionResponseHandler(
      const TransactionResponseHandler& handler);

  template <typename T>
  uint64_t EnqueueRequest(MessageType type, const T& object) {
    return EnqueueRequestInternal(type, SerializeObjectBinary(object));
  }

  template <typename T>
  void EnqueueRequest(MessageType type, const T* object) = delete;

  template <typename T>
  void ReceiveResponse(const Message& message, T* object) {
    DeserializeObjectBinary(message.GetData(), message.GetSize(), *object);
    ReceiveResponseInternal(message);
  }

  void Tick();

 private:
  bool IsTransactionQueueEmpty();
  std::shared_ptr<Transaction> PopTransaction();
  void InitiateTransaction(Transaction* transaction);
  void OnTransactionCompleted(Transaction* transaction);

  TransactionResponseHandler* GetResponseHandler(MessageType type);
  bool HasResponseHandler(MessageType type);
  void HandleResponse(const Message& message);

  uint64_t EnqueueRequestInternal(MessageType type, std::string&& object);
  void SendRequestInternal(MessageType type, const std::string& object);
  void ReceiveResponseInternal(const Message& message);

  TcpClient* client_;
  uint64_t request_counter_ = 0;
  std::queue<std::shared_ptr<Transaction>> transaction_queue_;
  std::shared_ptr<Transaction> current_transaction_ = nullptr;
  absl::flat_hash_map<MessageType, TransactionResponseHandler>
      transaction_response_handlers_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_CORE_TRANSACTION_CLIENT_H_
