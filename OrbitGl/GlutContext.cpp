#include "GlutContext.h"

#include "OpenGl.h"

namespace OrbitGl {
GlutContext::GlutContext(int* argc, char** argv) {
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
}
}  // namespace OrbitGl
