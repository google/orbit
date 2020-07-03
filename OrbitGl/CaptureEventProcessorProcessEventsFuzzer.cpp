
#include <libfuzzer/libfuzzer_macro.h>

#include "CaptureEventProcessor.h"
#include "CaptureListener.h"
#include "absl/flags/flag.h"
#include "services.pb.h"

ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

namespace {
class MyCaptureListener : public CaptureListener {
  void OnTimer(Timer) override {}
  void OnKeyAndString(uint64_t, std::string) override {}
  void OnCallstack(CallStack) override {}
  void OnCallstackEvent(CallstackEvent) override {}
  void OnThreadName(int32_t, std::string) override {}
  void OnAddressInfo(LinuxAddressInfo) override {}
};
}  // namespace

DEFINE_PROTO_FUZZER(const CaptureResponse& response) {
  MyCaptureListener listener;
  CaptureEventProcessor processor{&listener};
  processor.ProcessEvents(response.capture_events());
}