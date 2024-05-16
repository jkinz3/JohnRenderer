#pragma once
#include "Actor.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Texture;
class PointLight;
using Microsoft::WRL::ComPtr;

class Scene
{
public:

	Scene();

	std::vector<std::shared_ptr<Actor>> m_Actors;
	std::vector<std::shared_ptr<Texture>> m_LoadedTextures;
	std::vector<std::shared_ptr<PointLight>> m_PointLights;


	void LoadFromFile(const char* FileName);

	void ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<Actor> targetParent, aiMatrix4x4 accTransform);

	std::shared_ptr<Actor> ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::shared_ptr<Texture> LoadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeNAme, const aiScene* scene);

	ComPtr<ID3D11ShaderResourceView> LoadEmbeddedTexture(const aiTexture* embeddedTexture);

	std::vector<std::shared_ptr<Actor>>& GetActors();
	std::vector<std::shared_ptr<PointLight>>& GetPointLights();

	bool bIsLoaded = false;

	void SaveToDisk(const char* Path);

	void AddActor(std::shared_ptr<Actor> newActor);

	void AddPointLight(std::shared_ptr<PointLight> newLight);
};

