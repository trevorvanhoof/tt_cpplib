#pragma once

#include "windont.h"
#include <gl/gl.h>
#include "glext.h"
#include "tt_messages.h"

#ifndef TT_GLEXT_IMPLEMENTATION
namespace TT {
	extern bool checkGLErrors();
	extern void loadGLFunctions();
}

#ifdef TT_GL_DBG
#include "tt_gl_defs_dbg.inc"
#define TT_GL_DBG_ERR checkGLErrors();
#else
#include "tt_gl_defs.inc"
#define TT_GL_DBG_ERR
#endif

#else
namespace TT {
	bool checkGLErrors() {
		GLenum error = glGetError();
		switch (error) {
		case GL_NO_ERROR:
			return false;
		case GL_INVALID_ENUM:
			TT::warning("GL_INVALID_ENUM");
			return true;
		case GL_INVALID_VALUE:
			TT::warning("GL_INVALID_VALUE");
			return true;
		case GL_INVALID_OPERATION:
			TT::warning("GL_INVALID_OPERATION");
			return true;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			TT::warning("GL_INVALID_FRAMEBUFFER_OPERATION");
			return true;
		case GL_OUT_OF_MEMORY:
			TT::warning("GL_OUT_OF_MEMORY");
			return true;
		default:
			TT::warning("Unknown error code %d", error);
			return true;
		}
	}
}

#ifdef TT_GL_DBG
#include "tt_gl_impl_dbg.inc"
#else
#include "tt_gl_impl.inc"
#endif
#endif
