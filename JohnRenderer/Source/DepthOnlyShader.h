#pragma once

using Microsoft::WRL::ComPtr;
class DepthOnlyShader
{
public:

	DepthOnlyShader();

	void Apply();
	void LoadFromFiles(std::wstring VertFile);

	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11PixelShader> m_PixelShader;

	ComPtr<ID3D11InputLayout> m_InputLayout;

	struct TransformConstants
	{
		XMMATRIX MVP;
		XMMATRIX Model;
	};


	ConstantBuffer<TransformConstants> m_TransformCB;


	void SetModel(DirectX::SimpleMath::Matrix val);
	void SetView(DirectX::SimpleMath::Matrix val);
	void SetProj(DirectX::SimpleMath::Matrix val);


	void Recompile();

private:
	Matrix m_Model;
	Matrix m_View;
	Matrix m_Proj;


	std::wstring m_VertFileSource;
};

