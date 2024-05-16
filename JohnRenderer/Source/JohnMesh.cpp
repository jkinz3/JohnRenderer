#include "pch.h"
#include "JohnMesh.h"
#include "Application.h"
#include "Texture.h"

JohnMesh::JohnMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	:m_Vertices(vertices), m_Indices(indices)
{

}

void JohnMesh::Build()
{
	assert(m_Vertices.size() != 0);
	assert(m_Indices.size() != 0);

	auto device = Application::Get().GetDevice ();

	const size_t vertDataSize = m_Vertices.size() * sizeof(Vertex);
	const size_t indexDataSize = m_Indices.size() * sizeof(unsigned int);

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = (UINT)vertDataSize;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = m_Vertices.data();

	ThrowIfFailed (
		device->CreateBuffer(
			&desc,
			&data,
			m_VertexBuffer.ReleaseAndGetAddressOf ()
		)
	);

	desc.ByteWidth = (UINT)indexDataSize;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	data.pSysMem = m_Indices.data();

	ThrowIfFailed (
		device->CreateBuffer(
			&desc,
			&data,
			m_IndexBuffer.ReleaseAndGetAddressOf ()
		)
	);

}

void JohnMesh::Draw()
{
	auto context = Application::Get().GetContext ();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers (0, 1, m_VertexBuffer.GetAddressOf (), &stride, &offset);
	context->IASetIndexBuffer (m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

 	ID3D11ShaderResourceView* srvs[] =
 	{
 		m_BaseColorMap->GetSRV().Get(),
 		m_NormalMap->GetSRV().Get(),
 		m_RoughnessMap->GetSRV().Get(),
 		m_MetallicMap->GetSRV().Get()
 	};

	context->PSSetShaderResources (0, _countof(srvs), srvs);

	context->DrawIndexed (m_Indices.size(), 0, 0);
}

void JohnMesh::DrawOutline()
{

	auto context = Application::Get().GetContext ();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers (0, 1, m_VertexBuffer.GetAddressOf (), &stride, &offset);
	context->IASetIndexBuffer (m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	context->DrawIndexed (m_Indices.size(), 0, 0);
}

void JohnMesh::SaveMeshToDisk(std::string path)
{
	assert(m_Vertices.size() != 0 && m_Indices.size() != 0);

	std::string CachePath("Cache/");
	std::string fullName = std::string(CachePath + path);
	std::filesystem::create_directories(CachePath);
	std::ofstream outFile(fullName.c_str(), std::ios::binary | std::ios::out);
	if (!outFile)
	{
		return;
	}

	JohnMeshHeader header;
	header.VertCount = m_Vertices.size();
	header.IndexCount = m_Indices.size();

	outFile.write(reinterpret_cast<const char*>(&header), sizeof(JohnMeshHeader));
	outFile.write(reinterpret_cast<const char*>(m_Vertices.data()), m_Vertices.size() * sizeof(Vertex));
	outFile.write(reinterpret_cast<const char*>(m_Indices.data()), m_Indices.size() * sizeof(unsigned int));
	outFile.close();
}

void JohnMesh::DrawDepth() const
{
	auto context = Application::Get().GetContext ();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers (0, 1, m_VertexBuffer.GetAddressOf (), &stride, &offset);
	context->IASetIndexBuffer (m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	context->DrawIndexed (m_Indices.size(), 0, 0);
}

std::vector<Vertex>& JohnMesh::GetVertices()
{
	return m_Vertices;

}

std::vector<unsigned int>& JohnMesh::GetIndices()
{
	return m_Indices;
}

