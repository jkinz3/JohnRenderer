#include "pch.h"
#include "Primitives.h"
#include "JohnPrimitive.h"

namespace John
{
	void ComputeTangents( std::shared_ptr<JohnPrimitive> inMesh )
	{
		
		std::vector<Vertex> verts = *inMesh->GetVertices ();
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texCoords;

		std::vector<Vector3> tangents;
		std::vector<Vector3> bitangents;

		tangents.resize(verts.size());
		bitangents.resize(verts.size());

		for(const auto& vert : verts)
		{
			positions.push_back(vert.Position);
			normals.push_back(vert.Normal);
			texCoords.push_back(vert.TexCoord);
		}

		std::vector<Face> faces = *inMesh->GetFaces();
		std::vector<uint32_t> indices;
		for(const auto& face : faces)
		{
			indices.push_back(face.v1);
			indices.push_back(face.v2);
			indices.push_back(face.v3);
		}

		ComputeTangentFrame(indices.data(), faces.size(), positions.data(), normals.data(), texCoords.data(), positions.size(), tangents.data(), bitangents.data());

		std::vector<Vertex> newVerts;

		for(int i = 0; i < verts.size(); i++)
		{
			Vertex newVert = verts[i];
			newVert.Tangent = tangents[i];
			newVert.Bitangent = bitangents[i];

			newVerts.push_back(newVert);
		}

		inMesh->SetVertices(newVerts);

	}

	void ComputeTangents(JohnPrimitive* inMesh)
	{
		std::vector<Vertex> verts = *inMesh->GetVertices ();
		std::vector<Vector3> positions;
		std::vector<Vector3> normals;
		std::vector<Vector2> texCoords;

		std::vector<Vector3> tangents;
		std::vector<Vector3> bitangents;

		tangents.resize(verts.size());
		bitangents.resize(verts.size());

		for ( const auto& vert : verts )
		{
			positions.push_back(vert.Position);
			normals.push_back(vert.Normal);
			texCoords.push_back(vert.TexCoord);
		}

		std::vector<Face> faces = *inMesh->GetFaces();
		std::vector<uint32_t> indices;
		for ( const auto& face : faces )
		{
			indices.push_back(face.v1);
			indices.push_back(face.v2);
			indices.push_back(face.v3);
		}

		ComputeTangentFrame(indices.data(), faces.size(), positions.data(), normals.data(), texCoords.data(), positions.size(), tangents.data(), bitangents.data());

		std::vector<Vertex> newVerts;

		for ( int i = 0; i < verts.size(); i++ )
		{
			Vertex newVert = verts[i];
			newVert.Tangent = tangents[i];
			newVert.Bitangent = bitangents[i];

			newVerts.push_back(newVert);
		}

		inMesh->SetVertices(newVerts);
	}

	std::shared_ptr<JohnPrimitive> CreateSphere(float diameter, size_t tessellation)
	{
		auto device = DX::DeviceResources::Get().GetD3DDevice();

		std::shared_ptr<JohnPrimitive> newMesh = std::make_shared<JohnPrimitive>();

		if(tessellation < 3)
		{
			 throw std::invalid_argument("tesselation parameter must be at least 3");
		}

		CreateSphereData ( diameter, tessellation, *newMesh->GetVertices (), *newMesh->GetFaces () );

		ComputeTangents( newMesh );

		newMesh->Build ( device );

		return newMesh;

	}

	void CreateSphereData( float diameter, size_t tessellation, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces )
	{

		outVertices.clear ();
		outFaces.clear ();
		const size_t verticalSegments = tessellation;
		const size_t horizontalSegments = tessellation * 2;

		const float radius = diameter / 2;

		for ( size_t i = 0; i <= verticalSegments; i++ )
		{
			const float v = 1 - float( i ) / float( verticalSegments );

			const float latitude = (float( i ) * XM_PI / float( verticalSegments )) - XM_PIDIV2;
			float dy, dxz;

			XMScalarSinCos ( &dy, &dxz, latitude );

			for ( size_t j = 0; j <= horizontalSegments; j++ )
			{
				const float u = float( j ) / float( horizontalSegments );

				const float longitude = float( j ) * XM_2PI / float( horizontalSegments );
				float dx, dz;

				XMScalarSinCos ( &dx, &dz, longitude );

				dx *= dxz;
				dz *= dxz;

				const XMVECTOR normal = XMVectorSet( dx, dy, dz, 0 );
				const XMVECTOR texCoord = XMVectorSet( u, v, 0, 0 );

				Vertex newVert = {};
				newVert.Position = XMVectorScale( normal, radius );
				newVert.Normal = normal;
				newVert.TexCoord = texCoord;

				outVertices.push_back ( newVert );
			}
		}

		const size_t stride = horizontalSegments + 1;

		for ( size_t i = 0; i < verticalSegments; i++ )
		{
			for ( size_t j = 0; j <= horizontalSegments; j++ )
			{
				const size_t nextI = i + 1;
				const size_t nextJ = (j + 1) % stride;

				Face f1 =
				{
					i * stride + j,
					i * stride + nextJ,
					nextI * stride + j
				};

				outFaces.push_back ( f1 );
				Face f2 =
				{
					i * stride + nextJ,
					nextI * stride + nextJ,
					nextI * stride + j
				};

				outFaces.push_back ( f2 );
			}
		}
	}



	std::shared_ptr<JohnPrimitive> CreatePlane(  float size )
	{
		return std::shared_ptr<JohnPrimitive>();
	}

	void CreatePlaneData( float size, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces )
	{

	}

