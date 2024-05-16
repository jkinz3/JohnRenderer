#pragma once
#include "Scene.h"
#include "RenderTexture.h"
class JohnShader;
class SelectionOutlineShader;
class DepthOnlyShader;
class Sky;
using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
using namespace DirectX;

static constexpr int MaxPointLights = 64;

class SceneRenderer
{
public:
	~SceneRenderer();

	void Initialize();

	void DrawScene(Scene* InScene, Matrix view, Matrix proj);

	void SetupLights(Scene* InScene);

	void DrawPostProcess();

	void DrawSelectionOutline(std::shared_ptr<Actor> selectedActor);

	void SwitchToWireframe();

	void SwitchToLit();

	void OnViewportResize(int width, int height);

	DX::RenderTexture* GetPrePostProccessTarget() const;

	DX::RenderTexture* GetViewportRenderTarget() const;

	void CreateDefaultAssets();

	std::shared_ptr<Texture> CreateDefaultTexture(uint16_t color, DXGI_FORMAT format);

	void ClearRenderTargets();
	
	std::shared_ptr<Texture> GetDefaultDiffuse() const { return m_DefaultDiffuse; }
	std::shared_ptr<Texture> GetDefaultNormal() const { return m_DefaultNormal; }
	std::shared_ptr<Texture> GetDefaultMetallic() const { return m_DefaultMetallic; }
	std::shared_ptr<Texture> GetDefaultRoughness() const { return m_DefaultRoughness; }

	void RecompileShaders();
private:

	ID3D11RasterizerState* GetCurrentRSState() const;

	std::shared_ptr<Sky> m_Sky;

	std::unique_ptr<DirectX::GeometricPrimitive> m_SkyMesh;
	std::unique_ptr<DX::SkyboxEffect> m_SkyEffect;

	ComPtr<ID3D11InputLayout> m_SkyInputLayout;
	


	std::shared_ptr<JohnShader> m_PBRShader;
	std::shared_ptr<SelectionOutlineShader> m_SelectionOutlineShader;
	std::shared_ptr<DepthOnlyShader> m_DepthOnlyShader;

	ComPtr<ID3D11SamplerState> m_DefaultSampler;

	std::unique_ptr<CommonStates> m_CommonStates;

	enum class EDrawMode
	{
		Lit,
		Wireframe
	};

	EDrawMode m_CurrentDrawMode = EDrawMode::Lit;

	std::unique_ptr<DX::RenderTexture> m_PrePostProcessTarget;
	std::unique_ptr<DX::RenderTexture> m_ViewportRenderTarget;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	ComPtr<ID3D11Texture2D> m_DepthStencil;
	ComPtr<ID3D11DepthStencilState> m_StencilWriteState;
	ComPtr<ID3D11DepthStencilState> m_StencilMaskState;
	ComPtr<ID3D11DepthStencilState> m_StandardState;
	std::unique_ptr<DirectX::ToneMapPostProcess> m_ToneMap;

	

	std::unique_ptr<GeometricPrimitive> m_Sphere;
	std::unique_ptr<PBREffect> m_PBREffect;


	ComPtr<ID3D11InputLayout> m_InputLayout;


	ComPtr<ID3D11ShaderResourceView> m_ARM;
	ComPtr<ID3D11ShaderResourceView> m_Albedo;
	ComPtr<ID3D11ShaderResourceView> m_Normal;

	std::shared_ptr<Texture> m_DefaultDiffuse;
	std::shared_ptr<Texture> m_DefaultNormal;
	std::shared_ptr<Texture> m_DefaultMetallic;
	std::shared_ptr<Texture> m_DefaultRoughness;

	ComPtr<ID3D11Buffer> m_PointLightBuffer;
	ComPtr<ID3D11ShaderResourceView> m_PointLightSRV;

	struct GPUPointLight 
	{
		Vector3 Position;
		float pack1;
		Vector3 Color;
		float pack2;
		float Intensity;
	};

	Matrix m_CachedView;
	Matrix m_CachedProj;
};

