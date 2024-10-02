#pragma once

class Texture;

class Material
{
public:

	Material();

	void Apply();

	std::shared_ptr<Texture> GetBaseColorMap() const { return m_BaseColorMap; }
	void SetBaseColorMap(std::shared_ptr<Texture> val) { m_BaseColorMap = val; }
	std::shared_ptr<Texture> GetNormalMap() const { return m_NormalMap; }
	void SetNormalMap(std::shared_ptr<Texture> val) { m_NormalMap = val; }
	std::shared_ptr<Texture> GetMetallicMap() const { return m_MetallicMap; }
	void SetMetallicMap(std::shared_ptr<Texture> val) { m_MetallicMap = val; }
	std::shared_ptr<Texture> GetRoughnessMap() const { return m_RoughnessMap; }
	void SetRoughnessMap(std::shared_ptr<Texture> val) { m_RoughnessMap = val; }

private:
	std::shared_ptr<Texture> m_BaseColorMap;
	std::shared_ptr<Texture> m_NormalMap;
	std::shared_ptr<Texture> m_MetallicMap;
	std::shared_ptr<Texture> m_RoughnessMap;

};

