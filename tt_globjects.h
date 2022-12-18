#pragma once

#include <unordered_map>
#include <initializer_list>
#include "Mat44.h"

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

	struct Buffer : GLObject {
		Buffer(Buffer&& other);
		Buffer& operator=(Buffer&& other);
		~Buffer();
		Buffer(unsigned int attachment, size_t num_bytes, const char* data = nullptr, unsigned int usage = 0x88E4 /*GL_STATIC_DRAW*/);
		void bind() const;
		void bind(unsigned int custom_attachment) const;
		size_t get_size() const;
		void set_data(size_t num_bytes, const void* data);

		unsigned int raw_gl_handle() const;
		void bind_to_layout_location(unsigned int index) const;
		void bind_to_layout_location(unsigned int index, unsigned int custom_attachment) const;

	private:
		unsigned int attachment;
		unsigned int handle;
		size_t size;
	};

	struct Image : GLObject {
		enum class EChannelFormat {
			U8,
			F32,
		};
		enum class ELoadStoreMode {
			Read = 0x88B8, // GL_READ_ONLY
			Write = 0x88B9, // GL_WRITE_ONLY
			ReadWrite = 0x88BA, // GL_READ_WRITE
		};

		Image(Image&& other);
		Image& operator=(Image&& other);
		~Image();
		Image(const char* file);
		Image(int width, int height, int channels = 3, const char* data = nullptr, EChannelFormat channel_format = EChannelFormat::U8);
		void bind() const;

		unsigned int raw_gl_handle() const;
		void bind_to_layout_location(unsigned int index, ELoadStoreMode access = ELoadStoreMode::ReadWrite) const;

		int get_width() const;
		int get_height() const;
		int get_channels() const;

		// Data can be nullptr to just allocate
		void set_data(const void* data);
		void set_data(int width, int height, const void* data);

	private:
		unsigned int handle;
		int width;
		int height;
		int channels;
		EChannelFormat channel_format;
	};

	struct RenderTarget : GLObject {
		enum class Attachment {
			Color0 = 0x8CE0,
			Color1 = 0x8CE1,
			Color2 = 0x8CE2,
			Color3 = 0x8CE3,
			Color4 = 0x8CE4,
			Color5 = 0x8CE5,
			Color6 = 0x8CE6,
			Color7 = 0x8CE7,
			Depth = 0x8D00,
			Stencil = 0x8D20,
			DepthStencil = 0x821A,
		};
		RenderTarget(RenderTarget&& other);
		RenderTarget& operator=(RenderTarget&& other);
		~RenderTarget();
		RenderTarget();
		void attach(const Image& image, Attachment attachment);
		void bind() const;

	private:
		unsigned int handle;
	};

	struct Shader : GLObject {
		Shader(Shader&& other);
		Shader& operator=(Shader&& other);
		~Shader();
		Shader(const char* code, unsigned int type);

		static Shader from_file(const char* file_path, unsigned int type);

	private:
		friend struct Program;
		unsigned int handle;
	};

	struct Program : GLObject {
		Program(Program&& other);
		Program& operator=(Program&& other);
		~Program();
		Program(std::initializer_list<Shader> shaders);
		void use() const;
		unsigned int uniform(const char* name);

	private:
		unsigned int handle;
		std::unordered_map<std::string, unsigned int> uniforms;
	};

	union UniformData {
		float f[16];
		int i[16];
		unsigned int u[16];
		const Image* image_address;
	};

	struct UniformValue : GLObject {
		UniformValue(const UniformValue& other);
		UniformValue& operator=(const UniformValue& other);

		UniformValue() = default;
		~UniformValue() = default;
		UniformValue(float value);
		UniformValue(float x, float y);
		UniformValue(float x, float y, float z);
		UniformValue(float x, float y, float z, float w);
		UniformValue(int value);
		UniformValue(unsigned int x, unsigned int y);
		UniformValue(const Image* value);
		UniformValue(const Mat44& value);

	private:
		friend struct Material;
		enum class Type { Invalid, Int, UInt, Float, Image, Mat44 } type = Type::Invalid;
		int num_values = 0;
		UniformData data{};
	};

	struct Material {
		Material(Program& program);
		void set(const char* name, float value);
		void set(const char* name, float x, float y);
		void set(const char* name, float x, float y, float z);
		void set(const char* name, float x, float y, float z, float w);
		void set(const char* name, int value);
		void set(const char* name, unsigned int x, unsigned int y);
		void set(const char* name, const Image* value);
		void set(const char* name, const Mat44& value);
		void use() const;
	private:
		Program& program;
		std::unordered_map<unsigned int, UniformValue> uniforms;
	};

	struct ComputeMaterial : public Material {
	public:
		ComputeMaterial(Program& program, int workgroup_size_x, int workgroup_size_y, int workgroup_size_z);
		void use_and_dispatch(int work_units_x = 1, int work_units_y = 1, int work_units_z = 1, unsigned int barrier_bits = 0);
	private:
		int workgroup_size_x;
		int workgroup_size_y;
		int workgroup_size_z;
	};

	struct MeshAttribute {
		int dimensions;
		unsigned int elementType;

		int SizeOf() const;
	};

	struct Mesh : GLObject {
		Mesh(Mesh&& other);
		Mesh& operator=(Mesh&& other);
		~Mesh();
		Mesh(const std::initializer_list<float>& vertice, const std::initializer_list<unsigned int>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode = 0x88E4 /*GL_STATIC_DRAW*/);
		Mesh(const std::initializer_list<float>& vertices, const std::initializer_list<unsigned short>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode = 0x88E4 /*GL_STATIC_DRAW*/);
		Mesh(const std::initializer_list<float>& vertices, const std::initializer_list<unsigned char>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode = 0x88E4 /*GL_STATIC_DRAW*/);
		void draw() const;

		const Buffer& get_vbo() const { return vbo; }
		const Buffer& get_ibo() const  { return ibo; }

		void resize_vbo(unsigned int num_floats);
		void resize_ibo(unsigned int num_indices);

	private:
		__forceinline void init(const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type);
		unsigned int vao;
		Buffer vbo;
		Buffer ibo;
		unsigned int buffers[2];
		unsigned int primitive_type;
		unsigned int index_type;
		unsigned int index_count;
	};
}
