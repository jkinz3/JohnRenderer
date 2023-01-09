#pragma once
#include "pch.h"
#include "Camera.h"
#include "Types.h"
#include "JohnMesh.h"
#include "Material.h"
using namespace DirectX::SimpleMath;
using namespace DirectX;

using Microsoft::WRL::ComPtr;

struct TransformComponent
{
	Vector3 Translation = { 0.f, 0.f, 0.f };
	Vector3 Rotation = { 0.f, 0.f, 0.f };
	Vector3 Scale = { 1.f, 1.f, 1.f };

	TransformComponent() = default;
	TransformComponent( const TransformComponent& ) = default;
	TransformComponent(const Vector3& translation)
		:Translation(translation)
	{

	}

	Matrix GetTransformationMatrix() const
	{
		Quaternion quat = Quaternion::CreateFromYawPitchRoll( XMConvertToRadians( Rotation.y ), XMConvertToRadians( Rotation.x ), XMConvertToRadians( Rotation.z ) );
		Matrix rotMat = Matrix::CreateFromQuaternion( quat );

		Matrix transMat = Matrix::CreateTranslation( Translation );
		Matrix scaleMat = Matrix::CreateScale( Scale );

		return transMat * rotMat * scaleMat;
	}
};

struct MeshComponent
{
	std::shared_ptr<JohnMesh> Mesh;
	std::shared_ptr<Material> Material;
};