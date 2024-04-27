#pragma once

#include "Violet.h"
#include "../EditorLayer.h"
#include <imgui/imgui.h>
#include <ImGuizmo.h>


namespace Violet {
	class EditorUI
	{
	public:
		static EditorUI& Get()
		{
			static EditorUI instance;
			return instance;
		}
		EditorUI(const EditorUI&) = delete;
		void operator=(const EditorUI&) = delete;
	public:
		void Attach(EditorLayer* editor);
		void OnLoadProject(EditorLayer* editor);

		void OnImGuiRender(EditorLayer* editor);

		void SetScenePanelContext(Ref<Scene>& scene) { m_SceneHierarchyPanel.SetContext(scene); }
		Entity& GetSelectedEntity()	{ return m_SceneHierarchyPanel.GetSelectedEntity();	}
		void SetSelectedEntity(Entity entity) { m_SceneHierarchyPanel.SetSelectedEntity(entity); }
		SceneHierarchyPanel& GetSceneHierarchyPanel() { return m_SceneHierarchyPanel; }
		Scope<ContentBrowserPanel>& GetContentPanel() { return m_ContentBrowserPanel; }

		int& GetGizmoType() { return m_GizmoType; }
		void SetGizmoType(int type) { m_GizmoType = type; }
		bool IsGizmoUsing() { return ImGuizmo::IsUsing(); }
		bool IsGizmoOver() { return ImGuizmo::IsOver(); }
		bool IsViewHovered() { return m_ViewportHovered; }
		bool IsViewFocused() { return m_ViewportFocused; }
		ImVec2& GetMousePos() { return ImGui::GetMousePos();}
	private:
		EditorUI() = default;

		void StartDockSpace();
		void StopDockSpace();

		// UI Context
		void MenuBar(EditorLayer* editor);
		void StatusPanel(EditorLayer* editor);
		void Settings(EditorLayer* editor);
		void ViewPort(EditorLayer* editor);
		void UIToolbar(EditorLayer* editor);
	private:
		// Editor icon resources
		Ref<Texture2D> m_IconPlay, m_IconPause, m_IconStep, m_IconSimulate, m_IconStop;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		int m_GizmoType = -1;
	};

}
