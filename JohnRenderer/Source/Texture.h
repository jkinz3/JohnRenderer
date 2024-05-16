#pragma once

using Microsoft::WRL::ComPtr;
class Texture
{
public:

	Texture(ComPtr<ID3D11Texture2D> tex, ComPtr<ID3D11ShaderResourceView> srv);

	Texture(){}
	void LoadFromHDRTexture(const std::string file, DXGI_FORMAT format, UINT levels = 0);
	void LoadFromDDSTexture(char const* file);
	void LoadTexture(const std::string file, bool sRGB);

	void CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0);
	void CreateTexture(UINT width, UINT height, DXGI_FORMAT format, UINT levels = 0);

	ComPtr<ID3D11Texture2D> GetTexture() const;
	void SetTexture(ComPtr<ID3D11Texture2D> val);
	ComPtr<ID3D11ShaderResourceView> GetSRV() const;
	void SetSRV(ComPtr<ID3D11ShaderResourceView> val);
	UINT GetWidth() const;
	void SetWidth(UINT val);
	UINT GetHeight() const;
	void SetHeight(UINT val);
	UINT GetComponents() const;
	void SetComponents(UINT val);
	UINT GetLevels() const;

	UINT NumMipMapLevels()
	{
		UINT levels = 1;
		while ((m_Width | m_Height) >> levels)
		{
			++levels;
		};
		return levels;
	}
	std::string GetName() const { return m_Name; }
	void SetName(std::string val) { m_Name = val; }
	std::string GetPath() const { return m_Path; }
	void SetPath(std::string val) { m_Path = val; }

	void SaveToDisk(std::string Path);


private:

	int m_Width, m_Height, m_Components;
	UINT m_Levels;

	ComPtr<ID3D11ShaderResourceView> m_SRV;
	ComPtr<ID3D11Texture2D> m_Texture;

	std::string m_Name;
	std::string m_Path;




};

