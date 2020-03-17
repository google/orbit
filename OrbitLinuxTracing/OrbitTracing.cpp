#include <OrbitLinuxTracing/OrbitTracing.h>

#if ORBIT_TRACING_ENABLED

namespace orbit {
namespace tracing {

std::unique_ptr<orbit::tracing::Handler> GHandler;

}  // namespace tracing
}  // namespace orbit

namespace LinuxTracing {

void SetOrbitTracingHandler(std::unique_ptr<orbit::tracing::Handler> handler) {
  orbit::tracing::GHandler = std::move(handler);
}

}  // namespace LinuxTracing

#endif  // ORBIT_TRACING_ENABLED
