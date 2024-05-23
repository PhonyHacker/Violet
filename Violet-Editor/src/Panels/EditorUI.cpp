#include "vlpch.h"
#include "EditorUI.h"

#include "Violet/Math/Math.h"

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Violet {
	void EditorUI::Attach(EditorLayer* editor)
	{
		m_Editor = editor;

		m_IconPlay = Texture2D::Create("assets/icons/PlayButton.png");
		m_IconPause = Texture2D::Create("assets/icons/PauseButton.png");
		m_IconSimulate = Texture2D::Create("assets/icons/SimulateButton.png");
		m_IconStep = Texture2D::Create("assets/icons/StepButton.png");
		m_IconStop = Texture2D::Create("assets/icons/StopButton.png");

		m_SceneHierarchyPanel.SetContext(editor->GetActiveScene());
	}

	void EditorUI::OnLoadProject()
	{
		m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
	}

	float curTime = 0.0f;
	int frameCounts = 0;
	int FPS = 0;
	void EditorUI::StatusPanel() 
	{
		ImGui::Begin("States");

		auto stats = Violet::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		float step = m_Editor->GetTimeStep();
		curTime += step;
		frameCounts++;

		if (curTime >= 1.0f)
		{
			curTime = 0.0f;
			FPS = frameCounts;
			frameCounts = 0;
		}
		ImGui::Text("FPS: %d", FPS);
		//ImGui::Text("FPS: %s", std::to_string(1 / editor->GetTimeStep()).c_str());

		ImGui::End();
	}

	void EditorUI::MenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					m_Editor->OpenProject();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					m_Editor->NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					m_Editor->SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					m_Editor->SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
					ScriptEngine::ReloadAssembly();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}
	
	void EditorUI::Settings()
	{
		ImGui::Begin("Settings");
		ImGui::Checkbox("Show physics colliders", &(m_Editor->IsShowCollider()));
		ImGui::Checkbox("Show Gizmo", &(m_Editor->IsShowGizmo()));
		//ImGui::Image((ImTextureID)s_Font->GetAtlasTexture()->GetRendererID(), { 512,512 }, { 0, 1 }, { 1, 0 });

		ImGui::End();
	}

	void EditorUI::ViewPort()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_Editor->ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_Editor->ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_Editor->GetViewSize() = {viewportPanelSize.x, viewportPanelSize.y};

		uint32_t textureID = m_Editor->GetFramebuffer()->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ m_Editor->GetViewSize().x, m_Editor->GetViewSize().y }, { 0, 1 }, { 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::wstring wpath(path);
				std::wstring extension = L".violet";

				// 检查路径长度是否足够长
				if (wpath.size() >= extension.size())
				{
					// 比较路径的后缀是否为 ".violet"
					if (wpath.compare(wpath.size() - extension.size(), extension.size(), extension) == 0)
						m_Editor->OpenScene(path);

					// 输出文件路径 
					std::locale loc("");

					std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
					std::string str = converter.to_bytes(path);

					std::cout << "文件路径: " << str << std::endl;
				}

			}
			ImGui::EndDragDropTarget();
		}

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1 && m_Editor->IsShowGizmo())
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			auto bounds = m_Editor->ViewportBounds;
			ImGuizmo::SetRect(bounds[0].x, bounds[0].y, bounds[1].x - bounds[0].x, bounds[1].y - bounds[0].y);

			// Camera
			// --Editor
			const glm::mat4& cameraProjection = m_Editor->GetEditorCamera().GetProjection();
			glm::mat4 cameraView = glm::inverse(m_Editor->GetEditorCamera().GetViewMatrix());

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType  == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };


			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorUI::UIToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		bool toolbarEnabled = (bool)m_Editor->GetActiveScene();

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		bool hasPlayButton = m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Play;
		bool hasSimulateButton = m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Simulate;
		bool hasPauseButton = m_Editor->GetSceneState() != SceneState::Edit;

		if (hasPlayButton)
		{
			Ref<Texture2D> icon = (m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Simulate)
					m_Editor->OnScenePlay();
				else if (m_Editor->GetSceneState() == SceneState::Play)
					m_Editor->OnSceneStop();
			}
		}
		if (hasSimulateButton)
		{
			if (hasPlayButton)
				ImGui::SameLine();

			Ref<Texture2D> icon = (m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Play) ? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (m_Editor->GetSceneState() == SceneState::Edit || m_Editor->GetSceneState() == SceneState::Play)
					m_Editor->OnSceneSimulate();
				else if (m_Editor->GetSceneState() == SceneState::Simulate)
					m_Editor->OnSceneStop();
			}
		}
		if (hasPauseButton)
		{
			bool isPaused = m_Editor->GetActiveScene()->IsPaused();
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = m_IconPause;
				if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					m_Editor->GetActiveScene()->SetPaused(!isPaused);
				}
			}

			// Step button
			if (isPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = m_IconStep;
					bool isPaused = m_Editor->GetActiveScene()->IsPaused();
					if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
					{
						m_Editor->GetActiveScene()->Step();
					}
				}
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorUI::StartDockSpace()
	{
		static bool dockingEnabled = true;
		if (dockingEnabled)
		{
			static bool dockspaceOpen = true;
			static bool opt_fullscreen_persistant = true;
			bool opt_fullscreen = opt_fullscreen_persistant;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// DockSpace
			ImGuiIO& io = ImGui::GetIO();

			ImGuiStyle& style = ImGui::GetStyle();
			float minWinSizeX = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;

			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			style.WindowMinSize.x = minWinSizeX;
		}
	}

	void EditorUI::StopDockSpace()
	{
		ImGui::End();
	}
	void EditorUI::HandleMouse()
	{
		auto [mx, my] = GetMousePos();

		mx -= m_Editor->ViewportBounds[0].x;
		my -= m_Editor->ViewportBounds[0].y;

		glm::vec2 viewportSize = m_Editor->ViewportBounds[1] - m_Editor->ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		// VL_TRACE("MOUSE: {0}, {1}", mouseX, mouseY);
		Application::Get().CurrentMouse = glm::vec2(mx, my);

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_Editor->GetFramebuffer()->ReadPixel(1, mouseX, mouseY);
			m_Editor->GetHoveredEntity() = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_Editor->GetActiveScene().get());
			Application::Get().HoeveredEntity = m_Editor->GetHoveredEntity();
		}
	}

	void EditorUI::OnImGuiRender()
	{
		StartDockSpace();

		MenuBar();
		StatusPanel();
		Settings();
		ViewPort();
		UIToolbar();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel->OnImGuiRender();	

		StopDockSpace();
	}
}
