#pragma once

#include "tt_gl.h"
#include "tt_messages.h"

namespace TT {
	struct Buffer {
		GLuint handle = 0;

		void alloc(size_t size, GLenum anchor = GL_SHADER_STORAGE_BUFFER, void* data = nullptr, GLenum mode = GL_STATIC_DRAW);
		void realloc(size_t size, GLenum anchor = GL_SHADER_STORAGE_BUFFER, void* data = nullptr, GLenum mode = GL_STATIC_DRAW);
		void cleanup();
	};

	struct VAO {
		GLuint handle = 0;

		void alloc();
		void cleanup();
	};

	struct Image {
	private:
		int geti(GLenum v);
		void defaults();
		void alloc();

	public:
		GLuint handle = 0;
		GLenum anchor = GL_TEXTURE_2D;

		int width();
		int height();
		int depth();

		void alloc(int width, int height, GLenum internalFormat = GL_RGBA32F, GLenum channels = GL_RGBA, GLenum elementType = GL_FLOAT, void* data = nullptr);
		void alloc(int width, int height, int depth, GLenum internalFormat = GL_RGBA32F, GLenum channels = GL_RGBA, GLenum elementType = GL_FLOAT, void* data = nullptr);
		void realloc(int width, int height, GLenum internalFormat = GL_RGBA32F, GLenum channels = GL_RGBA, GLenum elementType = GL_FLOAT, void* data = nullptr);
		void realloc(int width, int height, int depth, GLenum internalFormat = GL_RGBA32F, GLenum channels = GL_RGBA, GLenum elementType = GL_FLOAT, void* data = nullptr);
		void cleanup();
		void alloc(const char* filePath);
		void realloc(const char* filePath);

		void repeat();
		void clamp();
	};
}
