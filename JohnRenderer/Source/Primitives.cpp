#include "pch.h"
#include "Primitives.h"
#include "JohnPrimitive.h"

namespace John
{
	std::shared_ptr<JohnPrimitive> CreateSphere( ID3D11Device * device, float diameter, size_t tessellation )
	{
		std::shared_ptr<JohnPrimitive> newMesh = std::make_shared<JohnPrimitive>();

		if(tessellation < 3)
		{
			 throw std::invalid_argument("tesselation parameter must be at least 3");
		}

		CreateSphereData ( diameter, tessellation, *newMesh->GetVertices (), *newMesh->GetFaces () );

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



	std::shared_ptr<JohnPrimitive> CreatePlane( ID3D11Device * device, float size )
	{
		return std::shared_ptr<JohnPrimitive>();
	}

	void CreatePlaneData( float size, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces )
	{

	}

	std::shared_ptr<JohnPrimitive> CreateCube( ID3D11Device * device, float size )
	{
		std::shared_ptr<JohnPrimitive> newMesh = std::make_shared<JohnPrimitive>();

		if ( size <= 0.f )
		{
			size = .1f;
		}

		CreateCubeData( size, *newMesh->GetVertices (), *newMesh->GetFaces () );

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

}