#include "pch.h"
#include "RawMesh.h"

bool RawMesh::HasPositions() const
{
	return m_Vertices.size() > 0;

}

bool RawMesh::HasFaces() const
{
	return m_Faces.size() > 0;
}

bool RawMesh::HasNormals() const
{
	return m_Normals.size() > 0;
}



bool RawMesh::HasTexCoords(unsigned int index) const
{
	return !m_TexCoords.empty();
}

PrimitiveType GetPrimitiveTypeForIndexCount(unsigned int n)
{
	
		return n > 3 ? PrimitiveType::fPolygon : (PrimitiveType)(1u << ((n)-1));
	


}
