#include "gl.hh"

GLenum g_glLastErrorCode = 0;
std::mutex g_mtxGlContext;
