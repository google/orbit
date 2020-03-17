#ifndef ORBIT_LINUX_TRACING_ORBIT_TRACING_H_
#define ORBIT_LINUX_TRACING_ORBIT_TRACING_H_

#include <OrbitBase/Tracing.h>
#include <memory.h>

#if ORBIT_TRACING_ENABLED

namespace LinuxTracing {
void SetOrbitTracingHandler(std::unique_ptr<orbit::tracing::Handler> handler);
}

#endif  // ORBIT_TRACING_ENABLED

#endif  // ORBIT_LINUX_TRACING_ORBIT_TRACING_H_
