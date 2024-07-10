#include "gl.hh"

GLenum glLastErrorCode = 0;
std::mutex g_mtxGlContext;
