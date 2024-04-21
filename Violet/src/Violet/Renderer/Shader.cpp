#include "vlpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Violet {
	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    VL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLShader>(filepath);
		}

		VL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
		switch (Renderer::GetAPI()) {
			case RendererAPI::API::None:
				VL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:
				return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		VL_CORE_ASSERT(false, "Unkown RendererAPI!");
		return nullptr;
	}
}
