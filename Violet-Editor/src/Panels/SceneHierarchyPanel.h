#pragma once

#include "Violet/Core/Base.h"
#include "Violet/Core/Log.h"
#include "Violet/Scene/Scene.h"
#include "Violet/Scene/Entity.h"

namespace Violet {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
		void SetSelectedEntity(Entity entity) { m_SelectionContext = entity; }
		Entity GetSelectedEntity() const { return m_SelectionContext; }
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}