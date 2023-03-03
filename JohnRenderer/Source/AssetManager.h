#pragma once
#include "JohnMesh.h"
#include "Material.h"
class AssetManager
{
public:

	~AssetManager();

	void RegisterMesh(std::shared_ptr<JohnMesh> Mesh);
	void RegisterMaterial(std::shared_ptr<Material> NewMaterial);

	static void ClearAssetManager();

	static AssetManager& Get();

	std::shared_ptr<Material> GetDefaultMaterial() const { return m_Materials[0]; }

	void IterateOverMaterials(std::function<void(Material* material)> Func = nullptr);

private:

	int GenerateAssetID();

	AssetManager();

	std::vector<std::shared_ptr<JohnMesh>> m_Meshes;
	std::vector<std::shared_ptr<Material>> m_Materials;

	int m_LastGivenID;

	static AssetManager* m_Instance;
};

