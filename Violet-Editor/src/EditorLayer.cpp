#include "vlpch.h"
#include "EditorLayer.h"

//#include <glm/glm/ext/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include "Violet/Scene/SceneSerializer.h"
#include "Violet/Math/Math.h"
#include "Violet/Utils/PlatformUtils.h"
#include "Violet/Scripting/ScriptEngine.h"
#include "Violet/Renderer/Font.h"

#include "Panels/EditorUI.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <chrono>

namespace Violet {
	static Ref<Font> s_Font;

	EditorLayer::EditorLayer(): Layer("EditorLayer")
	{
		// s_Font = new Font("assets/fonts/opensans/OpenSans-Regular.ttf");
		s_Font = Font::GetDefault();
	}

	void EditorLayer::OnAttach()
	{
		VL_PROFILE_FUNCTION();

		Violet::FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1200;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{
			// TODO: prompt the user to select a directory
			// NewProject();

			// if (!OpenProject())
			if (!OpenProject())
				Application::Get().Close();
		}
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		Renderer2D::SetLineWidth(4.0f);

		EditorUI::Get().Attach(this);
		
		// À­Ô¶ÉãÏñ»ú
		// m_CameraController.SetZoomLevel(8.0f);
	}

	void EditorLayer::OnDetach()
	{
		VL_PROFILE_FUNCTION();
	}


	void EditorLayer::OnUpdate(Violet::Timestep timestep)
	{
		VL_PROFILE_FUNCTION();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_Timestep = timestep;
		//VL_TRACE("FPS: {0} ({1} ms) ", 1/timestep, m_Timestep);

		Violet::Renderer2D::ResetStats();

		{
			// Render
			VL_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();
			Violet::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			Violet::RenderCommand::Clear();

			// Clear our entity ID attachment to -1
			m_Framebuffer->ClearAttachment(1, -1);
			switch (m_SceneState)
			{
				case SceneState::Edit:
				{
					m_EditorCamera.OnUpdate(timestep);

					m_ActiveScene->OnUpdateEditor(timestep, &m_EditorCamera);
					break;
				}
				case SceneState::Simulate:
				{
					m_EditorCamera.OnUpdate(timestep);

					m_ActiveScene->OnUpdateSimulation(timestep, &m_EditorCamera);
					break;
				}
				case SceneState::Play:
				{
					m_ActiveScene->OnUpdateRuntime(timestep);
					break;
				}
			}
		}

		// Handle Mouse Position
		EditorUI::Get().HandleMouse();

		OnOverlayRender();

		m_Framebuffer->Unbind();

	}

	void EditorLayer::OnImGuiRender()
	{
		VL_PROFILE_FUNCTION();

		EditorUI::Get().OnImGuiRender();
	}

	void EditorLayer::OnEvent(Violet::Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(VL_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(VL_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.IsRepeat())
				return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (control)
					NewScene();
				break;
			}
			case Key::O:
			{
				if (control)
					OpenProject();
				break;
			}
			case Key::S:
			{
				if (control)
				{
					if (shift)
						SaveSceneAs();
					else
						SaveScene();
				}

				break;
			}

			// Scene Commands
			case Key::D:
			{
				if (control)
					OnDuplicateEntity();

				break;
			}

			// Gizmos
			case Key::Q:
				EditorUI::Get().SetGizmoType(-1);
				break;
			case Key::W:
				EditorUI::Get().SetGizmoType(0);
				break;
			case Key::E:
				EditorUI::Get().SetGizmoType(1);
				break;
			case Key::R:
			{
				if (control)
				{
					ScriptEngine::ReloadAssembly();
				}
				else
				{
					if (!EditorUI::Get().IsGizmoUsing())
						EditorUI::Get().SetGizmoType(2);
				}
				break;
			}
			case Key::Delete:
			{
				if (Application::Get().GetImGuiLayer()->GetActiveWidgetID() == 0)
				{
					Entity selectedEntity = EditorUI::Get().GetSelectedEntity();
					if (selectedEntity)
					{
						EditorUI::Get().SetSelectedEntity({});
						m_ActiveScene->DestroyEntity(selectedEntity);
					}
				}
				break;
			}
		}

		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (EditorUI::Get().IsViewHovered() && !EditorUI::Get().IsGizmoOver() && !Input::IsKeyPressed(Key::LeftAlt))
			{
				EditorUI::Get().SetSelectedEntity(m_HoveredEntity);
				Application::Get().HoeveredEntity = m_HoveredEntity;
			}
		}
		return false;
	}

	void EditorLayer::OnOverlayRender()
	{
		if (m_SceneState == SceneState::Play)
		{
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera)
				return;
			Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		else
		{
			Renderer2D::BeginScene(m_EditorCamera);

			if (m_ShowPhysicsColliders)
			{
				// Box Colliders
				{
					auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
					for (auto entity : view)
					{
						auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

						glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
						glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

						glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
							* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
							* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
							* glm::scale(glm::mat4(1.0f), scale);

						// glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						//	* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
						//	* glm::scale(glm::mat4(1.0f), scale);

						Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
					}
				}

				// Circle Colliders
				{
					auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
					for (auto entity : view)
					{
						auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

						glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
						glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2.0f);

						glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
							* glm::scale(glm::mat4(1.0f), scale);

						Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.05f);
					}
				}
			}
			// Draw selected entity outline 
			if (Entity selectedEntity = EditorUI::Get().GetSelectedEntity())
			{
				const TransformComponent& transform = selectedEntity.GetComponent<TransformComponent>();
				Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
			}

		}

		Renderer2D::EndScene();
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			ScriptEngine::Init();

			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);
			EditorUI::Get().OnLoadProject();

			ScriptEngine::s_ProjectName = path.stem().string();
			VL_CORE_INFO(ScriptEngine::s_ProjectName);
		}
	}

	bool EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Violet Project (*.vproj)\0*.vproj\0");
		if (filepath.empty())
			return false;

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::SaveProject()
	{
		// Project::SaveActive();
	}

	void EditorLayer::NewScene()
	{
		// Renderer2D::Init();
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		// m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		EditorUI::Get().SetScenePanelContext(m_EditorScene);

		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Violet Scene (*.violet)\0*.violet\0");
		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		Renderer::Init();
		NewScene();
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension().string() != ".violet")
		{
			VL_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			// m_ActiveScene = newScene;
			// m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			// m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			m_EditorScene = newScene;
			// m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			EditorUI::Get().SetScenePanelContext(m_EditorScene);
			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Violet Scene (*.violet)\0*.violet\0");
		if (!filepath.empty())
		{
			SerializeScene(m_ActiveScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();
		m_SceneState = SceneState::Play;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		EditorUI::Get().SetScenePanelContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();

		EditorUI::Get().SetScenePanelContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		VL_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();
		m_SceneState = SceneState::Edit;

		m_ActiveScene = m_EditorScene;

		EditorUI::Get().SetScenePanelContext(m_ActiveScene);
	}
	
	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(true);
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = EditorUI::Get().GetSelectedEntity();
		if (selectedEntity)
		{
			Entity newEntity = m_EditorScene->DuplicateEntity(selectedEntity);
			EditorUI::Get().SetSelectedEntity(newEntity);
		}
	}

}
