#pragma once
using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

class Sky
{
public:

	bool LoadFromFile(const char* FileName);
	ComPtr<ID3D11ShaderResourceView> GetEnvMap() const;
	void SetEnvMap(ComPtr<ID3D11ShaderResourceView> val);
	ComPtr<ID3D11ShaderResourceView> GetSpecularIBL() const;
	void SetSpecularIBL(ComPtr<ID3D11ShaderResourceView> val);
	ComPtr<ID3D11ShaderResourceView> GetDiffuseIBL() const;
	void SetDiffuseIBL(ComPtr<ID3D11ShaderResourceView> val);
private:
	ComPtr<ID3D11ShaderResourceView> m_DiffuseIBL;
	ComPtr<ID3D11ShaderResourceView> m_SpecularIBL;
	ComPtr<ID3D11ShaderResourceView> m_EnvMap;
};

