#include "android-base/file.h"

#include <sys/stat.h>
#include <string>
#include <unistd.h>

#include "android-base/unique_fd.h"

namespace android{
namespace base {

bool ReadFdToString(borrowed_fd fd, std::string* content) {
  content->clear();

  // Although original we had small files in mind, this code gets used for
  // very large files too, where the std::string growth heuristics might not
  // be suitable. https://code.google.com/p/android/issues/detail?id=258500.
  struct stat sb;
  if (fstat(fd.get(), &sb) != -1 && sb.st_size > 0) {
    content->reserve(sb.st_size);
  }

  char buf[BUFSIZ];
  ssize_t n;
  while ((n = TEMP_FAILURE_RETRY(read(fd.get(), &buf[0], sizeof(buf)))) > 0) {
    content->append(buf, n);
  }
  return (n == 0) ? true : false;
}

bool ReadFileToString(const std::string& path, std::string* content, bool follow_symlinks) {
  content->clear();

  int flags = O_RDONLY | O_CLOEXEC | O_BINARY | (follow_symlinks ? 0 : O_NOFOLLOW);
  android::base::unique_fd fd(TEMP_FAILURE_RETRY(open(path.c_str(), flags)));
  if (fd == -1) {
    return false;
  }
  return ReadFdToString(fd, content);
}

}
}
