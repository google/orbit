
#include <filesystem>

#include "Message.h"
#include "OrbitAsioServer.h"

OrbitAsioServer asioServer{65001, LinuxTracing::TracingOptions{}};

extern "C" {

int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  Message message{};
  std::memcpy(&message, buf, std::min(len, sizeof(message)));

  const bool payload_available = len > sizeof(message);
  const auto payload_size = len - sizeof(message);
  std::vector<char> payload(payload_available ? payload_size : 0);
  std::memcpy(payload.data(), buf + sizeof(message), payload.size());

  try {
    GTcpServer->Callback(MessageOwner{message, std::move(payload)});
    GTcpServer->ProcessMainThreadCallbacks();
  } catch (...) {
  }
  return 0;
}

int LLVMFuzzerInitialize(int*, char***) { return 0; }
}  // extern "C"
