#include "tt_globjects.h"
#include "tt_messages.h"
#include "tt_files.h"

#include "tt_gl_cpp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace TT {
	Buffer::Buffer(Buffer&& other) { handle = other.handle; other.handle = 0; attachment = other.attachment; size = other.size; }
	Buffer& Buffer::operator=(Buffer&& other) { handle = other.handle; other.handle = 0; attachment = other.attachment; size = other.size; return *this; }
	Buffer::~Buffer() { glDeleteBuffers(1, &handle); }
	Buffer::Buffer(unsigned int attachment, size_t num_bytes, const char* data, unsigned int usage) : 
		attachment(attachment) {
		glGenBuffers(1, &handle);
		set_data(num_bytes, data); 
	}
	void Buffer::bind() const { glBindBuffer(attachment, handle);  }
	void Buffer::bind(unsigned int custom_attachment) const { glBindBuffer(custom_attachment, handle);  }
	size_t Buffer::get_size() const { return size; }
	void Buffer::set_data(size_t num_bytes, const void* data) { 
		size = num_bytes;
		if (num_bytes == 0) 
			return; 
		bind();
		glBufferData(attachment, num_bytes, data, GL_STATIC_DRAW); 
	}
	unsigned int Buffer::raw_gl_handle() const { return handle; }
	void Buffer::bind_to_layout_location(unsigned int index) const { bind(); glBindBufferBase(attachment, index, handle); }
	void Buffer::bind_to_layout_location(unsigned int index, unsigned int custom_attachment) const { bind(custom_attachment); glBindBufferBase(custom_attachment, index, handle); }

	namespace {
		GLenum gl_internal_format(int channels, Image::EChannelFormat format) {
			switch (format) {
			case Image::EChannelFormat::U8:
				switch (channels) {
				case 1: return GL_RED;
				case 2: return GL_RG;
				case 3: return GL_RGB;
				case 4: return GL_RGBA;
				default: AssertFatal(false); return 0;
				}
			case Image::EChannelFormat::F32:
				switch (channels) {
				case 1: return GL_R32F;
				case 2: AssertFatal(false, "RG32F is not supported on most GPUs."); return GL_RG32F;
				case 3: return GL_RGB32F;
				case 4: return GL_RGBA32F;
				default: AssertFatal(false); return 0;
				}
			default: AssertFatal(false); return 0;
			}

		}

		GLenum gl_channels(int channels) {
			switch (channels) {
			case 1: return GL_RED;
			case 2: return GL_RG;
			case 3: return GL_RGB;
			case 4: return GL_RGBA;
			default: AssertFatal(false); return 0;
			}
		}

		GLenum gl_channel_format(Image::EChannelFormat format) {
			switch (format) {
			case Image::EChannelFormat::U8:
				return GL_UNSIGNED_BYTE;
			case Image::EChannelFormat::F32:
				return GL_FLOAT;
			default: AssertFatal(false); return 0;
			}
		}
	}

	Image::Image(Image&& other) { 
		handle = other.handle;
		other.handle = 0;
		width = other.width;
		height = other.height;
		channels = other.channels;
		channel_format = other.channel_format;
	}

	Image& Image::operator=(Image&& other) { 
		handle = other.handle; 
		other.handle = 0;
		width = other.width;
		height = other.height;
		channels = other.channels;
		channel_format = other.channel_format; return *this; }

	Image::~Image() { glDeleteTextures(1, &handle); handle = 0; }

	Image::Image(const char* file) {
		glGenTextures(1, &handle);
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char* data = stbi_load(file, &width, &height, &channels, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format(channels, channel_format), width, height, 0, gl_channels(channels), gl_channel_format(channel_format), data);
		stbi_image_free(data);
	}

	Image::Image(int width, int height, int channels, const char* data, EChannelFormat channel_format) :
		width(width), height(height), channels(channels), channel_format(channel_format) {
		glGenTextures(1, &handle);
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format(channels, channel_format), width, height, 0, gl_channels(channels), gl_channel_format(channel_format), data);
	}

	void Image::bind() const { glBindTexture(GL_TEXTURE_2D, handle); }

	unsigned int Image::raw_gl_handle() const { return handle; }
	void Image::bind_to_layout_location(unsigned int index, ELoadStoreMode access) const { glBindImageTexture(index, handle, 0, false, 0, (GLenum)access, GL_RGBA32F); }
	int Image::get_width() const { return width; }
	int Image::get_height() const { return height; }
	int Image::get_channels() const { return channels; }

	void Image::set_data(const void* data) {
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format(channels, channel_format), width, height, 0, gl_channels(channels), gl_channel_format(channel_format), data);
	}

	void Image::set_data(int width, int height, const void* data) {
		this->width = width;
		this->height = height;
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format(channels, channel_format), width, height, 0, gl_channels(channels), gl_channel_format(channel_format), data);
	}

	RenderTarget::RenderTarget(RenderTarget&& other) {
		handle = other.handle;
		other.handle = 0;
	}

	RenderTarget& RenderTarget::operator=(RenderTarget&& other) {
		handle = other.handle;
		other.handle = 0;
		return *this;
	}

	RenderTarget::~RenderTarget() { glDeleteFramebuffers(1, &handle); handle = 0; }

	RenderTarget::RenderTarget() {
		glGenFramebuffers(1, &handle);
	}

	void RenderTarget::bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}

	void RenderTarget::attach(const Image& image, Attachment attachment) {
		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum)attachment, GL_TEXTURE_2D, image.raw_gl_handle(), 0);
	}

	Shader Shader::from_file(const char* file_path, unsigned int type) {
		std::string tmp = ReadAllBytes(file_path);
		return Shader(tmp.c_str(), type);
	}
	Shader::Shader(Shader&& other) { handle = other.handle; other.handle = 0; }
	Shader& Shader::operator=(Shader&& other) { handle = other.handle; other.handle = 0; return *this; }
	Shader::~Shader() { glDeleteShader(handle); handle = 0; }
	Shader::Shader(const char* code, unsigned int type) {
		handle = glCreateShader(type);
		glShaderSource(handle, 1, &code, nullptr);
		glCompileShader(handle);
		GLint status;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			char buffer[512];
			glGetShaderInfoLog(handle, 512, NULL, buffer);
			TT::Info(buffer);
			DebugBreak();
		}
	}

	Program::Program(Program&& other) { handle = other.handle; other.handle = 0; uniforms = other.uniforms; }
	Program& Program::operator=(Program&& other) { handle = other.handle; other.handle = 0; return *this; uniforms = other.uniforms; }
	Program::~Program() { glDeleteProgram(handle); handle = 0; }

	Program::Program(std::initializer_list<Shader> shaders) {
		handle = glCreateProgram();
		for (const Shader& shader : shaders) {
			glAttachShader(handle, shader.handle);
		}

		glLinkProgram(handle);
		for (const Shader& shader : shaders) {
			glDetachShader(handle, shader.handle);
		}

		GLint status;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		if (status != GL_TRUE) {
			char buffer[512];
			glGetProgramInfoLog(handle, 512, NULL, buffer);
			TT::Info(buffer);
			DebugBreak();
		}
	}

	void Program::use() const { glUseProgram(handle); }

	GLuint Program::uniform(const char* name) {
		if (uniforms.find(name) == uniforms.end())
			uniforms[name] = glGetUniformLocation(handle, name);
		return uniforms[name];
	}

	UniformValue::UniformValue(const UniformValue& other) { type = other.type; num_values = other.num_values; data = other.data; };
	UniformValue& UniformValue::operator=(const UniformValue& other) { type = other.type; num_values = other.num_values; data = other.data; return *this; };
	UniformValue::UniformValue(UniformValue&& other) {
		type = other.type;
		num_values = other.num_values;
		data = other.data;
		other.type = Type::Invalid;
		other.num_values = 0;
	}
	UniformValue& UniformValue::operator=(UniformValue&& other) {
		type = other.type;
		num_values = other.num_values;
		data = other.data;
		other.type = Type::Invalid;
		other.num_values = 0;
		return *this;
	}
	UniformValue::UniformValue(float value) { data.f[0] = value; num_values = 1; type = Type::Float; }
	UniformValue::UniformValue(float x, float y) { data.f[0] = x; data.f[1] = y; num_values = 2; type = Type::Float; }
	UniformValue::UniformValue(float x, float y, float z) { data.f[0] = x; data.f[1] = y; data.f[2] = z; num_values = 3; type = Type::Float; }
	UniformValue::UniformValue(float x, float y, float z, float w) { data.f[0] = x; data.f[1] = y; data.f[2] = z; data.f[3] = w; num_values = 4; type = Type::Float; }
	UniformValue::UniformValue(int value) { data.i[0] = value; num_values = 1; type = Type::Int; }
	UniformValue::UniformValue(unsigned int x, unsigned int y) { data.u[0] = x; data.u[1] = y; num_values = 2; type = Type::UInt; }
	UniformValue::UniformValue(const Image* value) { data.image_address = value; num_values = 1; type = Type::Image; }
	UniformValue::UniformValue(const Mat44& value) { static_assert(sizeof(Mat44) == sizeof(float) * 16, ""); CopyMemory(data.f, &value, sizeof(float) * 16); num_values = 1; type = Type::Mat44; }

	Material::Material(Program& program) : program(program) {}

	void Material::set(const char* name, float value) { uniforms[program.uniform(name)] = UniformValue(value); }
	void Material::set(const char* name, float x, float y) { uniforms[program.uniform(name)] = UniformValue(x, y); }
	void Material::set(const char* name, float x, float y, float z) { uniforms[program.uniform(name)] = UniformValue(x, y, z); }
	void Material::set(const char* name, float x, float y, float z, float w) { uniforms[program.uniform(name)] = UniformValue(x, y, z, w); }
	void Material::set(const char* name, int value) { uniforms[program.uniform(name)] = UniformValue(value); }
	void Material::set(const char* name, unsigned int x, unsigned int y) { uniforms[program.uniform(name)] = UniformValue(x, y); }
	void Material::set(const char* name, const Image* value) { uniforms[program.uniform(name)] = UniformValue(value); }
	void Material::set(const char* name, const Mat44& value) { uniforms[program.uniform(name)] = UniformValue(value); }

	void Material::use() const {
		program.use();
		int texture_cursor = 0;
		for (const auto& pair : uniforms) {
			if (pair.first == -1) continue;
			switch (pair.second.type) {
			case UniformValue::Type::Int:
			{
				switch (pair.second.num_values) {
				case 1:
					glUniform1iv(pair.first, 1, pair.second.data.i); break;
				case 2:
					glUniform2iv(pair.first, 1, pair.second.data.i); break;
				case 3:
					glUniform3iv(pair.first, 1, pair.second.data.i); break;
				case 4:
					glUniform4iv(pair.first, 1, pair.second.data.i); break;
				}
				break;
			}
			case UniformValue::Type::UInt:
			{
				switch (pair.second.num_values) {
				case 1:
					glUniform1uiv(pair.first, 1, pair.second.data.u); break;
				case 2:
					glUniform2uiv(pair.first, 1, pair.second.data.u); break;
				case 3:
					glUniform3uiv(pair.first, 1, pair.second.data.u); break;
				case 4:
					glUniform4uiv(pair.first, 1, pair.second.data.u); break;
				}
				break;
			}
			case UniformValue::Type::UInt:
			{
				switch (pair.second.num_values) {
				case 1:
					glUniform1uiv(pair.first, 1, (const unsigned int*)pair.second.data); break;
				case 2:
					glUniform2uiv(pair.first, 1, (const unsigned int*)pair.second.data); break;
				case 3:
					glUniform3uiv(pair.first, 1, (const unsigned int*)pair.second.data); break;
				case 4:
					glUniform4uiv(pair.first, 1, (const unsigned int*)pair.second.data); break;
				}
				break;
			}
			case UniformValue::Type::Float:
			{
				switch (pair.second.num_values) {
				case 1:
					glUniform1fv(pair.first, 1, pair.second.data.f); break;
				case 2:
					glUniform2fv(pair.first, 1, pair.second.data.f); break;
				case 3:
					glUniform3fv(pair.first, 1, pair.second.data.f); break;
				case 4:
					glUniform4fv(pair.first, 1, pair.second.data.f); break;
				}
				break;
			}
			case UniformValue::Type::Image:
			{
				glActiveTexture(GL_TEXTURE0 + texture_cursor);
				pair.second.data.image_address->bind();
				glUniform1i(pair.first, texture_cursor);
				texture_cursor += 1;
				break;
			}
			case UniformValue::Type::Mat44:
			{
				TT::Assert(pair.second.num_values == 1);
				glUniformMatrix4fv(pair.first, 1, false, pair.second.data.f);
				break;
			}
			}
		}
	}

	ComputeMaterial::ComputeMaterial(Program& program, int workgroup_size_x, int workgroup_size_y, int workgroup_size_z) :
		workgroup_size_x(workgroup_size_x) , workgroup_size_y(workgroup_size_y), workgroup_size_z(workgroup_size_z), Material(program){
	}

	void ComputeMaterial::use_and_dispatch(int work_units_x, int work_units_y, int work_units_z, unsigned int barrier_bits) {
		use();
		int gx = work_units_x / workgroup_size_x + (((work_units_x % workgroup_size_x) != 0) ? 1 : 0);
		int gy = work_units_y / workgroup_size_y + (((work_units_y % workgroup_size_y) != 0) ? 1 : 0);
		int gz = work_units_z / workgroup_size_z + (((work_units_z % workgroup_size_z) != 0) ? 1 : 0);
		glDispatchCompute(gx, gy, gz);
		if(barrier_bits != 0) glMemoryBarrier(barrier_bits);
	}

	int MeshAttribute::SizeOf() const {
		switch (elementType) {
		case GL_FLOAT:
			return sizeof(float) * dimensions;
		default:
			AssertFatal(false);
			return 0;
		}
	}

	__forceinline void Mesh::init(const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type) {
		this->primitive_type = primitive_type;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		vbo.bind();
		ibo.bind();
		int stride = 0;
		for (const TT::MeshAttribute& attr : attrs)
			stride += attr.SizeOf();
		int i = 0;
		size_t offset = 0;
		for (const TT::MeshAttribute& attr : attrs) {
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, attr.dimensions, attr.elementType, GL_FALSE, stride, (void*)offset);
			i += 1;
			offset += attr.SizeOf();
		}
	}

	Mesh::Mesh(Mesh&& other) : vbo(std::move(other.vbo)), ibo(std::move(other.ibo)) {
		vao = other.vao;
		other.vao = 0;
		primitive_type = other.primitive_type;
		index_type = other.index_type;
		index_count = other.index_count;
	}

	Mesh& Mesh::operator=(Mesh&& other) {
		vbo = std::move(other.vbo);
		ibo = std::move(other.ibo);
		vao = other.vao;
		other.vao = 0;
		primitive_type = other.primitive_type;
		index_type = other.index_type;
		index_count = other.index_count;

		return *this;
	}

	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	Mesh::Mesh(const std::initializer_list<float>& vertices, const std::initializer_list<unsigned int>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode) :
		vbo(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (const char*)vertices.begin(), mode),
		ibo(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), (const char*)indices.begin(), mode),
		index_type(GL_UNSIGNED_INT), 
		index_count((GLsizei)indices.size()) {
		init(attrs, primitive_type);
		glBindVertexArray(0);
	}

	Mesh::Mesh(const std::initializer_list<float>& vertices, const std::initializer_list<unsigned short>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode) :
		vbo(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (const char*)vertices.begin(), mode),
		ibo(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), (const char*)indices.begin(), mode),
		index_type(GL_UNSIGNED_SHORT), 
		index_count((GLsizei)indices.size()) {
		init(attrs, primitive_type);
		glBindVertexArray(0);
	}

	Mesh::Mesh(const std::initializer_list<float>& vertices, const std::initializer_list<unsigned char>& indices, const std::initializer_list<MeshAttribute>& attrs, unsigned int primitive_type, unsigned int mode) :
		vbo(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), (const char*)vertices.begin(), mode),
		ibo(GL_ELEMENT_ARRAY_BUFFER, indices.size(), (const char*)indices.begin(), mode),
		index_type(GL_UNSIGNED_BYTE), 
		index_count((GLsizei)indices.size()) {
		init(attrs, primitive_type);
		glBindVertexArray(0);
	}

	void Mesh::draw() const {
		glBindVertexArray(vao);
		glDrawElements(primitive_type, index_count, index_type, 0);
		glBindVertexArray(0);
	}

	void Mesh::resize_vbo(unsigned int num_floats) { vbo.set_data(sizeof(float) * num_floats, nullptr); }
	void Mesh::resize_ibo(unsigned int num_indices) {
		switch (index_type) {
		case GL_UNSIGNED_BYTE:
			ibo.set_data(num_indices, nullptr);
			break;
		case GL_UNSIGNED_SHORT:
			ibo.set_data(sizeof(short) * num_indices, nullptr);
			break;
		case GL_UNSIGNED_INT:
			ibo.set_data(sizeof(unsigned int) * num_indices, nullptr);
			break;
		default:
			AssertFatal(false);
		}
		index_count = num_indices;
	}
}
