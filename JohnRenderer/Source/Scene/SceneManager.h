#pragma once
#include "Singleton.h"
#include "Node.h"
#include "RootNode.h"
class SceneManager :
    public Singleton<SceneManager>
{
	friend class Singleton<SceneManager>;

public:

	std::shared_ptr<RootNode> GetSceneNode();
	const std::shared_ptr<RootNode> GetSceneNode() const;
	void AddNode(std::shared_ptr<Node> NewNode);



protected:
	SceneManager();

private:
	std::shared_ptr<RootNode> m_SceneNode;
};

