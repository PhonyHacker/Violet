#pragma once

#include "Violet.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "Violet/Renderer/EditorCamera.h"

namespace Violet {
	enum class SceneState
	{
		Edit = 0, Play = 1, Simulate = 2
	};

	class EditorLayer : public Layer {
	//public:
	//	static EditorLayer& Get()
	//	{
	//		m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	//		static EditorLayer instance("EditorLayer");
	//		s_Font = Font::GetDefault();

	//		return instance;
	//	}
	//	EditorLayer(const& EditorLayer) = delete;
	//	void operator=(const EditorLayer&) = delete;
	//private:
	//	EditorLayer();
	public:
		EditorLayer();

		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep step) override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event& event) override;

		void NewProject();
		bool OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneSimulate();
		void OnSceneStop();
		void OnScenePause();

#pragma region API
		Timestep GetTimeStep() { return m_Timestep; }
		bool& IsShowCollider() { return m_ShowPhysicsColliders; }
		glm::vec2* GetViewBounds() { return m_ViewportBounds; }
		EditorCamera& GetEditorCamera() { return m_EditorCamera; }
		glm::vec2& GetViewSize() { return m_ViewportSize; }
		Ref<Framebuffer>& GetFramebuffer() { return m_Framebuffer; }
		Ref<Scene>& GetActiveScene() { return m_ActiveScene; }
		Ref<Scene>& GetEditorScene() { return m_EditorScene; }
		SceneState& GetSceneState() { return m_SceneState; }
#pragma endregion
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnOverlayRender();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnDuplicateEntity();

	protected:
		Ref<Framebuffer> m_Framebuffer;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		std::filesystem::path m_EditorScenePath;

		Entity m_HoveredEntity;
		SceneState m_SceneState = SceneState::Edit;

		bool m_ShowPhysicsColliders = false;
		//bool m_PrimaryCamera = true;

		EditorCamera m_EditorCamera;
	private:
		Timestep m_Timestep = 0.0f;

	};
}
