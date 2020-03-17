#ifndef ORBIT_CORE_INTROSPECTION_H_
#define ORBIT_CORE_INTROSPECTION_H_

#include <OrbitBase/Tracing.h>
#include "ScopeTimer.h"

#if ORBIT_TRACING_ENABLED

namespace orbit {
namespace introspection {

class Handler : public orbit::tracing::Handler {
  void Begin(const char* name) final;
  void End() final;
  void Track(const char* name, int) final;
  void Track(const char* name, float) final;
};

struct Scope {
  Timer timer_;
  std::string name_;
};

}  // namespace introspection
}  // namespace orbit

#endif  // ORBIT_TRACING_ENABLED

#endif  // ORBIT_CORE_INTROSPECTION_H_
