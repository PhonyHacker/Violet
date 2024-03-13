#include "vlpch.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1200.0f / 800.0f, true){}

void Sandbox2D::OnAttach()
{
	m_CheckerboardTexture = Violet::Texture2D::Create("assets/textures/test.png");
}

void Sandbox2D::OnDetach()
{
}

void Sandbox2D::OnUpdate(Violet::Timestep timestep)
{
	// Update
	m_CameraController.OnUpdate(timestep);

	// Render
	Violet::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Violet::RenderCommand::Clear();

	Violet::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Violet::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	Violet::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
	Violet::Renderer2D::DrawQuad({ 0.0f, 0.4f, -0.f }, { 1.0f, 1.0f }, m_CheckerboardTexture);

	Violet::Renderer2D::EndScene();

}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();

}

void Sandbox2D::OnEvent(Violet::Event& e)
{
	m_CameraController.OnEvent(e);
}
