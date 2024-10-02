#include "pch.h"
#include "JohnCamera.h"
#include "GUI.h"
#define MAX_FLT 3.402823466e+38F
using namespace DirectX::SimpleMath;
JohnCamera::JohnCamera()
{
	m_FocalPosition = Vector3::Zero;
	m_Distance = 3.f;
	m_Velocity = Vector3::Zero;

}

JohnCamera::JohnCamera(Vector3 Position, Vector3 Rotation, float FOV /*= 70.f*/)
	:m_Position(Position), m_Rotation(Rotation), m_FOV(FOV)
{
	m_FocalPosition = Vector3::Zero;
	m_Distance = 3.f;
	m_Velocity = Vector3::Zero;
	UpdateFocalPosition ();
	UpdateMatrices ();
}

Vector3 JohnCamera::GetPosition() const
{
	return m_Position;
}

void JohnCamera::SetPosition(Vector3 val)
{
	m_Position = val;
}

Vector3 JohnCamera::GetRotation() const
{
	return m_Rotation;
}

void JohnCamera::SetRotation(Vector3 val)
{
	m_Rotation = val;
}

float JohnCamera::GetFOV() const
{
	return m_FOV;
}

void JohnCamera::SetFOV(float val)
{
	m_FOV = val;
}

float JohnCamera::GetMovementSpeed() const
{
	return m_MovementSpeed;
}

void JohnCamera::SetMovementSpeed(float val)
{
	m_MovementSpeed = val;
}

float JohnCamera::GetMouseLookSpeed() const
{
	return m_MouseLookSpeed;
}

void JohnCamera::SetMouseLookSpeed(float val)
{
	m_MouseLookSpeed = val;
}



void JohnCamera::MouseOrbit(Vector2 MouseDelta)
{
	m_Rotation.y += MouseDelta.x * MouseOrbitSensitivity;
	m_Rotation.x += MouseDelta.y * MouseOrbitSensitivity;
	UpdateCameraPosition ();
}

void JohnCamera::MousePan(Vector2 MouseDelta)
{
	MouseDelta *= m_MousePanSpeed;
	int width, height;
	Application::Get().GetBackBufferSize (width, height);
	const float x = std::min((float)width / 1000.0f, 2.4f);
	const float xSpeed = .0366f * (x * x) - .1778f * x + .3021f;

	const float y = std::min((float)height / 1000.f, 2.f);
	const float ySpeed = .0366f * (y * y) - .17784f * y + .3021f;

	Vector3 xDelta = -GetRightVector () * MouseDelta.x * xSpeed * m_Distance;
	Vector3 yDelta = GetUpVector() * MouseDelta.y * ySpeed * m_Distance;
	m_FocalPosition += xDelta;
	m_FocalPosition += yDelta;

	UpdateCameraPosition ();


}

void JohnCamera::MouseZoom(Vector2 MouseDelta)
{
	float distance = std::max(m_Distance * .8f, 0.f);
	float speed = std::min(distance * distance, 100.f);
	
	float oldDistance = m_Distance;
	m_Distance -= MouseDelta.x * speed;
	if (m_Distance < 1.f)
	{
		m_Distance = oldDistance;
	}
	UpdateCameraPosition ();
}

void JohnCamera::MouseDrive(Vector2 MouseDelta)
{
	m_Rotation.y += MouseDelta.x;

	Vector3 DragDelta;
	DragDelta.z = MouseDelta.y * cosf(XMConvertToRadians  (m_Rotation.y));
	DragDelta.x = MouseDelta.y * sinf(XMConvertToRadians  (m_Rotation.y));

	m_Position -= DragDelta * m_DriveSpeed;
}
void JohnCamera::UpdateMatrices()
{
	UpdateView();
	UpdateProj ();
}

void JohnCamera::UpdateView()
{
	m_ViewMatrix = XMMatrixLookAtLH (m_Position, m_FocalPosition, GetUpVector ());
}

void JohnCamera::UpdateProj()
{
	int width, height;
	Application::Get().GetViewportDimensions (width, height);
	float aspectRatio = (float)width / (float)height;
	m_ProjMatrix = XMMatrixPerspectiveFovLH (XMConvertToRadians (m_FOV), aspectRatio, .1f, 100.f);
}

