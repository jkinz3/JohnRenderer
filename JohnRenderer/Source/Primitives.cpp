#include "pch.h"
#include "Primitives.h"
#include "JohnMesh.h"
using namespace DirectX::SimpleMath;


std::shared_ptr<JohnMesh> John::Primities::CreateSphere(float diameter, size_t tessellation)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;


	if(tessellation <3)
	{
		tessellation = 3;
	}
	
	const size_t vertSegments = tessellation;
	const size_t horiSegments = tessellation * 2;

	const float radius = diameter / 2;

	for(size_t i = 0; i <= vertSegments; i++)
	{
		const float v = 1 - float(i) / float(vertSegments);

		const float lat = (float(i) * XM_PI / float(vertSegments)) - XM_PIDIV2;
		float dy, dxz;

		XMScalarSinCos  (&dy, &dxz, lat);

		for(size_t j = 0; j <= horiSegments; j++)
		{
			const float u = float(j) / float(horiSegments);

			const float longitude = float(j) * XM_2PI / float(horiSegments);

			float dx, dz;

			XMScalarSinCos  (&dx, &dz, longitude);

			dx *= dxz;
			dz *= dxz;

			const XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);
			const XMVECTOR texCoord = XMVectorSet  (u, v, 0, 0);

			Vertex vert;
			vert.Position = XMVectorScale(normal, radius);
			vert.Normal = normal;
			vert.TexCoord = texCoord;

			vertices.push_back  (vert);
		}
	}

	const size_t stride = horiSegments + 1;

	for(size_t i = 0; i < vertSegments; i++)
	{
		for(size_t j = 0; j <= horiSegments; j++)
		{
			const size_t nextI = i + 1;
			const size_t nextJ = (j + 1) % stride;

			indices.push_back  (i * stride + j);
			indices.push_back  (nextI * stride + j);
			indices.push_back  (i * stride + nextJ);

			indices.push_back  (i * stride + nextJ);
			indices.push_back  (nextI * stride + j);
			indices.push_back  (nextI * stride + nextJ);

		}
	}

	std::shared_ptr<JohnMesh> newMesh = std::make_shared<JohnMesh>(vertices, indices);
	newMesh->Build  ();

	return newMesh;
}

std::shared_ptr<JohnMesh> John::Primities::CreateBox(const DirectX::SimpleMath::Vector3 & size)
{
	return std::shared_ptr<JohnMesh>();
}

std::shared_ptr<JohnMesh> John::Primities::CreatePlane(const DirectX::SimpleMath::Vector3 Size)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;


	static const XMVECTORF32 faceNormals[1] =
	{
		{	{	{ 0 , 1 , 0 , 0 }	}	}
	};

	static const XMVECTORF32 texCoords[4] =
	{
		{{{ 1, 0, 0, 0 }}},
		{{{ 1, 1, 0, 0 }}},
		{{{ 0, 1, 0, 0 }}},
		{{{ 0, 0, 0, 0 }}},
	};

	XMFLOAT3 vsize = Size;

	XMVECTOR tSize = XMLoadFloat3(&vsize);

	tSize = XMVectorDivide  (tSize, g_XMTwo);

	const XMVECTOR normal = faceNormals[0];

	const XMVECTOR basis = g_XMIdentityR2;

	const XMVECTOR side1 = XMVector3Cross(normal, basis);
	const XMVECTOR side2 = XMVector3Cross(normal, side1);

	Vertex v1;
	XMVECTOR testSub = XMVectorSubtract  (normal, side1);
	XMVECTOR testSub2 = XMVectorSubtract  (testSub, side2);

	v1.Position = XMVectorMultiply  (testSub2, tSize);
	v1.Normal = normal;
	v1.TexCoord = texCoords[0];
	vertices.push_back  (v1);

	Vertex v2;
	v2.Position = XMVectorMultiply  (XMVectorAdd(XMVectorSubtract  (normal, side1), side2), tSize);
	v2.Normal = normal;
	v2.TexCoord = texCoords[1];
	vertices.push_back  (v2);

	Vertex v3;
	v3.Position = XMVectorMultiply  (XMVectorAdd  (XMVectorAdd(normal, side1), side2), tSize);
	v3.Normal = normal;
	v3.TexCoord = texCoords[2];
	vertices.push_back  (v3);

	Vertex v4;
	v4.Position = XMVectorMultiply  (XMVectorSubtract  (XMVectorAdd  (normal, side1), side2), tSize);
	v4.Normal = normal;
	v4.TexCoord = texCoords[3];
	vertices.push_back  (v4);

	indices.push_back  (0);
	indices.push_back  (2);
	indices.push_back  (1);

	indices.push_back  (0);
	indices.push_back  (3);
	indices.push_back  (2);

	std::shared_ptr<JohnMesh> NewMesh = std::make_shared<JohnMesh>(vertices, indices);
	if(NewMesh)
	{
		NewMesh->Build  ();
	}

	return NewMesh;
}
