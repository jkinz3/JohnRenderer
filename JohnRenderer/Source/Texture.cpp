#include "pch.h"
#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Texture::Texture(ComPtr<ID3D11Texture2D> tex, ComPtr<ID3D11ShaderResourceView> srv)
	:m_Texture(tex), m_SRV(srv)
{
	D3D11_TEXTURE2D_DESC desc = {};
	tex->GetDesc (&desc);
	SetWidth (desc.Width);
	SetHeight (desc.Height);
}


void Texture::LoadFromHDRTexture(const std::string file, DXGI_FORMAT format, UINT levels /*= 0*/)
{
	auto device = Application::Get().GetDevice();
	auto context = Application::Get().GetContext();

	if (!stbi_is_hdr(file.c_str()))
	{
		return;
	}
	SetPath(file);
	float* pixels = stbi_loadf(file.c_str(), &m_Width, &m_Height, &m_Components, 4);

	if (pixels)
	{
		m_Levels = (levels > 0) ? levels : NumMipMapLevels();
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = m_Width;
		texDesc.Height = m_Height;
		texDesc.MipLevels = levels;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		if (levels == 0)
		{
			texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = pixels;
		data.SysMemPitch = m_Width * 4 * sizeof(float);
		size_t sz = sizeof(float);

		ThrowIfFailed(
			device->CreateTexture2D(&texDesc, &data, m_Texture.ReleaseAndGetAddressOf())
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		ThrowIfFailed(
			device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRV.ReleaseAndGetAddressOf())
		);

	}


}

void Texture::LoadFromDDSTexture(char const* file)
{
	auto device = Application::Get().GetDevice();
	auto context = Application::Get().GetContext();


}

void Texture::LoadTexture(const std::string file, bool sRGB)
{
	//get extension
	size_t indexLocation = file.rfind('.', file.length());
	if (indexLocation == std::string::npos)
	{
		return;
	}

	SetPath(file);
	auto device = Application::Get().GetDevice();

	std::string fileExt = file.substr(indexLocation + 1, file.length() - indexLocation);
	if (fileExt == "dds")
	{
		LoadFromDDSTexture(file.c_str());
	}
	else
	{


		auto context = Application::Get().GetContext();
		std::wstring wideString(file.begin(),file.end());
		DirectX::CreateWICTextureFromFileEx(device, context, wideString.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0, D3D11_RESOURCE_MISC_GENERATE_MIPS, sRGB ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_IGNORE_SRGB, reinterpret_cast<ID3D11Resource**>(m_Texture.ReleaseAndGetAddressOf()), m_SRV.ReleaseAndGetAddressOf());


	}

}
void Texture::CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format, UINT levels /*= 0*/)
{
	auto device = Application::Get().GetDevice();

	m_Width = width;
	m_Height = height;
	m_Levels = (levels > 0) ? levels : NumMipMapLevels();

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = levels;
	texDesc.ArraySize = 6;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | texDesc.MiscFlags | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ThrowIfFailed(
		device->CreateTexture2D(&texDesc, nullptr, m_Texture.ReleaseAndGetAddressOf())
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = -1;

	ThrowIfFailed(
		device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRV.ReleaseAndGetAddressOf())
	);
}

void Texture::CreateTexture(UINT width, UINT height, DXGI_FORMAT format, UINT levels /*= 0*/)
{
	auto device = Application::Get().GetDevice();

	m_Width = width;
	m_Height = height;
	m_Levels = (levels > 0) ? levels : NumMipMapLevels();

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = levels;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	if (levels == 0)
	{
		texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	ThrowIfFailed(
		device->CreateTexture2D(&texDesc, nullptr, m_Texture.ReleaseAndGetAddressOf())
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	ThrowIfFailed(
		device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRV.ReleaseAndGetAddressOf())
	);

}

ComPtr<ID3D11Texture2D> Texture::GetTexture() const
{
	return m_Texture;
}

void Texture::SetTexture(ComPtr<ID3D11Texture2D> val)
{
	m_Texture = val;
}

ComPtr<ID3D11ShaderResourceView> Texture::GetSRV() const
{
	return m_SRV;
}

void Texture::SetSRV(ComPtr<ID3D11ShaderResourceView> val)
{
	m_SRV = val;

	// Extrapolate info from SRV. messy but we're gonna go with it
	
	ID3D11Resource* res = nullptr;
	m_SRV->GetResource (&res);
	res->QueryInterface (m_Texture.ReleaseAndGetAddressOf ());
	D3D11_TEXTURE2D_DESC desc;
	m_Texture->GetDesc (&desc);

	m_Width = desc.Width;
	m_Height = desc.Height;
	m_Levels = desc.MipLevels;

}

UINT Texture::GetWidth() const
{
	return m_Width;
}

void Texture::SetWidth(UINT val)
{
	m_Width = val;
}

UINT Texture::GetHeight() const
{
	return m_Height;
}

void Texture::SetHeight(UINT val)
{
	m_Height = val;
}

UINT Texture::GetComponents() const
{
	return m_Components;
}

void Texture::SetComponents(UINT val)
{
	m_Components = val;
}

UINT Texture::GetLevels() const
{
	return m_Levels;
}

void Texture::SaveToDisk(std::string Path)
{
	std::string OutFile = Path + m_Name;

	std::wstring wOutFile(OutFile.begin (), OutFile.end ());
	HRESULT hr = DirectX::SaveDDSTextureToFile (Application::Get().GetContext (), m_Texture.Get (), wOutFile.c_str ());

	ThrowIfFailed (hr);

}
