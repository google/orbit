#include "Introspection.h"

#if ORBIT_TRACING_ENABLED

#include <memory>
#include <vector>

#include "CoreApp.h"
#include "OrbitBase/Tracing.h"
#include "KeyAndString.h"
#include "PrintVar.h"
#include "TcpServer.h"
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

Handler::Handler(std::shared_ptr<StringManager> string_manager,
                 LinuxTracingSession* tracing_session)
    : string_manager_(string_manager), tracing_session_(tracing_session) {}

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
  SendKeyAndString(hash, scope.name_);
  scope.timer_.m_UserData[0] = hash;

  tracing_session_->RecordTimer(std::move(scope.timer_));

  scopes.pop_back();
}

void Handler::SendKeyAndString(uint64_t key, const std::string& str) {
  KeyAndString key_and_string;
  key_and_string.key = key;
  key_and_string.str = str;
  // TODO: This is not atomic, we might end up sending multiple keys
  // maybe change it to AddIfDoesNotExists()
  if (!string_manager_->Exists(key)) {
    std::string message_data = SerializeObjectBinary(key_and_string);
    // TODO: Remove networking from here.
    GTcpServer->Send(Msg_KeyAndString, message_data.c_str(),
                     message_data.size());
    string_manager_->Add(key, str);
  }
}

void Handler::Track(const char* name, int) {}

void Handler::Track(const char* name, float) {}

}  // namespace introspection
}  // namespace orbit

#endif  // ORBIT_TRACING_ENABLED
