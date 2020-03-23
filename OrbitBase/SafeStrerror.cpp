#include <OrbitBase/SafeStrerror.h>

#include <cstring>

char* SafeStrerror(int errnum) {
  constexpr size_t BUFLEN = 256;
  thread_local char buf[BUFLEN];
#ifdef _MSC_VER
  strerror_s(buf, BUFLEN, errnum);
  return buf;
#else
  return strerror_r(errnum, buf, BUFLEN);
#endif
}
