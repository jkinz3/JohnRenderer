#pragma once
#include "entt.hpp"

class Camera;
class Entity;

class Scene
{
public:
	Scene();
	~Scene();

	Entity CreateEntity( const std::string& name = std::string() );
	void DestroyEntity( Entity entity );

	void RenderScene( Camera& camera );
	
	template<typename... Components>
	auto GetAllEntitiesWith()
	{
		return m_Registry.view<Components...>();
	}

	entt::registry m_Registry;
private:

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	friend class Entity;

};

