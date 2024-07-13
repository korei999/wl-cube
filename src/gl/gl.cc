#include "gl.hh"

namespace gl
{

GLenum lastErrorCode = 0;
std::mutex mtxGlContext;

} /* namespace gl */
