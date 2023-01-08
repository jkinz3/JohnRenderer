#pragma once
#include "entt.hpp"

class Scene
{
public:
	Scene();
	~Scene();

private:
	entt::registry m_Registry;
};

