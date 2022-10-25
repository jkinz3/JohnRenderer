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
	void InitializeSky( const char* EnvMapFile );
	void CleanSky();
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
	void DrawSky();
	void DrawMesh(JohnMesh* MeshToDraw);
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

	bool CanMoveCamera() const;

	void ReloadShaders();

	void TickGizmo();

	void DrawSceneOutliner();
	void DrawModelDetails(JohnMesh* Mesh);


	void DeleteMesh( std::shared_ptr<JohnMesh> MeshToDelete );



	void SelectModel( JohnMesh* ModelToSelect );

	void DeselectAll();

	JohnMesh* MousePicking();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;


	//my sheeit
	John::ShaderProgram m_PhongProgram;
	JohnMesh* m_SelectedModel = nullptr;

	ComPtr<ID3D11Buffer> m_PhongTransformCB;
	ComPtr<ID3D11Buffer> m_PhongShadingCB;

	ComPtr<ID3D11SamplerState> m_StandardSampler;
	ComPtr<ID3D11SamplerState> m_brdfSampler;
	

	std::vector<std::shared_ptr<JohnMesh>> m_Meshes;

	std::unique_ptr<Camera> m_Camera;
	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;
	Vector2 m_MouseDelta;
	Vector3 m_LightPos;
	bool m_bIsRelativeMode = false;
	bool m_bWasCameraMoved = false;

	Vector2 m_DragAmount = Vector2::Zero;

	ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;
	ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

	John::Environment m_Environment;

	std::unique_ptr<DirectX::GeometricPrimitive> m_Sky;
	std::unique_ptr<DX::SkyboxEffect> m_SkyEffect;

	ComPtr<ID3D11InputLayout> m_SkyInputLayout;

	ComPtr<ID3D11ShaderResourceView> m_BrickNormal;
	


};
