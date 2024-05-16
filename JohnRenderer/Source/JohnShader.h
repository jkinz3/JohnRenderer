#pragma once

class Texture;
using Microsoft::WRL::ComPtr;
class JohnShader
{
public:

	JohnShader();

	void Apply();
	void LoadFromFiles(std::wstring VertFile, std::wstring PixelFile);

	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11PixelShader> m_PixelShader;

	ComPtr<ID3D11InputLayout> m_InputLayout;

	struct TransformConstants
	{
		XMMATRIX MVP;
		XMMATRIX Model;
	};

	struct ShadingConstants
	{
		XMVECTOR CamPos;
		float NumPointLights;
	};

	ConstantBuffer<TransformConstants> m_TransformCB;
	ConstantBuffer<ShadingConstants> m_ShadingCB;

	void SetModel(DirectX::SimpleMath::Matrix val);
	void SetView(DirectX::SimpleMath::Matrix val);
	void SetProj(DirectX::SimpleMath::Matrix val);

	void SetEnvironmentTextures(ComPtr<ID3D11ShaderResourceView> diff, ComPtr<ID3D11ShaderResourceView> spec);

	void SetNumPointLights(int numLights);

	void Recompile();

private:
	Matrix m_Model;
	Matrix m_View;
	Matrix m_Proj;

	ComPtr<ID3D11ShaderResourceView> m_DiffIBL;
	ComPtr<ID3D11ShaderResourceView> m_SpecIBL;

	ComPtr<ID3D11SamplerState> m_DefaultSampler;

	int m_NumPointLights;

	std::wstring m_VertFileSource;
	std::wstring m_PixelFileSource;
};

