#pragma once
class Scene;
class JohnMesh;

struct SceneHeader
{
	size_t NumMeshes;
	size_t NumPointLights;
};

class SceneSerializer
{
public:
	SceneSerializer(std::shared_ptr<Scene> inScene);
	SceneSerializer() {}
	void WriteToDisk(std::string FileName);

	std::shared_ptr<Scene> LoadFromDisk(std::string FileName);
private:

	std::shared_ptr<Scene> m_Scene;

};

