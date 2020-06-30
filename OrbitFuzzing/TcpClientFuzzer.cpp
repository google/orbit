#include <absl/flags/flag.h>

#include <filesystem>

#include "App.h"
#include "Capture.h"
#include "CaptureWindow.h"
#include "Message.h"
#include "TcpClient.h"
#include "TimeGraph.h"

// Hack: This is declared in a header we include here
// and the definition needs to take place somewhere.
ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

std::unique_ptr<CaptureWindow> capture_window;

extern "C" {

int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  Capture::NewSamplingProfiler();

  Message message{};
  std::memcpy(&message, buf, std::min(len, sizeof(message)));

  const bool payload_available = len > sizeof(message);
  const auto payload_size = len - sizeof(message);
  std::vector<char> payload(payload_available ? payload_size : 0);
  std::memcpy(payload.data(), buf + sizeof(message), payload.size());

  try {
    GTcpClient->Callback(MessageOwner{message, std::move(payload)});
    GTcpClient->ProcessMainThreadCallbacks();
  } catch (...) {
  }
  return 0;
}

int LLVMFuzzerInitialize(int*, char***) {
  OrbitApp::Init({"127.0.0.1:65001", ""});
  capture_window = std::make_unique<CaptureWindow>();
  GOrbitApp->PostInit();
  return 0;
}
}  // extern "C"
