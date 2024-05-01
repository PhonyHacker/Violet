#pragma once

#include "entt.hpp"

#include "Violet/Core/UUID.h"
#include "Violet/Core/Timestep.h"
#include "Violet/Renderer/EditorCamera.h"

class b2World;

namespace Violet {
	class Entity;
	class Scene;

	namespace SystemTag
	{
		enum Type
		{
			None = 0,
			Script = BIT(0),
			Physics = BIT(1),
			Render = BIT(2)
		};
	}

	class System {
	public:
		System() = default;
		System(Scene* scene) : m_Scene(scene) {}

		virtual void Init() = 0;
		virtual void Update(Timestep ts) = 0;
		virtual void Detach() = 0;

		virtual SystemTag::Type GetSystemType() = 0;
	protected:
		Scene* m_Scene;
	};

	class SystemList {
	public:
		~SystemList() {	for (auto ptr : m_List)	delete ptr;	}

		void Push(System* system) { m_List.push_back(system); }

		void Init(int tag)
		{
			for (auto system : m_List) { if (system->GetSystemType() & tag)	system->Init(); }
		}
		void Update(int tag, Timestep ts)
		{
			for (auto system : m_List) { if (system->GetSystemType() & tag)	system->Update(ts);	}
		}
		void Detach(int tag)
		{
			for (auto system : m_List) { if (system->GetSystemType() & tag)	system->Detach();}
		}
	private:
		std::vector<System*> m_List;
	};

	class ScriptSystem : public System {
	public:
		ScriptSystem() = default;
		ScriptSystem(Scene* scene) : System(scene) {}

		virtual void Init() override;
		virtual void Update(Timestep ts) override;
		virtual void Detach() override;

		virtual SystemTag::Type GetSystemType() { return SystemTag::Script; }
	};

	class PhysicsSystem : public System {
	public:
		PhysicsSystem() = default;
		PhysicsSystem(Scene* scene) : System(scene) {}

		virtual void Init() override;
		virtual void Update(Timestep ts) override;
		virtual void Detach() override;

		virtual SystemTag::Type GetSystemType() { return SystemTag::Physics; }
	private:
		b2World* m_PhysicsWorld = nullptr;
	};

	class RenderSystem : public System {
	public:
		RenderSystem() = default;
		RenderSystem(Scene* scene) : System(scene) {}

		virtual void Init() override;
		virtual void Update(Timestep ts) override;
		virtual void Detach() override;

		virtual SystemTag::Type GetSystemType() { return SystemTag::Render; }
	};

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
		void OnUpdateEditor(Timestep ts, EditorCamera* camera);
		void OnUpdateSimulation(Timestep ts, EditorCamera* camera);

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
		bool RunningRender = false;
		EditorCamera* UserCamera = nullptr;

	private:
		SystemList Systems;

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		std::unordered_map<UUID, entt::entity> m_EntityMap;


		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
