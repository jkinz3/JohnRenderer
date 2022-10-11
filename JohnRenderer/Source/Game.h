//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Types.h"
#include "Camera.h"
class JohnMesh;

using Microsoft::WRL::ComPtr;

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
	void InitializeUI();


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
	void Movement( float DeltaTime );
	
    void Render();
	void RenderUI();
	void DrawScene();
	void DrawMesh(JohnMesh* MeshToDraw);
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

	bool CanMoveCamera() const;

	void ReloadShaders();

	void DrawModelDetails(JohnMesh* Mesh);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;



	//my sheeit
	John::ShaderProgram m_PhongProgram;

	ComPtr<ID3D11Buffer> m_PhongTransformCB;
	ComPtr<ID3D11Buffer> m_PhongShadingCB;

	std::vector<std::shared_ptr<JohnMesh>> m_Meshes;

	std::unique_ptr<Camera> m_Camera;
	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;
	Vector2 m_MouseDelta;
	Vector3 m_LightPos;
	bool m_bIsRelativeMode = false;

};
