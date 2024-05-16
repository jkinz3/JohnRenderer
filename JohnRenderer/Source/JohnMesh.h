#pragma once

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;
class Texture;
struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector3 TexCoord;
};

struct JohnMeshHeader
{
	size_t VertCount;
	size_t IndexCount;
};

class JohnMesh
{
public:
	JohnMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	JohnMesh() {}

	void Build();

	void Draw();

	
	void DrawOutline();



	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;

	std::string m_Name;

	ComPtr<ID3D11Buffer> m_VertexBuffer;
	ComPtr<ID3D11Buffer> m_IndexBuffer;

	std::shared_ptr<Texture> GetMetallicMap() const { return m_MetallicMap; }
	void SetMetallicMap(std::shared_ptr<Texture> val) { m_MetallicMap = val; }
	std::shared_ptr<Texture> GetNormalMap() const { return m_NormalMap; }
	void SetNormalMap(std::shared_ptr<Texture> val) { m_NormalMap = val; }
	std::shared_ptr<Texture> GetBaseColorMap() const { return m_BaseColorMap; }
	void SetBaseColorMap(std::shared_ptr<Texture> val) { m_BaseColorMap = val; }
	std::shared_ptr<Texture> GetRoughnessMap() const { return m_RoughnessMap; }
	void SetRoughnessMap(std::shared_ptr<Texture> val) { m_RoughnessMap = val; }



	void SaveMeshToDisk(std::string path);

	void LoadMeshFromDisk(std::string path);

	void DrawDepth() const;

	std::vector<Vertex>& GetVertices();
	std::vector<unsigned int>& GetIndices();
private:

	std::shared_ptr<Texture> m_BaseColorMap;
	std::shared_ptr<Texture> m_NormalMap;
	std::shared_ptr<Texture> m_MetallicMap;
	std::shared_ptr<Texture> m_RoughnessMap;


};




