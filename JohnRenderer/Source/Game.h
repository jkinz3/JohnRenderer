//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Types.h"
#include "Camera.h"
class JohnMesh;



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

private:

    void Update(DX::StepTimer const& timer);
	void Movement(float DeltaTime);


	bool CanMoveCamera() const;
    void Render();
	void RenderUI();
	void RenderSky();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

	void ReloadShaders( John::ShaderProgram InProgram );

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	John::ShaderProgram m_StandardProgram;
	John::ShaderProgram m_PBRProgram;
	John::ShaderProgram m_SkyboxProgram;
	John::ComputeProgram m_CubemapConversionProgram;
	
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


	std::unique_ptr<Camera> m_Camera;

	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;


	John::Texture m_BrickAlbedo;
	John::Texture m_BrickNormal;
	John::Texture m_BrickRoughness;
	John::Texture m_BrickMetallic;


	John::Texture m_IrradianceMap;
	John::Texture m_BRDF_LUT;

	John::Texture m_EnvMap;


};


