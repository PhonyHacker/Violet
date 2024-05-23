#include "vlpch.h"
#include "OpenGLShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

//#include <shaderc/shaderc.hpp>
//#include <spirv_cross/spirv_cross.hpp>
//#include <spirv_cross/spirv_glsl.hpp>

#include <Violet/Core/Timer.h>

#include <fstream>
#include <filesystem>

namespace Violet {
	namespace Utils {
		// 将字符串类型的着色器类型转换为OpenGL的GLenum类型
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			VL_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}
	}

	// 构造函数：从文件加载着色器源码，进行预处理并编译
	OpenGLShader::OpenGLShader(const std::string& filepath) 
		: m_FilePath(filepath)
	{
		VL_PROFILE_FUNCTION();

		//Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		
		Compile(shaderSources);

		auto nameBegin = filepath.find_last_of("/\\");
		nameBegin = nameBegin == std::string::npos ? 0 : nameBegin + 1;
		auto nameEnd = filepath.rfind(".");
		auto count = nameEnd == std::string::npos ? filepath.size() - nameBegin : nameEnd - nameBegin;

		m_Name = filepath.substr(nameBegin, count);
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		VL_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		/*
			std::vector<GLenum> glShaderIDs(shaderSources.size());
			glShaderIDs.reserve(2);
		*/
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources) {
			GLenum type = kv.first;
			const std::string& source = kv.second;

			// Create an empty vertex shader handle
			GLuint shader = glCreateShader(type);

			// Send the vertex shader source code to GL
			// Note that std::string's .c_str is NULL character terminated.
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				//glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				glGetShaderInfoLog(shader, maxLength - 1, nullptr, infoLog.data());

				// We don't need the shader anymore.
				glDeleteShader(shader);

				// Use the infoLog as you see fit.

				// In this simple program, we'll just leave
				VL_CORE_ERROR("{0} ", infoLog.data());
				VL_CORE_ASSERT(false, "shader 编译失败!");

				break;
			}

			// Attach our shaders to our program
			glAttachShader(program, shader);

			//glShaderIDs.push_back(shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		m_RendererID = program;

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			for (auto id : glShaderIDs) {
				glDeleteShader(id);
			}
			// Use the infoLog as you see fit.
			// In this simple program, we'll just leave
			VL_CORE_ERROR("{0} ", infoLog.data());
			VL_CORE_ASSERT(false, "shader link failure!");
			return;
		}

		// Always detach shaders after a successful link.
		for (auto id : glShaderIDs) {
			glDetachShader(program, id);
		}
	}

	// 构造函数：从源码字符串创建着色器
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) 
		:m_Name(name)
	{
		VL_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		
		//CompileOrGetVulkanBinaries(sources);
		//CompileOrGetOpenGLBinaries();
		//CreateProgram();

		Compile(sources);

	}

	// 析构函数：释放着色器程序资源
	OpenGLShader::~OpenGLShader()
	{
		VL_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	// 从文件中读取着色器源码
	std::string OpenGLShader::ReadFile(const std::string& filepath) 
	{
		VL_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
			else
			{
				VL_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			VL_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	// 对源码进行预处理，解析不同类型的着色器
	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		VL_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";

		size_t typeTokenLength = strlen(typeToken);

		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			VL_CORE_ASSERT(eol != std::string::npos, "Syntax error");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			VL_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			VL_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos);
			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	// 绑定着色器程序
	void OpenGLShader::Bind() const
	{
		VL_PROFILE_FUNCTION(); // 性能分析

		// 使用着色器程序
		glUseProgram(m_RendererID);
	}

	// 解绑着色器程序
	void OpenGLShader::Unbind() const
	{
		VL_PROFILE_FUNCTION(); // 性能分析

		// 使用默认着色器程序（0代表没有程序）
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		VL_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int value) 
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}
	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}
	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2 value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}
	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3 value) 
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}
	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4 value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.r, value.g, value.b, value.a);
	}
	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3 value) 
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4 value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}
}
