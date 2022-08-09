#pragma once

struct Vertex
{
	DirectX::SimpleMath::Vector3 Position;
	DirectX::SimpleMath::Vector3 Normal;
	DirectX::SimpleMath::Vector2 TexCoord;
	DirectX::SimpleMath::Vector3 Tangent;
	DirectX::SimpleMath::Vector3 Bitangent;
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

	DirectX::SimpleMath::Matrix GetTransformationMatrix();

private:

	std::vector<Vertex> m_Vertices;
	std::vector<Face> m_Faces;

	UINT m_Stride;
	UINT m_Offset;

	UINT m_NumIndices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;

};

