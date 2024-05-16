#include "pch.h"
#include "JohnMeshSerializer.h"
#include "JohnMesh.h"
JohnMeshSerializer::JohnMeshSerializer(std::shared_ptr<JohnMesh> inMesh)
	:m_Mesh(inMesh)
{

}

void JohnMeshSerializer::WriteToDisk(std::string  FileName, std::ofstream& outStream)
{
	assert(m_Mesh);

	auto vertices = m_Mesh->GetVertices ();
	auto indices = m_Mesh->GetIndices ();

	JohnMeshHeader header;
	header.VertCount = vertices.size();
	header.IndexCount = indices.size ();

	size_t vertDataSize = vertices.size() * sizeof(Vertex);
	size_t indexDataSize = indices.size() * sizeof(unsigned int);
	outStream.write(reinterpret_cast<const char*>(&header), sizeof(JohnMeshHeader));
	outStream.write(reinterpret_cast<const char*>(vertices.data()), vertDataSize);
	outStream.write(reinterpret_cast<const char*>(indices.data()), indexDataSize);
	
	std::string path = FileName.substr (0, FileName.find_last_of ("\\/"));
	WriteTextures (path);




}

std::shared_ptr<JohnMesh> JohnMeshSerializer::LoadFromDisk(std::string  FileName, std::ifstream& inStream)
{
	
	JohnMeshHeader header;

	std::vector<Vertex> newVerts;
	std::vector<unsigned int> newIndices;

	inStream.read(reinterpret_cast<char*>(&header), sizeof(JohnMeshHeader));

	newVerts.resize (header.VertCount);
	newIndices.resize (header.IndexCount);

	size_t vertDataSize = header.VertCount * sizeof(Vertex);
	size_t indexDataSize = header.IndexCount * sizeof(unsigned int);
	inStream.read(reinterpret_cast<char*>(newVerts.data()), vertDataSize);
	inStream.read(reinterpret_cast<char*>(newIndices.data()), indexDataSize);



	auto newMesh = std::make_shared<JohnMesh>(newVerts, newIndices);
	newMesh->Build();
	return newMesh;
}

void JohnMeshSerializer::WriteTextures(std::string Path)
{
	
}
