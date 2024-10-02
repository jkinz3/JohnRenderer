#pragma once
#include "Scene.h"
#include "SceneRenderer.h"

using Microsoft::WRL::ComPtr;

using namespace DirectX;
using namespace DirectX::SimpleMath;

class JohnCamera;
class GUI;
class Application
{
public:


	bool Initialize();

	bool InitD3D(HWND hWnd);

	void CleanupD3D();
	void Cleanup();

	void CleanupRenderTarget();
	void CreateRenderTarget();
	void OnWindowResized(int width, int height);
	void GetViewportDimensions(int& width, int& height);

	void OnViewportResize(int width, int height);

	void ProcessInput(float DeltaTime);
	void Clear();
	void Render();
	void Present();

	void Run();
	void Tick(float DeltaTime);

	static Application& Get();

	static void CleanupApplication();


	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();

	void GetBackBufferSize(int& width, int& height);

	SDL_Window* GetWindow() const;

	void SignalQuit();

	std::shared_ptr<SceneRenderer> GetRenderer() const;

	HWND GetNativeWindow() const;

	GUI* GetGUI() const;

	void RequestExit();

	std::shared_ptr<JohnCamera> GetCamera() const {return m_Camera;}

	void BeginDebugEvent(_In_z_ const wchar_t* name);
	void EndDebugEvent();
	void SetDebugMarker(_In_z_ const wchar_t* name);

	std::shared_ptr<Actor> GetSelectedActor() const;
	bool IsActorSelected() const;

	void OpenScene(const std::wstring FileName);

	void ImportScene(const std::wstring FileName);

	void NewScene();
private:

	int m_Width;
	int m_Height;
	std::string m_AppName;

	ComPtr<ID3D11Device> m_Device;
	ComPtr<ID3D11DeviceContext> m_Context;
	ComPtr<IDXGISwapChain> m_SwapChain;
	ComPtr<ID3D11RenderTargetView> m_BackBuffer;
	ComPtr<ID3DUserDefinedAnnotation> m_DebugAnnotation;

	SDL_Window* m_Window;

	bool bWantsToQuit = false;


	Application(int width, int height, const char* name);

	inline static Application* Inst;

	std::shared_ptr<Scene> m_Scene;
	std::shared_ptr<SceneRenderer> m_Renderer;

	std::shared_ptr<JohnCamera> m_Camera;

	std::shared_ptr<GUI> m_GUI;

	std::unique_ptr<CommonStates> m_CommonStates;

	Vector2 m_TrackingDelta;

};

