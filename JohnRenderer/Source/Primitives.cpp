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
	std::shared_ptr<JohnPrimitive> CreateCube( ID3D11Device * device, float size )
	{
		return std::shared_ptr<JohnPrimitive>();
	}
}