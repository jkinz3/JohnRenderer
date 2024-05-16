#include "pch.h"
#include "Sky.h"
#include "Texture.h"
using Microsoft::WRL::ComPtr;

bool Sky::LoadFromFile(const char* FileName)
{
	auto device = Application::Get().GetDevice ();
	auto context = Application::Get().GetContext ();

	ID3D11UnorderedAccessView* const nullUAV[] = { nullptr };
	ID3D11Buffer* const nullBuffer[] = { nullptr };


    auto CreateUAV = [&](ComPtr<ID3D11Texture2D> texture, UINT mipSplice)
        {
            D3D11_TEXTURE2D_DESC texDesc = {};
            texture->GetDesc (&texDesc);
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = texDesc.Format;
            if (texDesc.ArraySize == 1)
            {
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = mipSplice;
            }
            else
            {
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = mipSplice;
                uavDesc.Texture2DArray.FirstArraySlice = 0;
                uavDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
            }

            ComPtr<ID3D11UnorderedAccessView> uav;
            ThrowIfFailed (
                device->CreateUnorderedAccessView(texture.Get(), &uavDesc, uav.ReleaseAndGetAddressOf ())
            );
            return uav;
        };

    auto CreateComputeShader = [&](const std::wstring& csFile)
        {
            auto blob = DX::ReadData (csFile.c_str ());

            ComPtr<ID3D11ComputeShader> shader;
            ThrowIfFailed (
                device->CreateComputeShader(blob.data (), blob.size (), nullptr, shader.ReleaseAndGetAddressOf ())
            );

            return shader;
        };

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    sampDesc.AddressU = sampDesc.AddressV = sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> ComputeSampler;
    ThrowIfFailed (
        device->CreateSamplerState(&sampDesc, ComputeSampler.ReleaseAndGetAddressOf ())
    );

    std::shared_ptr<Texture> EnvMapUnfiltered = std::make_shared<Texture>();

    EnvMapUnfiltered->CreateTextureCube (1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
    auto unfilteredUAV = CreateUAV(EnvMapUnfiltered->GetTexture (), 0);

    {
        ComPtr<ID3D11ComputeShader> equirect2CubeProgram = CreateComputeShader(L"Shaders/Runtime/equirect2cube.cso");
        std::shared_ptr<Texture> envTextureEquiRect = std::make_shared<Texture>();
        envTextureEquiRect->LoadFromHDRTexture(FileName, DXGI_FORMAT_R32G32B32A32_FLOAT, 1);

        context->CSSetShader(equirect2CubeProgram.Get(), nullptr, 0);
        context->CSSetShaderResources(0, 1, envTextureEquiRect->GetSRV ().GetAddressOf ());
        context->CSSetSamplers(0, 1, ComputeSampler.GetAddressOf ());
        context->CSSetUnorderedAccessViews(0, 1, unfilteredUAV.GetAddressOf(), nullptr);
        context->Dispatch(EnvMapUnfiltered->GetWidth () / 32, EnvMapUnfiltered->GetHeight () / 32, 6);
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    }

    context->GenerateMips(EnvMapUnfiltered->GetSRV ().Get());
    m_EnvMap = EnvMapUnfiltered->GetSRV ();
    
        struct SpecCB
        {
            float roughness;
            float padding[3];
        };
        ComPtr<ID3D11ComputeShader> spProgram = CreateComputeShader(L"Shaders/Runtime/spmap.cso");
        ConstantBuffer<SpecCB> specCB(device);

        std::shared_ptr<Texture> specIBL = std::make_shared<Texture>();
        specIBL->CreateTextureCube (1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
        m_SpecularIBL = specIBL->GetSRV ();

        for (int arrayslice = 0; arrayslice < 6; ++arrayslice)
        {
            const UINT subResourceIndex = D3D11CalcSubresource (0, arrayslice, specIBL->GetLevels ());
            context->CopySubresourceRegion(specIBL->GetTexture ().Get(), subResourceIndex, 0, 0, 0,
                EnvMapUnfiltered->GetTexture ().Get (), subResourceIndex, nullptr);
        }
        context->CSSetShaderResources(0, 1, EnvMapUnfiltered->GetSRV ().GetAddressOf ());
        context->CSSetShader(spProgram.Get(), nullptr, 0);

        const float deltaRoughness = 1.f / std::max(float(specIBL->GetLevels () - 1), 1.f);
        for (UINT level = 1, size = 512; level < specIBL->GetLevels (); ++level, size / 2)
        {
            const UINT numGroups = std::max<UINT>(1, size / 32);
            auto specUAV = CreateUAV(specIBL->GetTexture (), level);
            const SpecCB spConstants = { level * deltaRoughness };
            specCB.SetData (context, spConstants);

            auto cb = specCB.GetBuffer ();
            context->CSSetConstantBuffers(0, 1, &cb);
            context->CSSetUnorderedAccessViews(0, 1, specUAV.GetAddressOf(), nullptr);
            context->Dispatch(numGroups, numGroups, 6);
        }

        context->CSSetConstantBuffers(0, 1, nullBuffer);
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

    
    
        auto irMapProgram = CreateComputeShader(L"Shaders/Runtime/irmap.cso");
        std::shared_ptr<Texture> diffIBL = std::make_shared<Texture>();
        diffIBL->CreateTextureCube (32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
        m_DiffuseIBL = diffIBL->GetSRV ();
        auto diffUAV = CreateUAV(diffIBL->GetTexture (), 0);

        context->CSSetShaderResources(0, 1, specIBL->GetSRV ().GetAddressOf ());
        context->CSSetUnorderedAccessViews(0, 1, diffUAV.GetAddressOf(), nullptr);
        context->CSSetShader(irMapProgram.Get(), nullptr, 0);
        context->Dispatch(diffIBL->GetWidth () / 32, diffIBL->GetHeight () / 32, 6);
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    


	return true;
}

ComPtr<ID3D11ShaderResourceView> Sky::GetEnvMap() const
{
	return m_EnvMap;
}

void Sky::SetEnvMap(ComPtr<ID3D11ShaderResourceView> val)
{
	m_EnvMap = val;
}

ComPtr<ID3D11ShaderResourceView> Sky::GetSpecularIBL() const
{
	return m_SpecularIBL;
}

void Sky::SetSpecularIBL(ComPtr<ID3D11ShaderResourceView> val)
{
	m_SpecularIBL = val;
}

ComPtr<ID3D11ShaderResourceView> Sky::GetDiffuseIBL() const
{
	return m_DiffuseIBL;
}

void Sky::SetDiffuseIBL(ComPtr<ID3D11ShaderResourceView> val)
{
	m_DiffuseIBL = val;
}
