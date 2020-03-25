#include "Introspection.h"

#if ORBIT_TRACING_ENABLED

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "OrbitBase/Tracing.h"
#include "PrintVar.h"
#include "Utils.h"

#ifdef _WIN32
namespace orbit {
namespace tracing {

// Instanciate tracing handler. On Linux, see OrbitTracing.cpp.
std::unique_ptr<Handler> GHandler;

}  // namespace tracing
}  // namespace orbit
#endif

namespace orbit {
namespace introspection {

thread_local std::vector<Scope> scopes;

Handler::Handler(LinuxTracingSession* tracing_session)
    :  tracing_session_(tracing_session) {}

void Handler::Begin(const char* name) {
  scopes.emplace_back(Scope{Timer(), name});
  scopes.back().timer_.Start();
}

void Handler::End() {
  Scope& scope = scopes.back();

  scope.timer_.Stop();
  scope.timer_.m_Type = Timer::INTROSPECTION;
  scope.timer_.m_Depth = scopes.size() - 1;

  uint64_t hash = StringHash(scope.name_);
  tracing_session_->SendKeyAndString(hash, scope.name_);
  scope.timer_.m_UserData[0] = hash;

  tracing_session_->RecordTimer(std::move(scope.timer_));

  scopes.pop_back();
}

void Handler::Track(const char* name, int) {}

void Handler::Track(const char* name, float) {}

}  // namespace introspection
}  // namespace orbit

#endif  // ORBIT_TRACING_ENABLED