void JohnCamera::Tick(float DeltaTime, Vector2 MouseDelta)
{

	auto currentState = SDL_GetKeyboardState (NULL);
	Vector3 inputDir = Vector3::Zero;
	Vector2 mouseDelta = Vector2::Zero;
	mouseDelta = MouseDelta;
	float  x, y;
	auto MouseState = SDL_GetMouseState (&x, &y);

	bool bRightMouseDown = MouseState & SDL_BUTTON_RMASK;
	bool bLeftMouseDown = MouseState & SDL_BUTTON_LMASK;
	bool bMiddleMouseDown = MouseState & SDL_BUTTON_MMASK;

	if(MouseDelta != Vector2::Zero)
	{
		bIsCameraMoving =true;
	}
	else
	{
		bIsCameraMoving = false ;
	}

	if (SDL_GetRelativeMouseMode () && bCanMoveCamera)
	{
		if (currentState[SDL_SCANCODE_LALT])
		{
			mouseDelta *= .00314f;
			bool move = false;
			if (bMiddleMouseDown)
			{
				MousePan(mouseDelta);
				move = true;
			}
			else if (bLeftMouseDown)
			{
				MouseOrbit(mouseDelta);
				move = true;
			}
			else if (bRightMouseDown)
			{
				MouseZoom(mouseDelta);
				move = true;
			}
			if (move = true)
			{
				UpdateCameraPosition ();
			}

		}
		else if(bRightMouseDown && !bLeftMouseDown)
		{
			if (currentState[SDL_SCANCODE_D])
			{
				inputDir.x += 1.f;
			}
			if (currentState[SDL_SCANCODE_A])
			{
				inputDir.x -= 1.f;
			}
			if (currentState[SDL_SCANCODE_W])
			{
				inputDir.z += 1.f;
			}
			if (currentState[SDL_SCANCODE_S])
			{
				inputDir.z -= 1.f;
			}
			if (currentState[SDL_SCANCODE_E])
			{
				inputDir.y += 1.f;
			}
			if (currentState[SDL_SCANCODE_Q])
			{
				inputDir.y -= 1.f;
			}
			mouseDelta *= m_MouseLookSpeed;
			m_Rotation.x += mouseDelta.y;
			m_Rotation.y += mouseDelta.x;
			UpdateFocalPosition ();

			int width, height;
			Application::Get().GetBackBufferSize(width, height);
			SDL_WarpMouseInWindow (Application::Get().GetWindow (), m_CursorClickPos.x, m_CursorClickPos.y);
		}
		else if (bLeftMouseDown && !bRightMouseDown)
		{
			MouseDrive  (MouseDelta);

		}
		else if (bRightMouseDown && bLeftMouseDown)
		{
			mouseDelta *= .00314f * -1;
			MousePan(mouseDelta);
			UpdateCameraPosition  ();


			int width, height;
			Application::Get().GetBackBufferSize(width, height);
			SDL_WarpMouseInWindow (Application::Get().GetWindow (), m_CursorClickPos.x, m_CursorClickPos.y);
		}
	}


	UpdatePhysicsSimulation (inputDir, DeltaTime);

	bool bFlyCam = MouseState & SDL_BUTTON_RMASK;
	bool bMayaCam = currentState[SDL_SCANCODE_LALT] && (MouseState & SDL_BUTTON_LMASK || MouseState & SDL_BUTTON_RMASK || MouseState & SDL_BUTTON_MMASK);
	bool bDriveCam = MouseState & SDL_BUTTON_LMASK;

	bInRelativeMode = bCanMoveCamera && (bFlyCam || bMayaCam || bDriveCam);

	if (SDL_GetRelativeMouseMode() == SDL_FALSE && bInRelativeMode == true)
	{
		m_CursorClickPos.x = x;
		m_CursorClickPos.y = y;
	}

	bool relativeMode = SDL_GetRelativeMouseMode  ();

	SDL_SetRelativeMouseMode (bInRelativeMode ? SDL_TRUE : SDL_FALSE);

	if(relativeMode == false && SDL_GetRelativeMouseMode  ())
	{
		//we initiated relative mode. Warp mouse to center to avoid mouse from leaving viewport on large drags

		ImVec2 min, max;

		Application::Get().GetGUI  ()->GetViewportBounds  (min, max);

		ImVec2 viewportPos = Application::Get().GetGUI  ()->GetViewportPos  ();
		ImVec2 viewportSize = Application::Get().GetGUI  ()->GetViewportSize();

		float x = viewportSize.x / 2;
		float y = viewportSize.y / 2;

		float newX = viewportPos.x + x;
		float newY = viewportPos.y + y;

		//SDL_WarpMouseInWindow  (Application::Get().GetWindow  (), newX, newY);
		
	}
}

