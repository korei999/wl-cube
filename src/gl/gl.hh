#pragma once
#ifdef __linux__
#include <GLES3/gl32.h>
#elif _WIN32
#include "../platform/windows/glad.h"
#endif

#ifdef DEBUG
#    define D(C)                                                                                                       \
        {                                                                                                              \
            /* call function C and check for error, enabled if -DDEBUG and -DLOGS */                                   \
            C;                                                                                                         \
            while ((glLastErrorCode = glGetError()))                                                                   \
            {                                                                                                          \
                switch (glLastErrorCode)                                                                               \
                {                                                                                                      \
                    default:                                                                                           \
                        LOG(WARNING, "unknown error: {:#x}\n", glLastErrorCode);                                       \
                        break;                                                                                         \
                                                                                                                       \
                    case 0x506:                                                                                        \
                        LOG(BAD, "GL_INVALID_FRAMEBUFFER_OPERATION\n");                                                \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_ENUM:                                                                              \
                        LOG(BAD, "GL_INVALID_ENUM\n");                                                                 \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_VALUE:                                                                             \
                        LOG(BAD, "GL_INVALID_VALUE\n");                                                                \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_INVALID_OPERATION:                                                                         \
                        LOG(BAD, "GL_INVALID_OPERATION\n");                                                            \
                        break;                                                                                         \
                                                                                                                       \
                    case GL_OUT_OF_MEMORY:                                                                             \
                        LOG(BAD, "GL_OUT_OF_MEMORY\n");                                                                \
                        break;                                                                                         \
                }                                                                                                      \
            }                                                                                                          \
        }
#else
#    define D(C) C;
#endif

extern GLenum glLastErrorCode;
