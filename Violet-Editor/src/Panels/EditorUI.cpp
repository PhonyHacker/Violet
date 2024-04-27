#include "vlpch.h"
#include "EditorUI.h"

#include "Violet/Math/Math.h"

#include <glm/glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Violet {
	void EditorUI::Attach(EditorLayer* editor)
	{
		m_IconPlay = Texture2D::Create("assets/icons/PlayButton.png");
		m_IconPause = Texture2D::Create("assets/icons/PauseButton.png");
		m_IconSimulate = Texture2D::Create("assets/icons/SimulateButton.png");
		m_IconStep = Texture2D::Create("assets/icons/StepButton.png");
		m_IconStop = Texture2D::Create("assets/icons/StopButton.png");

		m_SceneHierarchyPanel.SetContext(editor->GetActiveScene());
	}

	void EditorUI::OnLoadProject(EditorLayer* editor)
	{
		m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
	}

	float curTime = 0.0f;
	int frameCounts = 0;
	int FPS = 0;
	void EditorUI::StatusPanel(EditorLayer* editor) 
	{
		ImGui::Begin("States");

		auto stats = Violet::Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		float step = editor->GetTimeStep();
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

	void EditorUI::MenuBar(EditorLayer* editor)
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
					editor->OpenProject();

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					editor->NewScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					editor->SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					editor->SaveSceneAs();

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
	
	void EditorUI::Settings(EditorLayer* editor)
	{
		ImGui::Begin("Settings");
		ImGui::Checkbox("Show physics colliders", &(editor->IsShowCollider()));
		//ImGui::Image((ImTextureID)s_Font->GetAtlasTexture()->GetRendererID(), { 512,512 }, { 0, 1 }, { 1, 0 });

		ImGui::End();
	}

	void EditorUI::ViewPort(EditorLayer* editor)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		editor->GetViewBounds()[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		editor->GetViewBounds()[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		editor->GetViewSize() = {viewportPanelSize.x, viewportPanelSize.y};

		uint32_t textureID = editor->GetFramebuffer()->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ editor->GetViewSize().x, editor->GetViewSize().y }, { 0, 1 }, { 1, 0 });

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
						editor->OpenScene(path);

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
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			auto bounds = editor->GetViewBounds();
			ImGuizmo::SetRect(bounds[0].x, bounds[0].y, bounds[1].x - bounds[0].x, bounds[1].y - bounds[0].y);

			// Camera
			// --Runtime
			// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			// const glm::mat4& cameraProjection = camera.GetProjection();
			// glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Camera
			// --Editor
			const glm::mat4& cameraProjection = editor->GetEditorCamera().GetProjection();
			glm::mat4 cameraView = editor->GetEditorCamera().GetViewMatrix();

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

	void EditorUI::UIToolbar(EditorLayer* editor)
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

		bool toolbarEnabled = (bool)editor->GetActiveScene();

		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5f;

		float size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		bool hasPlayButton = editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Play;
		bool hasSimulateButton = editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Simulate;
		bool hasPauseButton = editor->GetSceneState() != SceneState::Edit;

		if (hasPlayButton)
		{
			Ref<Texture2D> icon = (editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Simulate)
					editor->OnScenePlay();
				else if (editor->GetSceneState() == SceneState::Play)
					editor->OnSceneStop();
			}
		}
		if (hasSimulateButton)
		{
			if (hasPlayButton)
				ImGui::SameLine();

			Ref<Texture2D> icon = (editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Play) ? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				if (editor->GetSceneState() == SceneState::Edit || editor->GetSceneState() == SceneState::Play)
					editor->OnSceneSimulate();
				else if (editor->GetSceneState() == SceneState::Simulate)
					editor->OnSceneStop();
			}
		}
		if (hasPauseButton)
		{
			bool isPaused = editor->GetActiveScene()->IsPaused();
			ImGui::SameLine();
			{
				Ref<Texture2D> icon = m_IconPause;
				if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					editor->GetActiveScene()->SetPaused(!isPaused);
				}
			}

			// Step button
			if (isPaused)
			{
				ImGui::SameLine();
				{
					Ref<Texture2D> icon = m_IconStep;
					bool isPaused = editor->GetActiveScene()->IsPaused();
					if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
					{
						editor->GetActiveScene()->Step();
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

	void EditorUI::OnImGuiRender(EditorLayer* editor)
	{
		StartDockSpace();

		MenuBar(editor);
		StatusPanel(editor);
		Settings(editor);
		ViewPort(editor);
		UIToolbar(editor);

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel->OnImGuiRender();	

		StopDockSpace();
	}
}
