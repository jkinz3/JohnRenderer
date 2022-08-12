#pragma once
class Camera
{
public:
	Camera();
	~Camera();

	DirectX::SimpleMath::Matrix GetViewMatrix();
	DirectX::SimpleMath::Matrix GetProjectionMatrix();

	void UpdateMatrices();

	void UpdateView();

	void UpdateProjection();

	void MoveAndRotateCamera(DirectX::SimpleMath::Vector3 DeltaLoc, DirectX::SimpleMath::Vector2 MouseDelta);

	DirectX::SimpleMath::Vector3 GetPosition() const;
	void SetPosition(DirectX::SimpleMath::Vector3 NewPos);

	DirectX::SimpleMath::Vector2 GetRotation() const;
	void SetRotation(DirectX::SimpleMath::Vector2 NewRot);

	DirectX::SimpleMath::Quaternion GetOrientation() const;

	DirectX::SimpleMath::Vector3 GetForwardVector() const;

	DirectX::SimpleMath::Vector3 GetRightVector() const;

	DirectX::SimpleMath::Vector3 GetUpVector() const;

	void SetImageSize(int newWidth, int newHeight);

	void SetFOV(float NewFOV);

	void MouseOrbit(DirectX::SimpleMath::Vector2 MouseDelta);
	void MousePan(DirectX::SimpleMath::Vector2 MouseDelta);
	void Mousezoom(DirectX::SimpleMath::Vector2 MouseDelta);

	void UpdateCameraPosition();

	void UpdateFocalPosition();

private:

	float m_FOV;

	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_FocalPosition;

	DirectX::SimpleMath::Vector2 m_Rotation;

	DirectX::SimpleMath::Matrix m_ViewMatrix;
	DirectX::SimpleMath::Matrix m_ProjectionMatrix;

	int m_ImageWidth;
	int m_ImageHeight;

	float m_MovementSpeed;
	float m_MouseSensitivity; 

	float m_Distance;

	
};

