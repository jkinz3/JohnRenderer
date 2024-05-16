#include "pch.h"
#include "SceneSerializer.h"
#include "JohnMesh.h"
#include "JohnMeshSerializer.h"
#include "Scene.h"
#include "PointLight.h"
SceneSerializer::SceneSerializer(std::shared_ptr<Scene> inScene)
	:m_Scene(inScene)
{

}

void SceneSerializer::WriteToDisk(std::string FileName)
{
	assert(m_Scene);

	
	size_t indexdLocation = FileName.find('.', FileName.length ());

	std::filesystem::path Path(FileName);
	Path.remove_filename ();

	if (!Path.empty ())
	{
		std::filesystem::create_directories (Path);
	}

	std::ofstream outFile(FileName.c_str (), std::ios::binary | std::ios::out);

	if(!outFile)
	{
		assert(0);
	}

	auto actors = m_Scene->GetActors();
	auto pointLights = m_Scene->GetPointLights();
	SceneHeader header;
	header.NumMeshes = actors.size();
	header.NumPointLights = pointLights.size();

	outFile.write(reinterpret_cast<const char*>(&header), sizeof(SceneHeader));
	size_t vector3Size = sizeof(float) * 3;
	size_t quatSize = sizeof(float) * 4;
	for(auto& actor : actors)
	{
		auto mesh = actor->GetMesh ();

		DirectX::SimpleMath::Vector3 pos = actor->GetPosition ();
		DirectX::SimpleMath::Quaternion rot = actor->GetRotation ();
		DirectX::SimpleMath::Vector3 scale = actor->GetScale();

		outFile.write(reinterpret_cast<const char*>(&pos.x), vector3Size);
		outFile.write(reinterpret_cast<const char*>(&rot.x), quatSize);
		outFile.write(reinterpret_cast<const char*>(&scale .x), vector3Size);
		
		
		JohnMeshSerializer meshSerializer(mesh);

		meshSerializer.WriteToDisk (FileName.c_str(), outFile);
	}

	for(auto pointLight : pointLights)
	{
		DirectX::SimpleMath::Vector3 position = pointLight->GetPosition ();
		DirectX::SimpleMath::Vector3 color = pointLight->GetColor();
		float intensity = pointLight->GetIntensity();
		
		size_t vector3Size = sizeof(float) * 3;
		outFile.write(reinterpret_cast<const char*>(&position.x), vector3Size);
		outFile.write(reinterpret_cast<const char*>(&color.x), vector3Size);
		outFile.write(reinterpret_cast<const char*>(&intensity), sizeof(float));

	}

	outFile.close();


}

std::shared_ptr<Scene> SceneSerializer::LoadFromDisk(std::string FileName)
{

	std::ifstream inFile(FileName.c_str (), std::ios::binary | std::ios::in);

	assert(inFile);

	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
	SceneHeader header;

	size_t vector3Size = sizeof(float) * 3;
	size_t quatSize = sizeof(float) * 4;
	inFile.read(reinterpret_cast<char*>(&header), sizeof(SceneHeader));
	int actorCount = header.NumMeshes;
	int pointLightCount = header.NumPointLights;

	for(int i = 0; i < actorCount; i++)
	{
		JohnMeshSerializer serializer;

		std::shared_ptr<Actor> newActor = std::make_shared<Actor>();
		
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Quaternion rot;
		DirectX::SimpleMath::Vector3 scale;

		inFile.read(reinterpret_cast<char*>(&pos), vector3Size);
		inFile.read(reinterpret_cast<char*>(&rot), quatSize);
		inFile.read(reinterpret_cast<char*>(&scale.x), vector3Size);

		newActor->SetPosition (pos);
		newActor->SetRotation (rot);
		newActor->SetScale (scale);
		
		std::shared_ptr<JohnMesh> newMesh = serializer.LoadFromDisk (FileName.c_str (), inFile);
		newActor->SetMesh (newMesh);

		newScene->AddActor (newActor);
	}
	
	for (int i = 0; i < pointLightCount; i++)
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 color;
		float intensity;

		std::shared_ptr<PointLight> newLight = std::make_shared<PointLight>();
		inFile.read(reinterpret_cast<char*>(&position.x), vector3Size);
		inFile.read(reinterpret_cast<char*>(&color.x), vector3Size);
		inFile.read(reinterpret_cast<char*>(&intensity), sizeof(float));

		newScene->AddPointLight (newLight);

	}

	inFile.close();

	return newScene;

}
