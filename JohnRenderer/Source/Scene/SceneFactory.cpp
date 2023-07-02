#include "pch.h"
#include "SceneFactory.h"
#include "MeshNode.h"
std::shared_ptr<Node> SceneFactory::CreateSceneFromFile(const aiScene* scene)
{
	Node* parent = nullptr;
	aiNode* rootNode = scene->mRootNode;
	m_CurrentNode = std::make_shared<ContainerNode>(-1);
	m_CurrentNode->Initialize ();
	m_Root = m_CurrentNode;
	ProcessNode (rootNode, m_CurrentNode, scene);
}

void SceneFactory::ProcessNode(aiNode* node, std::shared_ptr<ContainerNode> targetParent, const aiScene* scene)
{
	if(node->mNumMeshes > 0)
	{
		auto newNode = std::make_shared<MeshNode>(-1);
		targetParent->AddChild (newNode);
		
		for(int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh (mesh, newNode, scene);
		}
	}
}

void SceneFactory::ProcessMesh(aiMesh* mesh, std::shared_ptr<MeshNode> meshNode, const aiScene* scene)
{

}
