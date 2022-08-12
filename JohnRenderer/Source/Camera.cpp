#include "pch.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera()
{
	m_FOV = 70.f;
	m_ImageWidth = 720;
	m_ImageHeight = 1280;
	m_Position = Vector3::Zero;
	m_Rotation = Vector2::Zero;
	
	m_MouseSensitivity = 50.f;
	m_MovementSpeed = 3.f;

	m_ViewMatrix = Matrix::Identity;
	m_ProjectionMatrix = Matrix::Identity;

	m_FocalPosition = Vector3::Zero;
	m_Distance = 5.f;
	
	UpdateFocalPosition();

	UpdateMatrices();
}

Camera::~Camera()
{

}

Matrix Camera::GetViewMatrix()
{
	return m_ViewMatrix;
}

Matrix Camera::GetProjectionMatrix()
{
	return m_ProjectionMatrix;
}

void Camera::UpdateMatrices()
{
	UpdateView();
	UpdateProjection();
}

void Camera::UpdateView()
{
	m_ViewMatrix = XMMatrixLookAtLH(m_Position, m_Position + GetForwardVector(), GetUpVector());
}

void Camera::UpdateProjection()
{
	float AspectRatio = (float)m_ImageWidth / (float)m_ImageHeight;
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FOV), AspectRatio, .1f, 1000.f);
}

void Camera::MoveAndRotateCamera(DirectX::SimpleMath::Vector3 DeltaLoc, DirectX::SimpleMath::Vector2 MouseDelta)
{
	m_Position += DeltaLoc * m_MovementSpeed;
	m_Rotation.x -= MouseDelta.y * m_MouseSensitivity;
	m_Rotation.y += MouseDelta.x * m_MouseSensitivity;

	UpdateFocalPosition();
	UpdateView();

}

Vector3 Camera::GetPosition() const
{
	return m_Position;
}

void Camera::SetPosition(DirectX::SimpleMath::Vector3 NewPos)
{
	m_Position = NewPos;
	UpdateFocalPosition();
	UpdateView();
}

Vector2 Camera::GetRotation() const
{
	return m_Rotation;
}

void Camera::SetRotation(DirectX::SimpleMath::Vector2 NewRot)
{
	m_Rotation = NewRot;
	UpdateFocalPosition();
	UpdateView();
}

Quaternion Camera::GetOrientation() const
{
	return Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(m_Rotation.y),XMConvertToRadians(m_Rotation.x), 0.f);
}

Vector3 Camera::GetForwardVector() const
{
	Vector3 ForwardVec = Vector3::Transform(Vector3::Forward, GetOrientation());
	ForwardVec.Normalize();
	return ForwardVec;
}

Vector3 Camera::GetRightVector() const
{
	Vector3 RightVec = Vector3::Transform(Vector3::Right, GetOrientation());
	RightVec.Normalize();
	return RightVec;
}

Vector3 Camera::GetUpVector() const
{
	Vector3 UpVec = Vector3::Transform(Vector3::Up, GetOrientation());
	UpVec.Normalize();
	return UpVec;
}

void Camera::SetImageSize(int newWidth, int newHeight)
{
	m_ImageWidth = newWidth;
	m_ImageHeight  = newHeight;
	UpdateProjection();
}

void Camera::SetFOV(float NewFOV)
{
	m_FOV = NewFOV;
	UpdateProjection();
}

void Camera::MouseOrbit(DirectX::SimpleMath::Vector2 MouseDelta)
{

}

void Camera::MousePan(DirectX::SimpleMath::Vector2 MouseDelta)
{

}

void Camera::Mousezoom(DirectX::SimpleMath::Vector2 MouseDelta)
{

}

void Camera::UpdateCameraPosition()
{

}

void Camera::UpdateFocalPosition()
{
	m_FocalPosition = m_Position + (GetForwardVector() * m_Distance);
}
