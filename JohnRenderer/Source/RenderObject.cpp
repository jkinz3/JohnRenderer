#include "pch.h"
#include "RenderObject.h"
#include "Material.h"
#include "JohnMesh.h"
Matrix RenderObject::GetTransformationMatrix()
{
	Matrix TransMat = Matrix::CreateTranslation( m_Transform.Translation );
	Matrix RotMat = Matrix::CreateFromQuaternion( m_Transform.Rotation );
	Matrix ScaleMat = Matrix::CreateScale( m_Transform.Scale );
	Matrix ModelMatrix = ScaleMat * RotMat * TransMat;
	return ModelMatrix;

}

void RenderObject::Draw( ID3D11DeviceContext* context )
{
	m_Material->Apply ( context );
	m_Mesh->Draw ( context );
}

John::Transform RenderObject::GetTransform() const
{
	return m_Transform;
}

void RenderObject::SetTransform( John::Transform val )
{
	m_Transform = val;
}

Vector3 RenderObject::GetPosition() const
{
	return m_Transform.Translation;
}

void RenderObject::SetPosition( Vector3 val )
{
	m_Transform.Translation = val;
}

Quaternion RenderObject::GetRotation() const
{
	return m_Transform.Rotation;
}

void RenderObject::SetRotation( Quaternion val )
{
	m_Transform.Rotation = val;
}

Vector3 RenderObject::GetScale() const
{
	return m_Transform.Scale;
}

void RenderObject::SetScale( Vector3 val )
{
	m_Transform.Scale = val;
}

Vector3 RenderObject::GetRotationEuler() const
{
	Vector3 EulerRot = m_Transform.Rotation.ToEuler();

	return John::EulerRadiansToDegrees( EulerRot );
}

void RenderObject::SetRotationEuler( Vector3 NewEuler )
{
	Vector3 EulerRadRot = John::EulerDegreesToRadians( NewEuler );
	m_Transform.Rotation = Quaternion::CreateFromYawPitchRoll( EulerRadRot.y, EulerRadRot.x, EulerRadRot.z );
}

void RenderObject::ResetTransformations()
{
	m_Transform.Translation = Vector3::Zero;
	m_Transform.Rotation = Quaternion::Identity;
	m_Transform.Scale = Vector3::One;

}

std::shared_ptr<Material> RenderObject::GetMaterial() const
{
	return m_Material;
}

void RenderObject::SetMaterial( std::shared_ptr<Material> val )
{
	m_Material = val;
}

std::shared_ptr<JohnMesh> RenderObject::GetMesh() const
{
	return m_Mesh;
}

void RenderObject::SetMesh( std::shared_ptr<JohnMesh> val )
{
	m_Mesh = val;
}


std::string RenderObject::GetName() const
{
	return m_Name;
}

void RenderObject::SetName( std::string val )
{
	m_Name = val;
}
