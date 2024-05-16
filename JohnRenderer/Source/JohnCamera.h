#pragma once
using namespace DirectX::SimpleMath;

class JohnCamera
{
public:
	JohnCamera();
	JohnCamera(Vector3 Position, Vector3 Rotation, float FOV = 70.f);

	DirectX::SimpleMath::Vector3 GetPosition() const;
	void SetPosition(DirectX::SimpleMath::Vector3 val);
	DirectX::SimpleMath::Vector3 GetRotation() const;
	void SetRotation(DirectX::SimpleMath::Vector3 val);
	float GetFOV() const;
	void SetFOV(float val);
	float GetMovementSpeed() const;
	void SetMovementSpeed(float val);
	float GetMouseLookSpeed() const;
	void SetMouseLookSpeed(float val);

	void MouseOrbit(Vector2 MouseDelta);
	void MousePan(Vector2 MouseDelta);
	void MouseZoom(Vector2 MouseDelta);

	void UpdateMatrices();

	void UpdateView();

	void UpdateProj();

	void Tick(float DeltaTime, Vector2 MouseDelta);

	Matrix GetViewMatrix() const;
	Matrix GetProjectionMatrix() const;

	void UpdateCameraPosition();

	void UpdateFocalPosition();

	void FocusOnPosition(Vector3 newPos);

	Vector3 GetForwardVector() const;
	Vector3 GetRightVector() const;
	Vector3 GetUpVector() const;

	void UpdatePhysicsSimulation(Vector3 InputDir, float DeltaTime);

	bool IsInRelativeMode() const;

	bool GetCanMoveCamera() const;
	void SetCanMoveCamera(bool val);
private:


	Matrix m_ViewMatrix;
	Matrix m_ProjMatrix;

	Vector3 m_Position;
	Vector3 m_Rotation;

	Vector3 m_FocalPosition;

	float m_FOV;

	float m_MovementSpeed = 150.f;
	float m_MouseLookSpeed = 0.5f;

	float m_Distance;


	float MouseOrbitSensitivity = 150.f;
	float m_MousePanSpeed = 15.f;

	Vector3 m_Velocity;

	bool bUsePhysicsMovement = true;

	float m_VelocityDampingAmount = 10.f;

	bool bCanMoveCamera = false;
	
	bool bInRelativeMode = false;

	DirectX::SimpleMath::Vector2 m_CursorClickPos;

};

