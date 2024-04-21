#pragma once

#include "RenderCommand.h"

//#include "OrthographicCamera.h"
#include "Shader.h"

namespace Violet {

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData 
		{
			glm::mat4 ViewProjectionView;
		};

		static SceneData* m_SceneData;
	};
}
