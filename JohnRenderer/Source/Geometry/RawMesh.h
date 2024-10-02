#pragma once
#include "AABB.h"

using namespace DirectX::SimpleMath;

#ifndef MAX_INDICES
#define MAX_INDICES 0x7fff
#endif

#ifndef MAX_VERTICES 
#define MAX_VERTICES 0x7fffffff
#endif

#ifndef MAX_FACES 
#define MAX_FACES 0x7fffffff
#endif



#ifndef MAX_TEXCOORDS
#define MAX_TEXCOORDS 0x8
#endif

//I hate this. This sucks. But I don't feel like figuring out the whole
// turning integer index into X/Y/Z thingy
inline float GetVector3ComponentFromIndex(Vector3 vec, int index)
{
	switch(index)
	{
	case 0:
		return vec.x;
	case 1:
		return vec.y;
	case 2:
		return vec.z;
	default:
		return vec.x;
	}
}

struct RawFace
{
	unsigned int m_NumIndices;

	std::vector<unsigned int> m_Indices;

	RawFace() 
		:m_NumIndices(0)
	{

	}

	~RawFace()
	{
		m_Indices.clear  ();
	}


	RawFace(const RawFace & o)
		:m_NumIndices(0)
	{
		*this = o;
	}

	RawFace &operator=(const RawFace& o)
	{
		if(&o == this)
		{
			return *this;
		}

		m_Indices.clear  ();
		m_NumIndices = o.m_NumIndices;
		if(m_NumIndices)
		{
			m_Indices.reserve  (m_NumIndices);
			m_Indices = o.m_Indices;
		}
		else
		{
			m_Indices.clear();
		}
		return *this;
	}

	bool operator==(const RawFace& o) const
	{
		if(m_Indices == o.m_Indices)
		{
			return true;
		}

		if(m_Indices.empty  ()&& m_NumIndices != o.m_NumIndices)
		{
			return false;
		}

		if(m_Indices.empty())
		{
			return false;
		}

		for(unsigned int i = 0; i < m_NumIndices; i++)
		{
			if(m_Indices[i] != o.m_Indices[i])
			{
				return false;
			}
		}

		return true;
	}

	bool operator!=(const RawFace& o) const
	{
		return !(*this == o);
	}

};
enum PrimitiveType
{
	Point = 0x1,
	Line = 0x2,
	Triangle = 0x3,
	fPolygon = 0x8,
	NGONEncodingFlag = 0x10
	};


PrimitiveType GetPrimitiveTypeForIndexCount(unsigned int n);
struct RawMesh
{
	unsigned int m_Primitivetypes;

	unsigned int m_NumVertices;

	unsigned int m_NumFaces;

	std::vector<Vector3> m_Vertices;

	std::vector<Vector3> m_Normals;

	std::vector<Vector2> m_TexCoords;

	std::vector<RawFace> m_Faces;

	AABB m_AABB;

	RawMesh()
		:m_Primitivetypes(),
		m_NumVertices(0),
		m_NumFaces(0),
		m_AABB()
	{

	}

	~RawMesh()
	{

		m_Vertices.clear  ();
		m_Normals.clear  ();
		m_TexCoords.clear  ();
		m_Faces.clear  ();
		
	}

	bool HasPositions() const;

	bool HasFaces() const;

	bool HasNormals() const;

	bool HasTexCoords(unsigned int index) const;

	void ExtractTriangles(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);


};