#include <absl/flags/flag.h>

#include <cstdio>
#include <filesystem>

#include "App.h"
#include "CaptureSerializer.h"
#include "SamplingProfiler.h"
#include "TimeGraph.h"

// Hack: This is declared in a header we include here
// and the definition needs to take place somewhere.
ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

std::string capture_file;

extern "C" {

int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  CaptureSerializer serializer{};
  TimeGraph time_graph{};
  auto string_manager = std::make_shared<StringManager>();
  time_graph.SetStringManager(string_manager);
  serializer.time_graph_ = &time_graph;

  std::istringstream stream(
      std::string(reinterpret_cast<const char*>(buf), len));
  try {
    (void)serializer.Load(stream);
  } catch (...) {
  }

  Capture::GSamplingProfiler.reset();
  return 0;
}

int LLVMFuzzerInitialize(int*, char***) {
  OrbitApp::Init({});
  return 0;
}
}  // extern "C"
