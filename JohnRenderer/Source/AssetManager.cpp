#include "pch.h"
#include "AssetManager.h"

AssetManager* AssetManager::m_Instance = nullptr;

AssetManager::~AssetManager()
{
	m_Meshes.clear ();
	m_Materials.clear ();
}

void AssetManager::RegisterMesh(std::shared_ptr<JohnMesh> Mesh)
{
	m_Meshes.push_back (Mesh);
	Mesh->m_AssetID = GenerateAssetID ();
}

void AssetManager::RegisterMaterial(std::shared_ptr<Material> NewMaterial)
{
	m_Materials.push_back (NewMaterial);
	NewMaterial->m_AssetID = GenerateAssetID ();
}

void AssetManager::ClearAssetManager()
{
	delete m_Instance;
	m_Instance = nullptr;
}

AssetManager& AssetManager::Get()
{
	if(!m_Instance)
	{
		m_Instance = new AssetManager();
	}

	return *m_Instance;
}

void AssetManager::IterateOverMaterials(std::function<void(Material* material)> Func /*= nullptr*/)
{
	if(Func)
	{
		for(auto& material : m_Materials)
		{
		Func(material.get());
			
		}
	}
}

int AssetManager::GenerateAssetID()
{
	int ID = m_LastGivenID;
	m_LastGivenID++;
	return ID;
}

AssetManager::AssetManager()
{
	m_LastGivenID = 0;
}

