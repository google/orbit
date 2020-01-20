#include "android-base/strings.h"

namespace android {
namespace base {

bool StartsWith(std::string s, std::string prefix) {
  return s.substr(0, prefix.size()) == prefix;
}

}
}
