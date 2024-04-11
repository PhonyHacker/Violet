#include "vlpch.h"
#include "OpenGLShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

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

		// 将OpenGL着色器类型转换为shaderc的着色器类型
		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			VL_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		// 将OpenGL着色器类型转换为字符串形式
		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			VL_CORE_ASSERT(false);
			return nullptr;
		}

		// 获取着色器缓存目录
		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		// 创建着色器缓存目录（如果不存在）
		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		// 根据OpenGL着色器类型获取缓存文件的扩展名
		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			}
			VL_CORE_ASSERT(false);
			return "";
		}

		// 根据OpenGL着色器类型获取Vulkan缓存文件的扩展名
		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			VL_CORE_ASSERT(false);
			return "";
		}
	}

	// 构造函数：从文件加载着色器源码，进行预处理并编译
	OpenGLShader::OpenGLShader(const std::string& filepath) 
		: m_FilePath(filepath)
	{
		VL_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		
		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			VL_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
		}

		auto nameBegin = filepath.find_last_of("/\\");
		nameBegin = nameBegin == std::string::npos ? 0 : nameBegin + 1;
		auto nameEnd = filepath.rfind(".");
		auto count = nameEnd == std::string::npos ? filepath.size() - nameBegin : nameEnd - nameBegin;

		m_Name = filepath.substr(nameBegin, count);
	}

	// 构造函数：从源码字符串创建着色器
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) 
		:m_Name(name)
	{
		VL_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		
		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
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

	// 编译或获取Vulkan二进制文件
	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					VL_CORE_ERROR(module.GetErrorMessage());
					VL_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	// 编译或获取OpenGL二进制文件
	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					VL_CORE_ERROR(module.GetErrorMessage());
					VL_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	// 创建着色器程序
	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			VL_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	// 从SPIR-V反射着色器资源信息，包括 uniform 缓冲和采样器等
	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		// 创建SPIR-V编译器并加载着色器数据
		spirv_cross::Compiler compiler(shaderData);
		// 获取着色器资源
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		// 输出反射信息
		VL_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		VL_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		VL_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		// 输出 uniform 缓冲详细信息
		VL_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			// 获取缓冲类型信息
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			// 计算缓冲大小
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			// 获取绑定点
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			// 获取成员数量
			int memberCount = bufferType.member_types.size();

			// 输出缓冲名称、大小、绑定点和成员数量
			VL_CORE_TRACE("  {0}", resource.name);
			VL_CORE_TRACE("    Size = {0}", bufferSize);
			VL_CORE_TRACE("    Binding = {0}", binding);
			VL_CORE_TRACE("    Members = {0}", memberCount);
		}
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