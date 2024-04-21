#include "vlpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Violet {
	Renderer::SceneData* Renderer::m_SceneData = new SceneData();

	void Renderer::Init() 
	{
		VL_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}
}
