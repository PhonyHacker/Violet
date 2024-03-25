#include "vlpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Violet {
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}