#include "Types.h"
#include "JohnMesh.h"
using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class JohnPrimitive;

namespace John
{
	std::shared_ptr<JohnPrimitive> CreateSphere( float diameter, size_t tessellation );

	void CreateSphereData( float diameter, size_t tessellation, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces );

	std::shared_ptr<JohnPrimitive> CreatePlane(float size);

	void CreatePlaneData( float size, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces );

	std::shared_ptr<JohnPrimitive> CreateCube( float size);

	void CreateCubeData( float size, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces );

	std::shared_ptr<JohnPrimitive> CreateTorus( float diameter, float thickness, int tessellation);

	void CreateTorusData( float diameter, float thickness, int tessellation, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces );

	void ComputeTangents(std::shared_ptr<JohnPrimitive> inMesh);
	void ComputeTangents(JohnPrimitive*  inMesh);

	
}