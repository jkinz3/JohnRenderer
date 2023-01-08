#include "Types.h"
#include "JohnMesh.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class JohnPrimitive;

namespace John
{
	std::shared_ptr<JohnPrimitive> CreateSphere( ID3D11Device* device, float diameter, size_t tessellation );

	void CreateSphereData( float diameter, size_t tessellation, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces );

	std::shared_ptr<JohnPrimitive> CreatePlane(ID3D11Device* device, float size);

	std::shared_ptr<JohnPrimitive> CreateCube(ID3D11Device* device, float size);

	


	
}