#pragma once
#include "pch.h"
#include "Camera.h"
#include "Types.h"
#include "JohnMesh.h"
#include "Material.h"
#include "Components.h"
using namespace DirectX::SimpleMath;
using namespace DirectX;

using Microsoft::WRL::ComPtr;

struct TransformComponent
{
	Vector3 Translation = { 0.f, 0.f, 0.f };
	Quaternion Rotation = Quaternion::Identity;
	Vector3 Scale = { 1.f, 1.f, 1.f };

	TransformComponent() = default;
	TransformComponent( const TransformComponent& ) = default;
	TransformComponent(const Vector3& translation)
		:Translation(translation)
	{

	}

	Matrix GetTransformationMatrix() const
	{
		//Quaternion quat = Quaternion::CreateFromYawPitchRoll( XMConvertToRadians( Rotation.y ), XMConvertToRadians( Rotation.x ), XMConvertToRadians( Rotation.z ) );
		Matrix rotMat = Matrix::CreateFromQuaternion( Rotation );

		Matrix transMat = Matrix::CreateTranslation( Translation );
		Matrix scaleMat = Matrix::CreateScale( Scale );

		return scaleMat * rotMat * transMat;
	}
	
	void Reset()
	{
		Translation = { 0.f, 0.f, 0.f };
		Rotation = Quaternion::Identity;
		Scale = { 1.f, 1.f, 1.f };
	}
};

struct MeshComponent
{
	std::shared_ptr<JohnMesh> Mesh;
	std::shared_ptr<Material> Material;
};

