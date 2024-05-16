#pragma once
class JohnMesh;

class JohnMeshSerializer
{
public:

	JohnMeshSerializer(std::shared_ptr<JohnMesh> inMesh);
	JohnMeshSerializer() {}

	void WriteToDisk(std::string FileName, std::ofstream& outSstream);
	std::shared_ptr<JohnMesh> LoadFromDisk(std::string FileName, std::ifstream& inStream);

	void WriteTextures(std::string Path);

private:

	std::shared_ptr<JohnMesh> m_Mesh;
};