Matrix JohnCamera::GetViewMatrix() const
{
	return m_ViewMatrix;
}

Matrix JohnCamera::GetProjectionMatrix() const
{
	return m_ProjMatrix;
}

void JohnCamera::UpdateCameraPosition()
{
	m_Position = m_FocalPosition - (GetForwardVector () * m_Distance);
	UpdateView ();
}

void JohnCamera::UpdateFocalPosition()
{
	m_FocalPosition = m_Position + (GetForwardVector () * m_Distance);
	UpdateView ();
}

void JohnCamera::FocusOnPosition(Vector3 newPos)
{
	m_FocalPosition = newPos;
	m_Distance = 3.f;
	m_Velocity = Vector3::Zero;
	UpdateCameraPosition ();
}

Vector3 JohnCamera::GetForwardVector() const
{
	XMMATRIX RotMat = XMMatrixRotationRollPitchYaw (XMConvertToRadians (m_Rotation.x), XMConvertToRadians (m_Rotation.y), 0.f);
	XMVECTOR ForwardVec = XMVector3TransformCoord (g_XMIdentityR2, RotMat);
	ForwardVec = XMVector3Normalize (ForwardVec);
	return ForwardVec;
}

Vector3 JohnCamera::GetRightVector() const
{

	XMMATRIX RotMat = XMMatrixRotationRollPitchYaw (XMConvertToRadians (m_Rotation.x), XMConvertToRadians (m_Rotation.y), 0.f);
	XMVECTOR RightVector = XMVector3TransformCoord (g_XMIdentityR0, RotMat);
	RightVector = XMVector3Normalize (RightVector);
	return RightVector;
}

Vector3 JohnCamera::GetUpVector() const
{
	XMMATRIX RotMat = XMMatrixRotationRollPitchYaw (XMConvertToRadians (m_Rotation.x), XMConvertToRadians (m_Rotation.y), 0.f);
	XMVECTOR UpVector = XMVector3TransformCoord (g_XMIdentityR1, RotMat);
	UpVector = XMVector3Normalize (UpVector);
	return UpVector;
}

void JohnCamera::UpdatePhysicsSimulation(Vector3 InputDir, float DeltaTime)
{
	XMMATRIX RotMat = XMMatrixRotationRollPitchYaw (XMConvertToRadians (m_Rotation.x), XMConvertToRadians (m_Rotation.y), 0.f);

	float ySign = InputDir.y;

	InputDir.y = 0.f;

	Vector3 WorldSpaceImpulse= Vector3::Transform(InputDir, RotMat);
	WorldSpaceImpulse += Vector3(0.f, ySign, 0.f);

	Vector3 WorldSpaceAcceleration = WorldSpaceImpulse * m_MovementSpeed;

	if(bUsePhysicsMovement)
	{
		m_Velocity += WorldSpaceAcceleration * DeltaTime;

		const float DampingFactor = std::clamp(m_VelocityDampingAmount * DeltaTime, 0.f, .75f);

		m_Velocity += -m_Velocity * DampingFactor;
	}
	else
	{
		m_Velocity = WorldSpaceAcceleration;
	}



	if (m_Velocity.LengthSquared () > std::pow(MAX_FLT * m_MovementSpeed, 2.f))
	{
		m_Velocity.Normalize();
		m_Velocity *= MAX_FLT * m_MovementSpeed;
	}

	m_Position += m_Velocity * DeltaTime;
	UpdateFocalPosition ();
	UpdateView ();
}

bool JohnCamera::IsInRelativeMode() const
{
	return bInRelativeMode;
}

bool JohnCamera::GetCanMoveCamera() const
{
	return bCanMoveCamera;
}

void JohnCamera::SetCanMoveCamera(bool val)
{
	bCanMoveCamera = val;
}

bool JohnCamera::GetIsCameraMoving() const
{
	return bIsCameraMoving;
}
