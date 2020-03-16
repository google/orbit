#include <OrbitLinuxTracing/OrbitTracing.h>

namespace orbit {
namespace tracing {

std::unique_ptr<orbit::tracing::Handler> GHandler;

}  // namespace tracing
}  // namespace orbit

namespace LinuxTracing {

void SetOrbitTracingHandler(std::unique_ptr<orbit::tracing::Handler> handler) {
  orbit::tracing::GHandler = std::move(handler);
}

}  // namespace OrbitLinuxTracing
