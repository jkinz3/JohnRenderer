#include "pch.h"
#include "JohnMesh.h"

using namespace DirectX::SimpleMath;

void JohnMesh::Build(ID3D11Device* device)
{
	assert(m_Vertices.size() != 0);
	assert(m_Faces.size() != 0);

	m_Stride = sizeof(Vertex);
	m_NumIndices = static_cast<UINT>(m_Faces.size() * 3);

	const size_t vertexDataSize = m_Vertices.size() * sizeof(Vertex);
	const size_t indexDataSize = m_Faces.size() * sizeof(Face);
	{
		D3D11_BUFFER_DESC vertDesc = {};
		vertDesc.ByteWidth = (UINT)vertexDataSize;
		vertDesc.Usage = D3D11_USAGE_DEFAULT;
		vertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertDesc.CPUAccessFlags = 0;
		vertDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = &m_Vertices[0];
		DX::ThrowIfFailed(
			device->CreateBuffer(
				&vertDesc, &data, m_VertexBuffer.ReleaseAndGetAddressOf()
			)
		);

	}
	{
		D3D11_BUFFER_DESC indexDesc = {};
		indexDesc.ByteWidth = (UINT)indexDataSize;
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = &m_Faces[0];

		DX::ThrowIfFailed( device->CreateBuffer( &indexDesc, &data, m_IndexBuffer.ReleaseAndGetAddressOf() ) );
	}


}

void TestFunction(int x)
{

}

int TestFunction2(int x)
{
	return 1;
}
int TestFunction3(int x, std::string string)
{
	return 1;
}

void Test()
{

}

void JohnMesh::Draw(ID3D11DeviceContext* context)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &m_Stride, &m_Offset);
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(m_NumIndices, 0, 0);
}

std::vector<Vertex>* JohnMesh::GetVertices()
{
	return &m_Vertices;
}

std::vector<Face>* JohnMesh::GetFaces()
{
	return &m_Faces;
}

DirectX::SimpleMath::Matrix JohnMesh::GetTransformationMatrix()
{
	return Matrix::Identity;
}
