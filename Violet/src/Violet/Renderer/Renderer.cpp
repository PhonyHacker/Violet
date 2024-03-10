#include "vlpch.h"
#include "Renderer.h"
#include "RenderCommand.h"

namespace Violet {
	Renderer::SceneData* Renderer::m_SceneData = new SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionView = camera.GetViewProjectionMartix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const std::shared_ptr<Shader> shader, const std::shared_ptr<VertexArray>& vertexArray)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionView);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}