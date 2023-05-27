//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Types.h"
#include "Camera.h"
#include "MouseDeltaTracker.h"
#include "Entity.h"

class JohnMesh;
class Material;
class RenderObject;
class CameraUserImpulseData;
class Scene;

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
	void InitializeDefaultAssets();
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
	void OnMouseMove();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

	void SetInputEnabled(bool bEnabled);

private:

    void Update(DX::StepTimer const& timer);
	void Movement( float DeltaTime );
	
    void Render();
	void RenderUI();
	void DrawCameraViewport();
	void DrawScene();
	void DrawSky();
	void DrawMesh(RenderObject* MeshToDraw);
	void DrawToneMapping();
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

	bool CanMoveCamera() const;

	void ReloadShaders();

	void ImportMeshFromFile(const char* File);

	void AddPrimitive( John::EPrimitiveType type );

	void AddPointLight();

	void TickGizmo();

	void DrawSceneOutliner();
	void DrawModelInOutliner( const char* prefix, int uid, Entity mesh);
	void DrawModelDetails(Entity Mesh);


	void DeleteMesh( Entity MeshToDelete );

	void ConvertMovementDeltaToDragRot( Vector3& InOutDragDelta,  Vector3& OutDrag, Vector3& Rot );


	void SelectModel( Entity ModelToSelect );

	void PrepareInputState();

	void DeselectAll();

	bool IsCameraViewportHovered() const;

	Entity MousePicking();



    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	//my sheeit
	Entity m_SelectedModel;

	ComPtr<ID3D11Buffer> m_TransformCB;
	ComPtr<ID3D11Buffer> m_ShadingCB;
	ComPtr<ID3D11Buffer> m_LightSphereTransformCB;



	ComPtr<ID3D11SamplerState> m_StandardSampler;
	ComPtr<ID3D11SamplerState> m_brdfSampler;
	ComPtr<ID3D11SamplerState> m_ComputeSampler;
	
	John::ShaderProgram m_ToneMapProgram;

	std::vector<std::shared_ptr<RenderObject>> m_RenderObjects;
	
	std::unique_ptr<DX::RenderTexture> m_ViewportRenderTarget;
	std::unique_ptr<DX::RenderTexture> m_FinalRenderTarget;



	std::unique_ptr<Camera> m_Camera;

	std::unique_ptr<DirectX::Mouse> m_Mouse;
	std::unique_ptr<DirectX::Keyboard> m_Keyboard;
	Mouse::State m_MouseState;
	Keyboard::State m_KeyboardState;
	DirectX::Keyboard::KeyboardStateTracker m_Keys;
	DirectX::Mouse::ButtonStateTracker m_MouseButtons;

	WCHAR* ImportFile(COMDLG_FILTERSPEC FileExtension[], UINT ExtensionCount);

	void ImportMesh();

	John::MouseDeltaTracker m_MouseDeltaTracker;
	bool m_bIsRelativeMode = false;
	bool m_bWasCameraMoved = false;

	Vector2 m_DragAmount = Vector2::Zero;

	ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::MODE::WORLD;
	ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

	John::Environment m_Environment;

	std::unique_ptr<DirectX::GeometricPrimitive> m_Sky;
	std::unique_ptr<DX::SkyboxEffect> m_SkyEffect;

	ComPtr<ID3D11InputLayout> m_SkyInputLayout;

	John::Texture m_BrickBaseColor;
	John::Texture m_BrickNormal;
	John::Texture m_BrickRoughness;
	
	bool m_bShowImGuiDemoWindow = false;

	std::map<EShaderProgram, John::ShaderProgram> m_Shaders;


	John::FrameBuffer m_DefaultFrameBuffer;

	Vector2 m_MouseDelta;

	std::shared_ptr<CameraUserImpulseData> m_CameraUserImpulseData;

	bool m_bJustFinishedMovingCamera = false;


	std::shared_ptr<Scene> m_Scene;
	

	Entity NullEntity;

	Vector2 m_RelativeMouseDelta;

	bool m_bWantsRightClick;

	bool m_bRightClickOpen;

	std::string m_ViewportWindowName;

	bool m_bCanMoveCamera = false;

	
	Vector3 LineStart, LineEnd;

	Vector2 m_ViewportSize;

	ImVec2 m_ViewportBounds[2];

	Vector2 GetMouseViewportSpace();

	bool m_bViewportPanelHovered;
	
	bool m_bIsGizmoHovered = false;

};
