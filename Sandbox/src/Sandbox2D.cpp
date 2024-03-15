#include "vlpch.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1200.0f / 800.0f, true){}

void Sandbox2D::OnAttach()
{
	VL_PROFILE_FUNCTION();

	m_CheckerboardTexture = Violet::Texture2D::Create("assets/textures/test.png");
	m_TextureAltas = Violet::Texture2D::Create("assets/textures/TextureAltas.png");

	// Particle Init
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.1f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

}

void Sandbox2D::OnDetach()
{
	VL_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Violet::Timestep timestep)
{
	VL_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(timestep);
	
	Violet::Renderer2D::ResetStats();

	{
		// Render
		VL_PROFILE_SCOPE("Renderer Prep");
		Violet::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Violet::RenderCommand::Clear();
	}
#if 0
	{
		static float rotation = 0.0f;
		rotation += timestep * 50.0f;
		if (rotation > 360)
			rotation -= 360;

		VL_PROFILE_SCOPE("Renderer Draw");
		Violet::Renderer2D::BeginScene(m_CameraController.GetCamera());
		// Violet::Renderer2D::DrawRotatedQuad({ 1.0f, 0.0f }, { 0.8f, 0.8f }, -45.0f, { 0.8f, 0.2f, 0.3f, 1.0f });
		// Violet::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
		// Violet::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Violet::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_CheckerboardTexture, 10.0f);
		Violet::Renderer2D::DrawRotatedQuad({ -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_CheckerboardTexture);
		Violet::Renderer2D::DrawRotatedQuad({ 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_CheckerboardTexture);
		
		Violet::Renderer2D::EndScene();
		
		Violet::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.2f, (y + 5.0f) / 10.0f, 0.7f };
				Violet::Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			}
		}

		Violet::Renderer2D::EndScene();
	}
#endif
	{
		// 粒子系统demo
		if (Violet::Input::IsMouseButtonPressed(VL_MOUSE_BUTTON_LEFT))
		{
			auto [x, y] = Violet::Input::GetMousePosition();
			auto width = Violet::Application::Get().GetWindow().GetWidth();
			auto height = Violet::Application::Get().GetWindow().GetHeight();

			auto bounds = m_CameraController.GetBounds();
			auto pos = m_CameraController.GetCamera().GetPosition();
			x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
			y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
			m_Particle.Position = { x + pos.x, y + pos.y };
			for (int i = 0; i < 5; i++)
				m_ParticleSystem.Emit(m_Particle);
		}
		m_ParticleSystem.OnUpdate(timestep);
		m_ParticleSystem.OnRender(m_CameraController.GetCamera());
	}

	{
		// 绘制纹理集的一个
		Violet::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Violet::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.4f }, { 1.0f, 1.0f }, m_TextureAltas, 1.0f);
		Violet::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	VL_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Violet::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::End();
}

void Sandbox2D::OnEvent(Violet::Event& e)
{
	m_CameraController.OnEvent(e);
}
