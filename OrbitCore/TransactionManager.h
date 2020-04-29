#ifndef ORBIT_CORE_TRANSACTION_MANAGER_H
#define ORBIT_CORE_TRANSACTION_MANAGER_H

#include <memory>
#include <queue>

#include "Message.h"
#include "Serialization.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "Transaction.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"

namespace orbit {

// The transaction manager is responsible for coordinating requests from the
// client (UI) to the service and responses from the service back to the client.
// The goal is to centralize communications between client and service to ensure
// that only *one* transaction is in flight at at any given time. The user can
// enqueue transactions having a guarantee that they will be executed in order.
// Note that enqueueing requests is thread-safe.
//
// Usage: Register one TransactionHandler per message type. A TransactionHandler
//        consists of a request handler that will be executed on the service
//        side and a response handler for the client side (see Transaction.h).
//        The steps for issuing requests and receiveing a response are:
//
//        1. The client enqueues a request through "EnqueueRequest".
//        2. The TransactionManager schedules and sends out the request.
//        3. Then service receives the request in its request handler.
//           The request can be deserialized by calling "ReceiveRequest".
//        4. The service sends a response through "SendResponse".
//        5. The client receives the response in its response handler.
//           The response can be deserialized by calling "ReceiveResponse".

class TransactionManager {
 public:
  TransactionManager(TcpClient* client, TcpServer* server);

  TransactionManager() = delete;
  TransactionManager(const TransactionManager&) = delete;
  TransactionManager& operator=(const TransactionManager&) = delete;
  TransactionManager(TransactionManager&&) = delete;
  TransactionManager& operator=(TransactionManager&&) = delete;

  void RegisterTransactionHandler(const TransactionHandler& handler);

  template <typename T>
  uint32_t EnqueueRequest(MessageType type, const T& object) {
    return EnqueueRequestInternal(type, SerializeObjectBinary(object));
  }

  template <typename T>
  void EnqueueRequest(MessageType type, const T* object) = delete;

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

  template <typename T>
  void ReceiveResponse(const Message& message, T* object) {
    DeserializeObjectBinary(message.GetData(), message.GetSize(), *object);
    ReceiveResponseInternal(message);
  }

  void Tick();

 private:
  uint32_t EnqueueRequestInternal(MessageType type, std::string&& object);
  void InitiateTransaction(std::shared_ptr<Transaction> transaction);
  void SendRequestInternal(MessageType type, const std::string& object);
  void ReceiveRequestInternal(const Message& message);
  void SendResponseInternal(MessageType type, const std::string& object);
  void ReceiveResponseInternal(const Message& message);
  void OnTransactionCompleted(std::shared_ptr<Transaction> transaction);

  std::shared_ptr<TransactionHandler> GetHandler(MessageType type);
  bool HasHandler(MessageType type) const;
  bool IsTransactionQueueEmpty() const;
  std::shared_ptr<Transaction> PopTransaction();
  void HandleRequest(const Message& message);
  void HandleResponse(const Message& message);

  typedef std::function<void(const Message&)> Callback;
  Callback on_response_;
  Callback on_request_;
  TcpClient* client_ = nullptr;
  TcpServer* server_ = nullptr;
  std::queue<std::shared_ptr<Transaction>> transaction_queue_;
  std::shared_ptr<Transaction> current_transaction_ = nullptr;
  absl::flat_hash_map<MessageType, std::shared_ptr<TransactionHandler>>
      transaction_handlers_;
  mutable absl::Mutex mutex_;
  uint32_t request_counter_ = 0;
};

}  // namespace orbit

#endif  // ORBIT_CORE_TRANSACTION_MANAGER_H
