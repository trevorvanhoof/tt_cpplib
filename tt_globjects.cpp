#include "tt_globjects.h"
#include "tt_messages.h"

#include "tt_gl_cpp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace TT {
	Image::Image(Image&& other) { handle = other.handle; other.handle = 0; }
	Image& Image::operator=(Image&& other) { handle = other.handle; other.handle = 0; return *this; }
	Image::~Image() { glDeleteTextures(1, &handle); handle = 0; }

	Image::Image(const char* file) {
		glGenTextures(1, &handle);
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char* data = stbi_load(file, &width, &height, &channels, 0);
		switch (channels) {
		case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data); break;
		case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data); break;
		case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); break;
		case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); break;
		default: AssertFatal(false); break;
		}
		stbi_image_free(data);
	}

	Image::Image(int width, int height, int channels, const char* data) :
		width(width), height(height), channels(channels) {
		glGenTextures(1, &handle);
		bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		switch (channels) {
		case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data); break;
		case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data); break;
		case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); break;
		case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); break;
		default: AssertFatal(false); break;
		}
	}

	void Image::bind() const { glBindTexture(GL_TEXTURE_2D, handle); }

	unsigned int Image::raw_gl_handle() { return handle; }
	int Image::get_width() { return width; }
	int Image::get_height() { return height; }
	int Image::get_channels() { return channels; }

	void Image::set_data(const void* data) {
		static const unsigned int channel_map[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, channel_map[get_channels() - 1], get_width(), get_height(), 0, channel_map[get_channels() - 1], GL_UNSIGNED_BYTE, data);
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

	Program::Program(Program&& other) { handle = other.handle; other.handle = 0; }
	Program& Program::operator=(Program&& other) { handle = other.handle; other.handle = 0; return *this; }
	Program::~Program() { glDeleteProgram(handle); handle = 0; }

	Program::Program(std::initializer_list<Shader> shaders, int num_shaders) {
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

	UniformValue::UniformValue(UniformValue&& other) {
		type = other.type;
		num_values = other.num_values;
		data = other.data;
		other.type = Type::Invalid;
		other.num_values = 0;
		other.data = nullptr;
	}
	UniformValue& UniformValue::operator=(UniformValue&& other) {
		type = other.type;
		num_values = other.num_values;
		data = other.data;
		other.type = Type::Invalid;
		other.num_values = 0;
		other.data = nullptr;
		return *this;
	}
	UniformValue::~UniformValue() {
		if (type != Type::Image) {
			delete[] data;
		}
		data = nullptr;
	};
	UniformValue::UniformValue() {}
	UniformValue::UniformValue(float value) { data = new float[] { value }; num_values = 1; type = Type::Float; }
	UniformValue::UniformValue(float x, float y) { data = new float[] { x, y }; num_values = 2; type = Type::Float; }
	UniformValue::UniformValue(float x, float y, float z) { data = new float[] { x, y, z }; num_values = 3; type = Type::Float; }
	UniformValue::UniformValue(float x, float y, float z, float w) { data = new float[] { x, y, z, w }; num_values = 4; type = Type::Float; }
	UniformValue::UniformValue(int value) { data = new int[] { value }; num_values = 1; type = Type::Int; }
	UniformValue::UniformValue(Image& value) { data = &value; num_values = 1; type = Type::Image; }

	Material::Material(Program& program) : program(program) {}

	void Material::set(const char* name, float value) { uniforms[program.uniform(name)] = UniformValue(value); }
	void Material::set(const char* name, float x, float y) { uniforms[program.uniform(name)] = UniformValue(x, y); }
	void Material::set(const char* name, float x, float y, float z) { uniforms[program.uniform(name)] = UniformValue(x, y, z); }
	void Material::set(const char* name, float x, float y, float z, float w) { uniforms[program.uniform(name)] = UniformValue(x, y, z, w); }
	void Material::set(const char* name, int value) { uniforms[program.uniform(name)] = UniformValue(value); }
	void Material::set(const char* name, Image& value) { uniforms[program.uniform(name)] = UniformValue(value); }

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
					glUniform1iv(pair.first, 1, (const int*)pair.second.data); break;
				case 2:
					glUniform2iv(pair.first, 1, (const int*)pair.second.data); break;
				case 3:
					glUniform3iv(pair.first, 1, (const int*)pair.second.data); break;
				case 4:
					glUniform4iv(pair.first, 1, (const int*)pair.second.data); break;
				}
				break;
			}
			case UniformValue::Type::Float:
			{
				switch (pair.second.num_values) {
				case 1:
					glUniform1fv(pair.first, 1, (const float*)pair.second.data); break;
				case 2:
					glUniform2fv(pair.first, 1, (const float*)pair.second.data); break;
				case 3:
					glUniform3fv(pair.first, 1, (const float*)pair.second.data); break;
				case 4:
					glUniform4fv(pair.first, 1, (const float*)pair.second.data); break;
				}
				break;
			}
			case UniformValue::Type::Image:
			{
				glActiveTexture(GL_TEXTURE0 + texture_cursor);
				((Image*)pair.second.data)->bind();
				glUniform1i(pair.first, texture_cursor);
				texture_cursor += 1;
				break;
			}
			}
		}
	}

	__forceinline void Mesh::init(std::initializer_list<float> vertices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type) {
		this->primitive_type = primitive_type;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, buffers);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.begin(), GL_STATIC_DRAW);
		int i = 0;
		for (auto attr : attrs) {
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, attr.dimensions, attr.elementType, GL_FALSE, 0, nullptr);
			i += 1;
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	}

	Mesh::Mesh(Mesh&& other) {
		buffers[0] = other.buffers[0];
		buffers[1] = other.buffers[1];
		vao = other.vao;

		buffers[0] = 0;
		buffers[1] = 0;
		vao = 0;
	}

	Mesh& Mesh::operator=(Mesh&& other) {
		buffers[0] = other.buffers[0];
		buffers[1] = other.buffers[1];
		vao = other.vao;
		buffers[0] = 0;
		buffers[1] = 0;
		vao = 0;
		return *this;
	}

	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(2, buffers);
		buffers[0] = 0;
		buffers[1] = 0;
		vao = 0;
	}

	Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned int> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type) :
		index_type(GL_UNSIGNED_INT), index_count((GLsizei)indices.size()) {
		init(vertices, attrs, primitive_type);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.begin(), GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned short> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type) :
		index_type(GL_UNSIGNED_SHORT), index_count((GLsizei)indices.size()) {
		init(vertices, attrs, primitive_type);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(short), indices.begin(), GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	Mesh::Mesh(std::initializer_list<float> vertices, std::initializer_list<unsigned char> indices, std::initializer_list<MeshAttribute> attrs, unsigned int primitive_type) :
		index_type(GL_UNSIGNED_BYTE), index_count((GLsizei)indices.size()) {
		init(vertices, attrs, primitive_type);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.begin(), GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	void Mesh::draw() const {
		glBindVertexArray(vao);
		glDrawElements(primitive_type, index_count, index_type, 0);
		glBindVertexArray(0);
	}
}
