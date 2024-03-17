#pragma once

#include "entt.hpp"
#include "Violet/Core/Timestep.h"

namespace Violet {
	class Entity;
	class Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		inline entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep timestep);
	private:
		entt::registry m_Registry;

		friend class Entity;
	};
}