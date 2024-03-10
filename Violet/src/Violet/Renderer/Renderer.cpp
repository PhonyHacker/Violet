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

	void Renderer::Submit(const std::shared_ptr<Shader> shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4 transform)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionView);
		shader->UploadUniformMat4("u_Transform", transform);
	
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}