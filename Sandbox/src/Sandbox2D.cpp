#include "vlpch.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>

/*
0不绘制
1是墙壁
2是草地
3是箱子
*/
static const uint32_t s_MapWidth = 20;
static const char* s_MapTiles =
				"00000000000000000000"
				"01111111111111111100"
				"01222312332312323100"
				"01213111121111213100"
				"01212222222222212100"
				"01211211121112113100"
				"01213233333332213100"
				"01331212313212133100"
				"01111232111232311100"
				"01113212313212132100"
				"01311233333332112100"
				"01321212121312333100"
				"01313211121112113100"
				"01311222222222212100"
				"01211131121111312100"
				"01213131121111212100"
				"01223333233333322100"
				"01111111111111111100"
				"00000000000000000000"
				"00000000000000000000"
;

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1200.0f / 800.0f, true){}

void Sandbox2D::OnAttach()
{
	VL_PROFILE_FUNCTION();

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;

	m_CheckerboardTexture = Violet::Texture2D::Create("assets/textures/test.png");

	m_TextureAltas = Violet::Texture2D::Create("assets/textures/TextureAltas.png");

	Violet::FramebufferSpecification fbSpec;
	fbSpec.Width = 1200;
	fbSpec.Height = 720;
	m_Framebuffer = Violet::Framebuffer::Create(fbSpec);

	//m_TextureStairs, m_TextureTree, m_TextureBush
	m_TextureStair = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 7, 6 }, { 128, 128 });
	m_TextureBush = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 2, 3 }, { 128, 128 });
	m_TextureTree = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 4, 1 }, { 128, 128 }, { 1,2 });

	s_TextureMap['1'] = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 10, 8 }, { 128, 128 });
	s_TextureMap['2'] = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 1, 11 }, { 128, 128 });
	s_TextureMap['3'] = Violet::SubTexture2D::CreateFromCoords(m_TextureAltas, { 11, 11 }, { 128, 128 });

	// 粒子系统初始化
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.1f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	// 拉远摄像机
	m_CameraController.SetZoomLevel(8.0f);
}

void Sandbox2D::OnDetach()
{
	VL_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Violet::Timestep timestep)
{
	VL_PROFILE_FUNCTION();
	// VL_TRACE("FPS: {0} ({1} / ms) ", 1/timestep, timestep);

	// Update
	m_CameraController.OnUpdate(timestep);
	
	Violet::Renderer2D::ResetStats();

	{
		// Render
		VL_PROFILE_SCOPE("Renderer Prep");
		m_Framebuffer->Bind();
		Violet::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Violet::RenderCommand::Clear();
	}

// 基本渲染API
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

// 粒子系统Demo
#if 0
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
#endif


	{
		// 绘制纹理集的一个		
		
		Violet::Renderer2D::BeginScene(m_CameraController.GetCamera());
		// Violet::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureBush, 1.0f);
		// Violet::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureStair, 1.0f);
		// Violet::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.9f }, { 1.0f, 2.0f }, m_TextureTree, 1.0f);
		
		Violet::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 30.0f, 30.0f }, m_CheckerboardTexture, 10.0f);


		for (uint32_t y = 0; y < m_MapHeight; y++) {
			for (uint32_t x = 0; x < m_MapWidth; x++) {
				// x + y * m_MapWidth; 切割成2维数组
				char titleType = s_MapTiles[x + y * m_MapWidth];
				if (titleType == '0') { // 0的东西不画
					continue;
				}
				Violet::Ref<Violet::SubTexture2D> texture;
				if (s_TextureMap.find(titleType) != s_TextureMap.end())
				{
					texture = s_TextureMap[titleType];
				}
				else 
				{
					texture = m_TextureBush;
				}
				//	Violet::Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, m_MapHeight / 2.0f - y, 0.5f }, { 1.0f, 1.0f }, texture); // x - m_MapWidth / 2.0f,  y - m_MapHeight / 2.0f, 0.5f // 会导致y轴相反绘画
			}
		}
		

		Violet::Renderer2D::EndScene();
		m_Framebuffer->Unbind();
	}
}

void Sandbox2D::OnImGuiRender()
{
	VL_PROFILE_FUNCTION();

	// Note: Switch this to true to enable dockspace
	static bool dockingEnabled = true;
	if (dockingEnabled)
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Exit")) Violet::Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");

		auto stats = Violet::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

		ImGui::End();

		ImGui::Begin("Image: ");

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ 1200.0f, 800.0f });

		ImGui::End();

		ImGui::End();
	}
	else
	{
		ImGui::Begin("Settings");

		auto stats = Violet::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

		uint32_t textureID = m_CheckerboardTexture->GetRendererID();
		ImGui::Image((void*)textureID, ImVec2{ 256.0f, 256.0f });
		ImGui::End();
	}
}

void Sandbox2D::OnEvent(Violet::Event& e)
{
	m_CameraController.OnEvent(e);
}
