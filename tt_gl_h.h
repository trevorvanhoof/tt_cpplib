#pragma once

#ifndef APIENTRY
#define APIENTRY __stdcall
#define UNDEF_APIENTRY
#endif
#ifndef WINGDIAPI 
#define WINGDIAPI __declspec(dllimport)
#define UNDEF_WINGDIAPI
#endif
#include <gl/gl.h>
#include "glext.h"
#include "tt_glext_win.h"
#ifdef UNDEF_APIENTRY
#undef UNDEF_APIENTRY
#undef APIENTRY
#endif
#ifdef UNDEF_WINGDIAPI
#undef UNDEF_WINGDIAPI
#undef WINGDIAPI
#endif
