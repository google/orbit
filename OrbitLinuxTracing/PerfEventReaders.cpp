#include "PerfEventReaders.h"

#include <OrbitBase/Logging.h>
#include "PerfEventRingBuffer.h"

namespace LinuxTracing {

uint16_t ReadTracepointCommonType(PerfEventRingBuffer* ring_buffer) {
  uint16_t common_type = -1;
  ring_buffer->ReadValueAtOffset(
      &common_type, offsetof(perf_event_tracepoint, common_type));
  return common_type;
}

}  // namespace LinuxTracing
