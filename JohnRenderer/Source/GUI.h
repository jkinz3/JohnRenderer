#pragma once
class Scene;
class SceneRenderer;
class GUI
{

public:

	GUI(){}

	~GUI();

	void Initialize();

	void TickUI();

	void TickMainMenuBar();

	void TickGizmo();

	void SetScene(std::shared_ptr<Scene> InScene);

	void RenderUI();

	void SetViewportTexture(DX::RenderTexture* ViewportImage);
	
	void DrawViewport();

	void DrawSideBar();


	void SetRenderer(std::shared_ptr<SceneRenderer> renderer);

	void GetViewportDimensions(int & width, int & height);

	bool IsViewportHovered() const;

	void MousePicking();

	DirectX::SimpleMath::Vector2 GetMousePosViewportSpace() const;

	ImVec2 m_ViewportBounds[2];

	DirectX::SimpleMath::Vector2 m_MousePosViewSpace;

	bool HasActorSelected() const;

	std::shared_ptr<Actor> GetSelectedActor();

	void SwitchGizmoOperation(SDL_Keycode key);

	wchar_t* OpenScene(COMDLG_FILTERSPEC FileExt[], UINT ExtensionCount);

	void SelectActor(std::shared_ptr<Actor> actor);

	void ClearScene();

	void DeselectAll();

private:

	std::shared_ptr<Scene> m_Scene;

	std::shared_ptr<SceneRenderer> m_Renderer;

	std::shared_ptr<Actor> m_SelectedActor;

	DX::RenderTexture* m_ViewportImage;

	std::string m_ViewportName;

	bool bClickedOnObject = false;

	bool bShowDemoWindow = false;

	ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	ImGuizmo::MODE m_GizmoMode = ImGuizmo::MODE::WORLD;

	bool bIsGizmoActive = false;

};

