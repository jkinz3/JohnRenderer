#pragma once
#include "Types.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;
class Material;
class SourceMesh;

struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;
	Vector3 Tangent;
	Vector3 Bitangent;
};

struct Face
{
	uint32_t v1, v2, v3;
};

class JohnMesh
{

public:

	void Build(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* context);
	std::vector<Vertex>* GetVertices();
	std::vector<Face>* GetFaces();

	static John::EAssetType GetStaticType() { return John::EAssetType::JohnMesh; }
	virtual John::EAssetType GetAssetType() const { return GetStaticType(); }

	std::shared_ptr<SourceMesh> GetSourceMesh() const { return m_SourceMesh; }
	void SetSourceMesh( std::shared_ptr<SourceMesh> val ) { m_SourceMesh = val; }

	void SetVertices(std::vector<Vertex> inVerts);
	void SetFaces(std::vector<Face>inFaces);


private:

	std::vector<Vertex> m_Vertices;
	std::vector<Face> m_Faces;

	UINT m_Stride;
	UINT m_Offset;

	UINT m_NumIndices;

	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;

	std::shared_ptr<SourceMesh> m_SourceMesh;


	John::EAssetType m_ComponentType;

public:

	int m_AssetID;
};

