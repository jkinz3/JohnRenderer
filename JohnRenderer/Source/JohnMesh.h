#pragma once
#include "Types.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;

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

	Matrix GetTransformationMatrix();

	John::Transform GetTransform() const;
	void SetTransform( John::Transform val );

	Vector3 GetPosition() const;
	void SetPosition( Vector3 val );
	Quaternion GetRotation() const;
	void SetRotation( Quaternion val );
	Vector3 GetScale() const;
	void SetScale( Vector3 val );
	
	Vector3 GetRotationEuler() const;
	void SetRotationEuler( Vector3 NewEuler );

	void ResetTransformations();

	std::string GetName() const;
	void SetName( std::string val );
private:

	std::vector<Vertex> m_Vertices;
	std::vector<Face> m_Faces;

	UINT m_Stride;
	UINT m_Offset;

	UINT m_NumIndices;

	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;

	John::Transform m_Transform;
	
	std::string m_Name = std::string("Model");
};

