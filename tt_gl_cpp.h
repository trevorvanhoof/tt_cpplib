// Access to GL and Windows.h in source files, also see tt_gl_h.h
// Include this in 1 source file whilst defining TT_GLEXT_IMPLEMENTATION
#pragma once

#include "tt_gl_h.h"

#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN 1
#endif
#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>
#undef max
#undef min
#undef near
#undef far
