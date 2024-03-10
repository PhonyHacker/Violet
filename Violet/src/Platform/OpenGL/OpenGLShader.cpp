#include "vlpch.h"
#include "OpenGLShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Violet {
	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		// 创建顶点着色器空句柄
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// 添加顶点着色器源代码至GL
		const GLchar* source = vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		// 编译顶点着色器
		glCompileShader(vertexShader);

		// 顶点着色器编译错误处理
		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);

			VL_CORE_ERROR("{0}", infoLog.data());
			VL_CORE_ASSERT(false, "Vertex shader compilation failure!");
			return;
		}

		// 创建片段着色器空句柄
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// 发送片段着色器源代码至GL
		source = fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		// 编译片段着色器
		glCompileShader(fragmentShader);

		// 片段编译错误处理
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			glDeleteShader(vertexShader);

			VL_CORE_ERROR("{0}", infoLog.data());
			VL_CORE_ASSERT(false, "Fragment shader compilation failure!");
			return;
		}
		
		// 获取项目ID
		m_RendererID = glCreateProgram();
		GLuint program = m_RendererID;

		// 将着色器以项目链接
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		// 链接整个项目
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			VL_CORE_ERROR("{0}", infoLog.data());
			VL_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		// 分离着色器对象
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);

	}
	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}
	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2 value){
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}
	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3 value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}
	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4 value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.r, value.g, value.b, value.a);
	}
	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3 value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4 value) {
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}
}