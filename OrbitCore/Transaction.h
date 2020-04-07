#ifndef ORBIT_CORE_TRANSACTION_H_
#define ORBIT_CORE_TRANSACTION_H_

#include <atomic>
#include <functional>
#include <string>

#include "Message.h"

namespace orbit {

struct TransactionHandler {
  MessageType GetType() const { return message_type; }
  typedef std::function<void(const Message&)> Callback;
  Callback request_handler = nullptr;
  Callback response_handler = nullptr;
  MessageType message_type = Msg_Invalid;
  std::string description;
};

struct Transaction {
  MessageType message_type = Msg_Invalid;
  std::string payload;
  uint32_t id = 0;
  uint64_t start_time = 0;
  uint64_t end_time = 0;
  std::atomic<bool> completed = false;
};

}  // namespace orbit

#endif  // ORBIT_CORE_TRANSACTION_H_
