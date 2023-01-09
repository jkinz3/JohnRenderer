#include "pch.h"
#include "Scene.h"
#include "Camera.h"
#include "Entity.h"


Scene::Scene()
{
	

}

Scene::~Scene()
{

}

Entity Scene::CreateEntity( const std::string& name /*= std::string() */ )
{
	Entity entity = { m_Registry.create(), this };
	return entity;

}

void Scene::DestroyEntity( Entity entity )
{
	m_Registry.destroy( entity );
}

void Scene::RenderScene( Camera& camera )
{

}
