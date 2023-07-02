#pragma once
#include "Singleton.h"
#include "ContainerNode.h"
#include "MeshNode.h"
#include "assimp/scene.h"
class SceneFactory :
    public Singleton<SceneFactory>
{
	friend class Singleton<SceneFactory>;

public:
	std::shared_ptr<Node> CreateSceneFromFile(const aiScene* scene);
	void ProcessNode(aiNode* node, std::shared_ptr<ContainerNode> targetParent, const aiScene* scene);
	void ProcessMesh(aiMesh* mesh, std::shared_ptr<MeshNode> meshNode, const aiScene* scene);

private:
	std::shared_ptr<ContainerNode> m_Root;
	std::shared_ptr<ContainerNode> m_CurrentNode;

};

