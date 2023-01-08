#pragma once
#include "Types.h"
using namespace DirectX::SimpleMath;

#define MINMOUSELOOKSPEED .0001f;
#define MAXMOUSELOOKSPEED .02f;

#define MINMOUSEORBITSPEED 1.f;
#define MAXMOUSEORBITSPEED 10.f;

#define MAX_FLT 3.402823466e+38F

struct CameraMovementSettings
{
	float MovementSpeed = 500.f;
	float MouseLookSensitivity = 500.f;
	float MouseOrbitSensitivity = 8.f;
	float MovementAccelerationRate = 20000.f;
	float MovementVelocityDampingAmount = 10.f;
	float MaximumMovementSpeed = MAX_FLT;
	float RotationAccelerationRate = 1600.f;

	float RotationVelocityDampingAmount = 12.f;

	float MaximumRotationSpeed = MAX_FLT;	

};

class CameraUserImpulseData
{
public:

	float MoveForwardBackwardImpulse;

	float MoveRightLeftImpulse;

	float MoveUpDownImpulse;

	float RotateYawImpulse;

	float RotatePitchImpulse;
	
	float RotateRollImpulse;

	float RotateYawVelocityModifier;

	float RotatePitchVelocityModifier;

	float RotateRollVelocityModifier;

	CameraUserImpulseData()
		: MoveForwardBackwardImpulse(0.f),
		MoveRightLeftImpulse(0.f),
		MoveUpDownImpulse(0.f),
		RotateYawImpulse(0.f),
		RotatePitchImpulse(0.f),
		RotateRollImpulse(0.f),
		RotateYawVelocityModifier(0.f),
		RotatePitchVelocityModifier(0.f),
		RotateRollVelocityModifier(0.f)
	{

	}

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

	void AdjustSpeed( John::EScrollDirection Direction);

	Vector3 GetPosition() const;
	void SetPosition(Vector3 NewPos);

	Vector3 GetRotation() const;
	void SetRotation(Vector3 NewRot);

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


	void UpdateSimulation(
		const CameraUserImpulseData& UserImpulseData,
		const float DeltaTime,
		const float MovementSpeedScale,
		Vector3& InOutCameraPosition,
		Vector3& InOutCameraEuler);

	void UpdatePosition( const CameraUserImpulseData& UserImpulse, const float DeltaTime, const float MovementSpeedScale, const Vector3& CameraEuler, Vector3& InOutCameraPosition );

	void UpdateRotation( const CameraUserImpulseData& UserImpulse, const float DeltaTime, Vector3& InOutCameraEuler );

	bool GetUsePhysicsBasedMovement() const;
	void SetUsePhysicsBasedMovement( bool val );

private:

	float m_FOV;

	Vector3 m_Position;
	Vector3 m_FocalPosition;

	Vector3 m_Rotation;

	Matrix m_ViewMatrix;
	Matrix m_ProjectionMatrix;

	int m_ImageWidth;
	int m_ImageHeight;

	float m_Distance;

	float m_SpeedScale = 1.f;

	Vector3 MovementVelocity;

	bool bUsePhysicsBasedMovement = true;

	
};

