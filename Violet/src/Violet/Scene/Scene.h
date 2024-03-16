#pragma once

#include "entt.hpp"
#include "Violet/Core/Timestep.h"

namespace Violet {
	class Scene {
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		inline entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep timestep);
	private:
		entt::registry m_Registry;
	};
}