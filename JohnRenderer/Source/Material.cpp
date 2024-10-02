#include "pch.h"
#include "Material.h"
#include "Texture.h"

Material::Material()
{


}

void Material::Apply()
{
    auto context = Application::Get().GetContext  ();

    ID3D11ShaderResourceView* srvs[] =
    {
        m_BaseColorMap->GetSRV().Get(),
        m_NormalMap->GetSRV().Get(),
        m_RoughnessMap->GetSRV().Get(),
        m_MetallicMap->GetSRV().Get()
    };

    context->PSSetShaderResources (0, _countof(srvs), srvs);
}
