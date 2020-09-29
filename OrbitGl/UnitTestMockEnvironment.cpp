#include <absl/flags/flag.h>

// Flag declarations
ABSL_DECLARE_FLAG(bool, devmode);
ABSL_DECLARE_FLAG(bool, local);

ABSL_DECLARE_FLAG(uint16_t, sampling_rate);
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);

// Flag usages
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");