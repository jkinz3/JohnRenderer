#pragma once

using namespace DirectX::SimpleMath;

struct CameraMovementSettings
{
	float MovementSpeed = 5.f;
	float MouseLookSensitivity = 110.f;
	float MouseOrbitSensitivity = 50.f;

};

class Camera
{
public:
	Camera();
	~Camera();

	Matrix GetViewMatrix();
	Matrix GetProjectionMatrix();

	void UpdateMatrices();

	void UpdateView();

	void UpdateProjection();

	void MoveAndRotateCamera(Vector3 DeltaLoc, Vector2 MouseDelta);

	Vector3 GetPosition() const;
	void SetPosition(Vector3 NewPos);

	Vector2 GetRotation() const;
	void SetRotation(Vector2 NewRot);

	Quaternion GetOrientation() const;
	Vector3 GetForwardVector() const;
	Vector3 GetRightVector() const;
	Vector3 GetUpVector() const;

	void SetImageSize(int newWidth, int newHeight);

	void SetFOV(float NewFOV);

	void MouseOrbit( Vector2 MouseDelta );
	void MousePan(Vector2 MouseDelta);
	void Mousezoom(Vector2 MouseDelta);

	void UpdateCameraPosition();

	void UpdateFocalPosition();

	void FocusOnPosition( Vector3 NewPos );

	CameraMovementSettings m_MovementSettings;

	Vector2 GetRotationInRadians() const;

private:

	float m_FOV;

	Vector3 m_Position;
	Vector3 m_FocalPosition;

	Vector2 m_Rotation;

	Matrix m_ViewMatrix;
	Matrix m_ProjectionMatrix;

	int m_ImageWidth;
	int m_ImageHeight;

	float m_Distance;

	
};

