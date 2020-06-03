#include "OpenGlDetect.h"

#include <absl/strings/str_split.h>

#include <charconv>

#include "OpenGl.h"
#include "OrbitBase/Logging.h"

namespace OrbitGl {

std::optional<OpenGlVersion> DetectOpenGlVersion(GlutContext*) {
  const auto window =
      glutCreateWindow("Determining supported OpenGL version...");

  if (window <= 0) {
    return std::nullopt;
  }

  glewInit();

  const auto str_ptr = glGetString(GL_VERSION);
  std::string str{};

  if (str_ptr != nullptr) {
    str = std::string{reinterpret_cast<const char*>(str_ptr)};
  }

  glutDestroyWindow(window);

  if (str_ptr == nullptr) {
    return std::nullopt;
  }

  // We have to trigger the event loop three times to destroy the window fully.
  glutMainLoopEvent();
  glutMainLoopEvent();
  glutMainLoopEvent();

  std::vector<std::string> pieces =
      absl::StrSplit(str, absl::MaxSplits(absl::ByAnyChar(". "), 2));

  if (pieces.size() < 2) {
    ERROR("Could not split OpenGL version string: %s", str);
    return std::nullopt;
  }

  OpenGlVersion version{};
  const auto major_result = std::from_chars(
      pieces[0].data(), pieces[0].data() + pieces[0].size(), version.major);

  if (major_result.ec != std::errc()) {
    ERROR("Could not parse major version in OpenGL version string: %s", str);
    return std::nullopt;
  }

  const auto minor_result = std::from_chars(
      pieces[1].data(), pieces[1].data() + pieces[1].size(), version.minor);

  if (minor_result.ec != std::errc()) {
    ERROR("Could not parse minor version in OpenGL version string: %s", str);
    return std::nullopt;
  }

  return version;
}

}  // namespace OrbitGl
