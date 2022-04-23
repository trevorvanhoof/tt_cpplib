#pragma once

#include <unordered_map>
#include <initializer_list>

namespace TT {
	/*
	This base class deletes the copy constructor to protect you from accidentally
	having 2 owners of an OpenGL object handle.

	The idea is to inherit this class and implement the following:
		Constructor -> aquire OpenGL object handle.
		Destructor -> release OpenGL object handle.
		Move constructor -> take ownership and properly zero out other.handle.
		Move assignment operator -> see move constructor.
	*/
	struct GLObject
	{
		// As we own handles to external APIs we disable copy construction & assignment.
		GLObject() = default;

	private:
		GLObject(const GLObject&) = delete;
		GLObject(GLObject&& other) = delete;
		GLObject& operator=(const GLObject&) = delete;
		GLObject& operator=(GLObject&&) = delete;
	};

	struct Image : GLObject {
		Image(Image&& other);
		Image& operator=(Image&& other);
		~Image();
		Image(const char* file);
		Image(int width, int height, int channels = 3, const char* data = nullptr);
		void bind() const;

		unsigned int raw_gl_handle();
		int get_width();
		int get_height();
		int get_channels();
		void set_data(const void* data);

	private:
		unsigned int handle;
		int width;
		int height;
		int channels;
	};

	struct Shader : GLObject {
		Shader(Shader&& other);
		Shader& operator=(Shader&& other);
		~Shader();
		Shader(const char* code, unsigned int type);

	private:
		friend struct Program;
		unsigned int handle;
	};

	struct Program : GLObject {
		Program(Program&& other);
		Program& operator=(Program&& other);
		~Program();
		Program(std::initializer_list<Shader> shaders, int num_shaders);
		void use() const;
		unsigned int uniform(const char* name);

	private:
		unsigned int handle;
		std::unordered_map<std::string, unsigned int> uniforms;
	};

	struct UniformValue : GLObject {
		UniformValue(UniformValue&& other);
		UniformValue& operator=(UniformValue&& other);
		~UniformValue();
		UniformValue();
		UniformValue(float value);
		UniformValue(float x, float y);
		UniformValue(float x, float y, float z);
		UniformValue(float x, float y, float z, float w);
		UniformValue(int value);
		UniformValue(Image& value);

	private:
		friend struct Material;
		enum class Type { Invalid, Int, Float, Image } type = Type::Invalid;
		int num_values = 0;
		void* data = nullptr;
	};

	struct Material {
		Material(Program& program);
		void set(const char* name, float value);
		void set(const char* name, float x, float y);
		void set(const char* name, float x, float y, float z);
		void set(const char* name, float x, float y, float z, float w);
		void set(const char* name, int value);
		void set(const char* name, Image& value);
		void use() const;

	private:
		Program& program;
		std::unordered_map<unsigned int, UniformValue> uniforms;
	};

	struct MeshAttribute {
		int dimensions;
		unsigned int elementType;
	};

	struct Mesh : GLObject {
		Mesh(Mesh&& other);
		Mesh& operator=(Mesh&& other);
		~Mesh();
		Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned int> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type);
		Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type);
		Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned char> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type);
		void draw() const;

	private:
		__forceinline void init(std::initializer_list<float> vertices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type);
		unsigned int vao;
		unsigned int buffers[2];
		unsigned int primitive_type;
		unsigned int index_type;
		unsigned int index_count;
	};
}
