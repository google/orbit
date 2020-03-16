#ifndef ORBIT_TRACING_TRACING_H_
#define ORBIT_TRACING_TRACING_H_

#include <memory>

#define ORBIT_TRACING_ENABLED 1

#if ORBIT_TRACING_ENABLED

// Scoped trace with user defined name.
#define ORBIT_SCOPE(name) orbit::tracing::Scope ORBIT_UNIQUE(ORB)(name)
// Scoped trace with calling function name.
#define ORBIT_SCOPE_FUNC ORBIT_SCOPE(__FUNCTION__)
// Manual scope begin.
#define ORBIT_BEGIN(name) ORBIT_CALL(Begin(name))
// Manual scope end.
#define ORBIT_END ORBIT_CALL(End())
// Track named variable (int or float).
#define ORBIT_TRACK(name, var) ORBIT_CALL(Track(name, var))

// Internal macros.
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y) ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x) ORBIT_CONCAT(x, __COUNTER__)
#define ORBIT_CALL(f)           \
  if (orbit::tracing::GHandler) { \
    orbit::tracing::GHandler->f;  \
  }

namespace orbit {
namespace tracing {

class Handler {
 public:
  virtual void Begin(const char* name) = 0;
  virtual void End() = 0;
  virtual void Track(const char* name, int) = 0;
  virtual void Track(const char* name, float) = 0;
};

// This must be instanciated in user code.
extern std::unique_ptr<Handler> GHandler;

struct Scope {
  Scope(const char* name) { ORBIT_BEGIN(name); }
  ~Scope() { ORBIT_END; }
};

}  // namespace tracing
}  // namespace orbit

#else

#define ORBIT_SCOPE(name)
#define ORBIT_SCOPE_FUNC
#define ORBIT_BEGIN(name)
#define ORBIT_END
#define ORBIT_TRACK(var)

#endif

#endif  // ORBIT_TRACING_TRACING_H_
