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
private:
	Vector3 Translation = { 0.f, 0.f, 0.f };
	Quaternion Rotation = Quaternion::Identity;
	Vector3 Scale = { 1.f, 1.f, 1.f };
	Vector3 RotationEuler = Vector3::Zero;
public:

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

	void SetTranslation( Vector3 NewTranslation ) { Translation = NewTranslation; }
	void SetScale( Vector3 NewScale ) { Scale = NewScale; }
	void SetRotation(Vector3 InRotation)
	{
		RotationEuler = InRotation;
		Rotation = Quaternion::CreateFromYawPitchRoll( RotationEuler.y, RotationEuler.x, RotationEuler.z );

	}
	void SetRotation(Quaternion InRotation)
	{
		Rotation = InRotation;
		RotationEuler = Rotation.ToEuler();

	}

	Vector3 GetTranslation() const { return Translation; }
	Vector3 GetScale() const { return Scale; }
	Quaternion GetRotation() const { return Rotation; }
	Vector3 GetRotationEuler() const { return RotationEuler; }

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

struct PointLightComponent
{
	Vector3 Position;
	Vector3 LightColor;
	float LightIntensity;
};


struct NameComponent
{
	std::string Name;
};
