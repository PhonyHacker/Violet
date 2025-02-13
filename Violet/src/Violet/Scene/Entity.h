#pragma once

#include "Violet/Core/UUID.h"
#include "Scene.h"
#include "Components.h"

#include "entt.hpp"

namespace Violet {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			VL_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			//m_Scene->OnComponentAdded<T>(*this, component);

			if constexpr (std::is_same<T, CameraComponent>::value)
			{
				if(m_Scene->GetViewportWidth()>0 && m_Scene->GetViewportHeight()>0)
					component.Camera.SetViewportSize(m_Scene->GetViewportWidth(), m_Scene->GetViewportHeight());
			}

			return component;
		}
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			// m_Scene->OnComponentAdded<T>(*this, component);

			if constexpr (std::is_same<T, CameraComponent>::value)
			{
				if (m_Scene->GetViewportWidth() > 0 && m_Scene->GetViewportHeight() > 0)
					component.Camera.SetViewportSize(m_Scene->GetViewportWidth(), m_Scene->GetViewportHeight());
			}
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			VL_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T> 
		void RemoveComponent()
		{
			VL_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		const UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		operator bool() const 	{return  m_EntityHandle != entt::null;}
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity m_EntityHandle{ entt::null};
		Scene* m_Scene = nullptr;
	};

}
