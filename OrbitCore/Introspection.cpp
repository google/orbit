#include "Introspection.h"

#if ORBIT_TRACING_ENABLED

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "PrintVar.h"
#include "Utils.h"

namespace orbit {
namespace introspection {

thread_local std::vector<Scope> scopes;

void Handler::Begin(const char* name) {
  scopes.emplace_back(Scope{Timer(), name});
  scopes.back().timer_.Start();
}

void Handler::End() {
  Scope& scope = scopes.back();
  scope.timer_.Stop();
  scope.timer_.m_Type = Timer::INTROSPECTION;
  scope.timer_.EncodeString(scope.name_.c_str());
  scope.timer_.m_Depth = scopes.size() - 1;
  GCoreApp->ProcessTimer(scope.timer_, scope.name_);
  scopes.pop_back();
}

void Handler::Track(const char* name, int) {}

void Handler::Track(const char* name, float) {}

}  // namespace introspection
}  // namespace orbit

#endif  // ORBIT_TRACING_ENABLED
