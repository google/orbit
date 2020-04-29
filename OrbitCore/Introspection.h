#ifndef ORBIT_CORE_INTROSPECTION_H_
#define ORBIT_CORE_INTROSPECTION_H_

#include "LinuxTracingBuffer.h"
#include "OrbitBase/Tracing.h"
#include "ScopeTimer.h"
#include "StringManager.h"

#if ORBIT_TRACING_ENABLED

namespace orbit::introspection {

class Handler : public orbit::tracing::Handler {
 public:
  explicit Handler(LinuxTracingBuffer* tracing_buffer);

  void Begin(const char* name) final;
  void End() final;
  void Track(const char* name, int) final;
  void Track(const char* name, float) final;

 private:
  LinuxTracingBuffer* tracing_buffer_;
};

struct Scope {
  Timer timer_;
  std::string name_;
};

}  // namespace orbit::introspection

#endif  // ORBIT_TRACING_ENABLED

#endif  // ORBIT_CORE_INTROSPECTION_H_
