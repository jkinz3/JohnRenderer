#pragma once
#include "Scene.h"

class Entity
{
public:
	Entity() = default;
	Entity( entt::entity handle, Scene* scene );
	Entity( const Entity& other ) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		T& component = m_Scene->m_Registry.emplace<T>( m_EntityHandle, std::forward<Args>( args )... );
		return component;
	}

	template<typename T, typename... Args>
	T& AddOrReplaceComponent( Args&&... args )
	{
		T& component = m_Scene->m_Registry.emplace_or_replace<T>( m_EntityHandle, std::forward<Args>( args )... );
		return component;
	}


	template<typename T>
	T& GetComponent()
	{
		if(!HasComponent<T> ())
		{
			
		}
		return m_Scene->m_Registry.get<T>( m_EntityHandle );
	}

	template<typename T>
	bool HasComponent()
	{
		return m_Scene->m_Registry.all_of<T>(m_EntityHandle);


	}

	template<typename T>
	void RemoveComponent()
	{
		if(!HasComponent<T>())
		{
			return;
		}
		m_Scene->m_Registry.remove<T>( m_EntityHandle );
	}

	operator bool() const { return m_EntityHandle != entt::null; }
	operator entt::entity() const { return m_EntityHandle; }
	operator uint32_t() const { return (uint32_t)m_EntityHandle; }

	bool operator==( const Entity& other ) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	bool operator!=( const Entity& other ) const
	{
		return !(*this == other);
	}

	entt::entity GetEntityHandle() const { return m_EntityHandle; }
	void SetEntityHandle( entt::entity val ) { m_EntityHandle = val; }


	std::string m_Name;
private:
	entt::entity m_EntityHandle{ entt::null };

	Scene* m_Scene = nullptr;


};

