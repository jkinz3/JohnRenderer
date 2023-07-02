#include "pch.h"
#include "SceneManager.h"

SceneManager::SceneManager()
{
	m_SceneNode = std::make_shared<RootNode>(-1);

}


std::shared_ptr<RootNode> SceneManager::GetSceneNode()
{
	return m_SceneNode;
}

const std::shared_ptr<RootNode> SceneManager::GetSceneNode() const
{
	return m_SceneNode;
}

void SceneManager::AddNode(std::shared_ptr<Node> NewNode)
{
	GetSceneNode ()->AddChild(NewNode);
}

