//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Types.h"

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

    // Basic game loop
    void Tick();

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
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	John::ShaderProgram m_StandardProgram;

	std::shared_ptr<JohnMesh> m_Sphere;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_TransformCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ShadingCB;

	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;
};
