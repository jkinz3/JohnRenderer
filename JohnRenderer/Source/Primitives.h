#pragma once

class JohnMesh;
namespace John
{
	namespace Primities
	{
		std::shared_ptr<JohnMesh> CreateSphere(float diameter = 1, size_t tessellation = 16);
		std::shared_ptr<JohnMesh> CreateBox(const DirectX::SimpleMath::Vector3& size);
		std::shared_ptr<JohnMesh> CreatePlane(const DirectX::SimpleMath::Vector3 Size);
	}
}