#include "vlpch.h"
#include "Entity.h"

namespace Violet {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
	}

}