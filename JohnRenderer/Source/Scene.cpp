#include "pch.h"
#include "Scene.h"
#include "Texture.h"
#include "PointLight.h"
void Scene::LoadFromFile(const char* FileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile (FileName, aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded);

	if(scene == nullptr)
	{
		throw std::runtime_error("failed to load file");
		return;
	}


	ProcessNode (scene->mRootNode, scene, nullptr, aiMatrix4x4());


	auto light1 = std::make_shared<PointLight>();
	light1->SetPosition (Vector3(0.f, 1.5f, 0.f));
	light1->SetIntensity (1.f);
	light1->SetColor (Vector3::One);
	m_PointLights.push_back (light1);
// 
// 	auto light2 = std::make_shared<PointLight>();
// 	light2->SetPosition (Vector3(-1.f, 1.5f, 1.f));
// 	light2->SetIntensity (1.f);
// 	light2->SetColor (Vector3::One);
// 	m_PointLights.push_back (light2);
}

void Scene::ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<Actor> targetParent, aiMatrix4x4 accTransform)
{
	std::shared_ptr<Actor> parent;
	aiMatrix4x4 worldMatrix;

	if (node->mNumMeshes > 0)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto newMesh = ProcessMesh (mesh, scene);
			aiVector3D pos, scale;
			aiQuaternion rot;
		 worldMatrix = node->mTransformation * accTransform;
			worldMatrix.Decompose (scale, rot, pos);
	
			newMesh->SetPosition ({ pos.x, pos.y, pos.z });
			newMesh->SetScale({ scale.x, scale.y, scale.z });
			newMesh->SetRotation({ rot.x, rot.y, rot.z, rot.w });
			parent = newMesh;
			m_Actors.push_back(newMesh);
		}
	}

	for(UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode (node->mChildren[i], scene, parent, worldMatrix);
	}
}

std::shared_ptr<Actor> Scene::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for(UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;

		if(mesh->HasNormals ())
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}
		if(mesh->HasTextureCoords (0))
		{
			vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back (vertex);
	}

	for(UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back (face.mIndices[j]);
		}
	}

	std::shared_ptr<JohnMesh> newMesh = std::make_shared<JohnMesh>(vertices, indices);

	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		std::shared_ptr<Texture> baseColor = LoadMaterialTexture (mat, aiTextureType_DIFFUSE, "baseColor", scene);
		newMesh->SetBaseColorMap (baseColor);
		std::shared_ptr<Texture> normal = LoadMaterialTexture (mat, aiTextureType_NORMALS, "normal", scene);
		newMesh->SetNormalMap(normal);
		std::shared_ptr<Texture> roughness = LoadMaterialTexture (mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
		newMesh->SetRoughnessMap(roughness);
		std::shared_ptr<Texture> metallic = LoadMaterialTexture (mat, aiTextureType_METALNESS, "metallic", scene);
		newMesh->SetMetallicMap(metallic);


	}

	newMesh->Build ();

	std::shared_ptr<Actor> newActor = std::make_shared<Actor>();

	newActor->SetMesh (newMesh);
	newActor->GetTransform ().UpdateMatrix ();
	newActor->SetName (mesh->mName.C_Str ());

	return newActor;
}

std::shared_ptr<Texture> Scene::LoadMaterialTexture(aiMaterial* mat, aiTextureType type, std::string typeNAme, const aiScene* scene)
{
	std::shared_ptr<Texture> texture;
	for(int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture (type, i, &str);
		bool skip = false;
		for(int j = 0; j < m_LoadedTextures.size (); j++)
		{
			if(std::strcmp(m_LoadedTextures[j]->GetName ().c_str (), str.C_Str ()) == 0)
			{
				texture = m_LoadedTextures[j];
				skip = true;
				break;
			}
		}

		if(!skip)
		{
			HRESULT hr;
			texture = std::make_shared<Texture>();

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture (str.C_Str ());
			
			if(embeddedTexture != nullptr)
			{
				texture->SetSRV (LoadEmbeddedTexture (embeddedTexture));
			}
			else
			{
				std::string filename = std::string(str.C_Str ());
				filename = "Content/" + filename;
				std::wstring filenameW = std::wstring(filename.begin(), filename.end());
				ComPtr<ID3D11ShaderResourceView> srv;
				hr = CreateWICTextureFromFile (Application::Get().GetDevice (), Application::Get().GetContext (), filenameW.c_str(), nullptr, srv.ReleaseAndGetAddressOf ());
				texture->SetSRV (srv);

				if(FAILED(hr))
				{
					MessageBox(Application::Get().GetNativeWindow (), L"Couldn't load texture!", L"Error!", MB_ICONERROR | MB_OK);
				}
			}
			texture->SetName (str.C_Str ());
			m_LoadedTextures.push_back (texture);
		}
	}

	return texture;
}

ComPtr<ID3D11ShaderResourceView> Scene::LoadEmbeddedTexture(const aiTexture* embeddedTexture)
{
	auto device = Application::Get().GetDevice ();
	auto context = Application::Get().GetContext ();

	ComPtr<ID3D11ShaderResourceView> srv;
	if (embeddedTexture->mHeight != 0)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = embeddedTexture->mWidth;
		desc.Height = embeddedTexture->mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = embeddedTexture->pcData;
		data.SysMemPitch = embeddedTexture->mWidth * 4;
		data.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

		ID3D11Texture2D* tex;
		ThrowIfFailed (
			device->CreateTexture2D(&desc, &data, &tex)
		);
		ThrowIfFailed (
			device->CreateShaderResourceView(tex, nullptr, srv.ReleaseAndGetAddressOf ())
		);
		return srv;
	}

	const size_t size = embeddedTexture->mWidth;

	HRESULT hr = CreateWICTextureFromMemory (device, context, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, srv.ReleaseAndGetAddressOf ());
	if (FAILED(hr))
	{
		MessageBox(Application::Get().GetNativeWindow (), L"Texture couldn't be created from memory", L"Error!", MB_ICONERROR | MB_OK);
	}

	return srv;
}

std::vector<std::shared_ptr<Actor>>& Scene::GetActors()
{
	return m_Actors;

}

std::vector<std::shared_ptr<PointLight>>& Scene::GetPointLights()
{
	return m_PointLights;
}

void Scene::SaveToDisk(const char* Path)
{

}

void Scene::AddActor(std::shared_ptr<Actor> newActor)
{
	if(newActor != nullptr)
	{
		m_Actors.push_back (newActor);
	}

}

void Scene::AddPointLight(std::shared_ptr<PointLight> newLight)
{
	if(newLight != nullptr)
	{
		m_PointLights.push_back (newLight);
	}

}
