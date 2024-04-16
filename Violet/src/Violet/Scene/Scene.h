#pragma once

#include "entt.hpp"

#include "Violet/Core/UUID.h"
#include "Violet/Core/Timestep.h"
#include "Violet/Renderer/EditorCamera.h"

class b2World;

namespace Violet {
	class Entity;
	class Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		inline entt::registry& Reg() { return m_Registry; }

		void OnUpdateRuntime(Timestep timestep);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);

		void OnViewportResize(uint32_t m_Width, uint32_t m_Height);

		Entity DuplicateEntity(Entity entity);

		//Entity FindEntity(Entity entity);
		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		void Step(int frames = 1);
		inline uint32_t GetViewportWidth() { return m_ViewportWidth; }
		inline uint32_t GetViewportHeight() { return m_ViewportHeight; }

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		// Scipt
		void ScriptSystemUpdate(Timestep step);
		void ScriptSystemInit();

		
		// Render
		void RenderSystemUpdate(Timestep step, EditorCamera* camera = nullptr, bool isRuning = true);
		
		// Physics
		void PhysicsSystemUpdate(Timestep step);
		void PhysicsSystemInit();
		void PhysicsSystemFinalize();

		//template<typename T>
		//void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		b2World* m_PhysicsWorld = nullptr;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
