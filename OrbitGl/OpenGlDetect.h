#ifndef ORBIT_GL_OPEN_GL_DETECT__H_
#define ORBIT_GL_OPEN_GL_DETECT__H_

#include <optional>

#include "GlutContext.h"

namespace OrbitGl {

struct OpenGlVersion {
  int major;
  int minor;
};

std::optional<OpenGlVersion> DetectOpenGlVersion(GlutContext*);

}  // namespace OrbitGl

#endif  // ORBIT_GL_OPEN_GL_DETECT__H_