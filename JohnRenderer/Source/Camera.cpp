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
	
	m_ViewMatrix = Matrix::Identity;
	m_ProjectionMatrix = Matrix::Identity;

	m_FocalPosition = Vector3::Zero;
	m_Distance = 3.f;
	
	m_MovementSettings.MovementSpeed = 6.f;
	m_MovementSettings.MouseLookSensitivity = 1.5f;
	m_MovementSettings.MouseOrbitSensitivity = 8.f;

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
	m_ViewMatrix = XMMatrixLookAtLH(m_Position, m_FocalPosition, GetUpVector());
}

void Camera::UpdateProjection()
{
	float AspectRatio = (float)m_ImageWidth / (float)m_ImageHeight;
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FOV), AspectRatio, .1f, 1000.f);
}

void Camera::MoveAndRotateCamera(DirectX::SimpleMath::Vector3 DeltaLoc, DirectX::SimpleMath::Vector2 MouseDelta)
{
	m_Position += DeltaLoc * m_MovementSettings.MovementSpeed;
	Vector2 radRot = GetRotationInRadians();

	radRot.x += MouseDelta.y * m_MovementSettings.MouseLookSensitivity;
	radRot.y += MouseDelta.x * m_MovementSettings.MouseLookSensitivity;

	m_Rotation = Vector2( XMConvertToDegrees( radRot.x ), XMConvertToDegrees( radRot.y ) );

	if(m_Rotation.y >= 360.f)
	{
		m_Rotation.y -= 360.f;
	}
	if(m_Rotation.y <= -360.f)
	{
		m_Rotation.y += 360.f;
	}

	UpdateFocalPosition();

}

Vector3 Camera::GetPosition() const
{
	return m_Position;
}

void Camera::SetPosition(DirectX::SimpleMath::Vector3 NewPos)
{
	m_Position = NewPos;
	UpdateFocalPosition();
}

Vector2 Camera::GetRotation() const
{
	return m_Rotation;
}

void Camera::SetRotation(DirectX::SimpleMath::Vector2 NewRot)
{
	m_Rotation = NewRot;
	UpdateFocalPosition();

}

Quaternion Camera::GetOrientation() const
{
	return Quaternion::CreateFromYawPitchRoll( XMConvertToRadians( m_Rotation.y ), XMConvertToRadians( m_Rotation.x ), 0.f );
}

Vector3 Camera::GetForwardVector() const
{
	Vector3 ForwardVec = Vector3::Transform(Vector3::UnitZ, GetOrientation());
	ForwardVec.Normalize();
	return ForwardVec;
}

Vector3 Camera::GetRightVector() const
{
	Vector3 RightVec = Vector3::Transform(Vector3::UnitX, GetOrientation());
	RightVec.Normalize();
	return RightVec;
}

Vector3 Camera::GetUpVector() const
{
	Vector3 UpVec = Vector3::Transform(Vector3::UnitY, GetOrientation());
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
	Vector2 RadRot = GetRotationInRadians();

	float yawSign = GetUpVector().y < 0 ? -1.f : 1.f;
	RadRot.y += yawSign * MouseDelta.x * m_MovementSettings.MouseOrbitSensitivity;
	RadRot.x += MouseDelta.y * m_MovementSettings.MouseOrbitSensitivity;
	m_Rotation = Vector2( XMConvertToDegrees( RadRot.x ), XMConvertToDegrees( RadRot.y ) );
	UpdateCameraPosition();
}

void Camera::MousePan(DirectX::SimpleMath::Vector2 MouseDelta)
{

	MouseDelta /= .03f;
	const float x = std::min( (float)m_ImageWidth  / 1000.0f, 2.4f);
	const float xSpeed = .0366f * (x * x) - .1778f * x + .3021f;

	const float y = std::min( (float)m_ImageHeight / 1000.f, 2.f );
	const float ySpeed = .0366f * (y * y) - .17784f * y + .3021f;



	Vector3 xDelta = -GetRightVector() * MouseDelta.x * xSpeed * m_Distance;
	Vector3 yDelta = GetUpVector() * MouseDelta.y * ySpeed * m_Distance;

	m_FocalPosition += xDelta;
	m_FocalPosition += yDelta;


	UpdateCameraPosition();
}

void Camera::Mousezoom(DirectX::SimpleMath::Vector2 MouseDelta)
{
	float distance = m_Distance * .8f;
	distance = std::max( distance, 0.f );
	float speed = distance * distance;
	speed = std::min( speed, 100.f );

	float oldDistance = m_Distance;
	m_Distance -= MouseDelta.x * speed;
	if(m_Distance < 1.f)
	{
		m_Distance = oldDistance;
	}
	UpdateCameraPosition();
}

void Camera::UpdateCameraPosition()
{
	m_Position = m_FocalPosition - (GetForwardVector() * m_Distance);
	UpdateView();
}

void Camera::UpdateFocalPosition()
{
	m_FocalPosition = m_Position + (GetForwardVector() * m_Distance);
	UpdateView();
}

void Camera::FocusOnPosition( Vector3 NewPos )
{
	m_FocalPosition = NewPos;
	m_Distance = 3.f;
	UpdateCameraPosition();
}

Vector2 Camera::GetRotationInRadians() const
{
	return Vector2( XMConvertToRadians( m_Rotation.x ), XMConvertToRadians( m_Rotation.y ) );
}