	std::shared_ptr<JohnPrimitive> CreateCube(  float size )
	{
		auto device = DX::DeviceResources::Get().GetD3DDevice();
		std::shared_ptr<JohnPrimitive> newMesh = std::make_shared<JohnPrimitive>();

		if ( size <= 0.f )
		{
			size = .1f;
		}

		CreateCubeData( size, *newMesh->GetVertices (), *newMesh->GetFaces () );
		ComputeTangents(newMesh);
		newMesh->Build ( device );

		return newMesh;
	}

	void CreateCubeData( float size, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces )
	{
		outVertices.clear();
		outFaces.clear();

		constexpr int FaceCount = 6;

		static const XMVECTORF32 faceNormals[FaceCount] =
		{
			{ { {  0,  0,  1, 0 } } },
			{ { {  0,  0, -1, 0 } } },
			{ { {  1,  0,  0, 0 } } },
			{ { { -1,  0,  0, 0 } } },
			{ { {  0,  1,  0, 0 } } },
			{ { {  0, -1,  0, 0 } } },
		};

		static const XMVECTORF32 textureCoordinates[4] =
		{
			{ { { 0, 0, 0, 0 } } },
			{ { { 0, 1, 0, 0 } } },
			{ { { 1, 1, 0, 0 } } },
			{ { { 1, 0, 0, 0 } } },
		};
		XMFLOAT3 fsize( size, size, size );
		XMVECTOR tsize = XMLoadFloat3( &fsize);
		tsize = XMVectorDivide( tsize, g_XMTwo );

		for(int i = 0; i < FaceCount; i++)
		{
			const XMVECTOR normal = faceNormals[i];

			const XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

			const XMVECTOR side1 = XMVector3Cross( normal, basis );
			const XMVECTOR side2 = XMVector3Cross( normal, side1);

			const size_t vbase = outVertices.size();

			Face f1 =
			{
				vbase + 0,
				vbase + 1,
				vbase + 2
			};

			Face f2 =
			{
				vbase + 0,
				vbase + 2,
				vbase + 3
			};

			outFaces.push_back( f1 );
			outFaces.push_back( f2 );

			Vertex v1;
			v1.Position = XMVectorMultiply( XMVectorSubtract( XMVectorSubtract( normal, side1 ), side2), tsize );
			v1.Normal = normal;
			v1.TexCoord = textureCoordinates[0];


			Vertex v2;
			v2.Position = XMVectorMultiply( XMVectorAdd( XMVectorSubtract( normal, side1 ), side2 ), tsize );
			v2.Normal = normal;
			v2.TexCoord = textureCoordinates[1];

			Vertex v3;
			v3.Position = XMVectorMultiply( XMVectorAdd( normal, XMVectorAdd( side1, side2 ) ), tsize );
			v3.Normal = normal;
			v3.TexCoord = textureCoordinates[2];

			Vertex v4;
			v4.Position = XMVectorMultiply( XMVectorSubtract( XMVectorAdd( normal, side1), side2), tsize );
			v4.Normal = normal;
			v4.TexCoord = textureCoordinates[3];

			outVertices.push_back( v1 );
			outVertices.push_back( v2 );
			outVertices.push_back( v3 );
			outVertices.push_back( v4 );
		}


	}

	std::shared_ptr<JohnPrimitive> CreateTorus( float diameter, float thickness, int tessellation )
	{
		auto device = DX::DeviceResources::Get().GetD3DDevice();
		std::shared_ptr<JohnPrimitive> newMesh = std::make_shared<JohnPrimitive>();
		CreateTorusData( diameter, thickness, tessellation, *newMesh->GetVertices(), *newMesh->GetFaces() );
		ComputeTangents(newMesh);
		newMesh->Build( device );

		return newMesh;

	}

	void CreateTorusData( float diameter, float thickness, int tessellation , std::vector<Vertex>& outVertices, std::vector<Face>& outFaces )
	{
		outVertices.clear();
		outFaces.clear();
		if(tessellation < 3)
		{
			tessellation = 3;
		}
		if(diameter<= 0)
		{
			diameter = .1f;
		}
		
		const size_t stride = tessellation + 1;

		for(size_t i = 0; i <=tessellation; i++)
		{
			const float u = float( i ) / float( tessellation );

			const float outerAngle = float( i ) * XM_2PI / float( tessellation ) - XM_PIDIV2;

			const XMMATRIX transform = XMMatrixTranslation( diameter / 2, 0, 0 ) * XMMatrixRotationY( outerAngle );

			for(size_t j = 0; j <= tessellation; j++)
			{
				const float v = 1 - float( j ) / float( tessellation );
				const float innerAngle = float( j ) * XM_2PI / float( tessellation ) + XM_PI;
				float dx, dy;

				XMScalarSinCos( &dy, &dx, innerAngle );

				XMVECTOR normal = XMVectorSet( dx, dy, 0, 0 );
				XMVECTOR position = XMVectorScale( normal, thickness / 2 );
				const XMVECTOR textureCoordinate = XMVectorSet( u, v, 0, 0 );
				position = XMVector3Transform( position, transform );
				normal = XMVector3TransformNormal( normal, transform );

				Vertex vert = {};
				vert.Position = position;
				vert.Normal = normal;
				vert.TexCoord = textureCoordinate;
				outVertices.push_back( vert );

				const size_t nextI = (i + 1) % stride;
				const size_t nextJ = (j + 1) % stride;

				Face f1 =
				{
					i * stride + j,
					nextI * stride + j,
					i * stride + nextJ
				};
				outFaces.push_back( f1 );

				Face f2 =
				{
					i * stride + nextJ,
					nextI * stride + j,
					nextI * stride + nextJ
				};
				outFaces.push_back( f2 );
			}
		}
	}

}