#pragma once

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;
class Texture;
class Material;
struct RawMesh;
struct Vertex
{
	Vector3 Position;
	Vector3 Normal;
	Vector2 TexCoord;
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





	void SaveMeshToDisk(std::string path);

	void LoadMeshFromDisk(std::string path);

	void DrawDepth() const;

	std::vector<Vertex>& GetVertices();
	std::vector<unsigned int>& GetIndices();
	std::shared_ptr<Material> GetMaterial() const { return m_Material; }
	void SetMaterial(std::shared_ptr<Material> val) { m_Material = val; }
	std::shared_ptr<RawMesh> GetRawMesh() const { return m_RawMesh; }
	void SetRawMesh(std::shared_ptr<RawMesh> val) { m_RawMesh = val; }
private:

	std::shared_ptr<RawMesh> m_RawMesh;

	std::shared_ptr<Material> m_Material;

};




