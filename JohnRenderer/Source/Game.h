//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Types.h"

class JohnMesh;

struct CameraMovementSettings
{
	float MovementSpeed = 5.f;
	float MouseLookSensitivity = 110.f;
	float MouseOrbitSensitivity = 50.f;
	
};

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);
	void InitUI(HWND window);
	void InitializeSky();

    // Basic game loop
    void Tick();
	void TickUI();
    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);


    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

	DirectX::SimpleMath::Vector3 GetCameraForwardVector() const;
	DirectX::SimpleMath::Vector3 GetCameraUpVector() const;
	DirectX::SimpleMath::Vector3 GetCameraRightVector() const;

	DirectX::SimpleMath::Quaternion GetCameraOrientation() const;

private:

    void Update(DX::StepTimer const& timer);
	void Movement(float DeltaTime);
	void CameraPan(DirectX::SimpleMath::Vector2 Delta);
	void CameraOrbit(DirectX::SimpleMath::Vector2  Delta);
	void CameraZoom(DirectX::SimpleMath::Vector2 Delta);
	void UpdateLookAt();
	void FocusOnPosition(DirectX::SimpleMath::Vector3 NewLookAt);

	DirectX::SimpleMath::Vector3 CalculatePosition();

	bool CanMoveCamera() const;
    void Render();
	void RenderUI();
	void RenderSky();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	John::ShaderProgram m_StandardProgram;
	John::ShaderProgram m_PBRProgram;
	John::ShaderProgram m_SkyboxProgram;
	John::ComputeProgram m_CubemapConversionProgram;
	
	DirectX::SimpleMath::Vector3 m_CameraPos;
	float m_Pitch, m_Yaw;
	float m_Distance = 3.f;
	DirectX::SimpleMath::Vector3 m_LookPosition = DirectX::SimpleMath::Vector3::Zero;

	DirectX::SimpleMath::Matrix m_ViewMatrix;
	DirectX::SimpleMath::Matrix m_ProjMatrix;

	std::shared_ptr<JohnMesh> m_Mesh;
	std::shared_ptr<JohnMesh> m_Skydome;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_TransformCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ShadingCB;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_StandardSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SkyboxSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_ComputeSampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_BRDF_Sampler;


	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_SkyboxDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_StandardDepthStencilState;



	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;

	CameraMovementSettings m_CamSettings;

	John::Texture m_BrickAlbedo;
	John::Texture m_BrickNormal;
	John::Texture m_BrickRoughness;
	John::Texture m_BrickMetallic;

	John::Texture m_EnvMap;
	John::Texture m_IrradianceMap;
	John::Texture m_BRDF_LUT;


};


