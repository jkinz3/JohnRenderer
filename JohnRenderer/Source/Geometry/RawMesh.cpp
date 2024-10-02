#include "pch.h"
#include "RawMesh.h"

#define EPSILON 1e-6

template<typename T>
double GetArea2D(const T& v1, const T& v2, const T& v3)
{
	return 0.5 * (v1.x * ((double)v3.y - v2.y) + v2.x * ((double)v1.y - v3.y) + v3.x * ((double)v2.y - v1.y));
}




template<typename T>
int OnLeftSideOfLine2D(const T& p0, const T& p1, const T& p2)
{
	double area = GetArea2D(p0, p2, p1);
	if(std::abs(area) < EPSILON)
	{
		return 0;
	}
	else if (area > 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

template<typename T>
bool PointInTriangle2D(const T& p0, const T& p1, const T& p2, const T& pp)
{
	int c1 = OnLeftSideOfLine2D(p0, p1, pp);
	int c2 = OnLeftSideOfLine2D(p1, p2, pp);
	int c3 = OnLeftSideOfLine2D(p2, p0, pp);
	return (c1 >= 0) && (c2 >= 0) && (c3 >= 0);
}

template <int ofs_x, int ofs_y, int ofs_z, typename TReal>
inline void NewellNormal (Vector3& out, int num, TReal* x, TReal* y, TReal* z) {
	// Duplicate the first two vertices at the end
	x[(num + 0) * ofs_x] = x[0];
	x[(num + 1) * ofs_x] = x[ofs_x];

	y[(num + 0) * ofs_y] = y[0];
	y[(num + 1) * ofs_y] = y[ofs_y];

	z[(num + 0) * ofs_z] = z[0];
	z[(num + 1) * ofs_z] = z[ofs_z];

	TReal sum_xy = 0.0, sum_yz = 0.0, sum_zx = 0.0;

	TReal *xptr = x + ofs_x, *xlow = x, *xhigh = x + ofs_x * 2;
	TReal *yptr = y + ofs_y, *ylow = y, *yhigh = y + ofs_y * 2;
	TReal *zptr = z + ofs_z, *zlow = z, *zhigh = z + ofs_z * 2;

	for (int tmp = 0; tmp < num; tmp++)
	{
		sum_xy += (*xptr) * ((*yhigh) - (*ylow));
		sum_yz += (*yptr) * ((*zhigh) - (*zlow));
		sum_zx += (*zptr) * ((*xhigh) - (*xlow));

		xptr += ofs_x;
		xlow += ofs_x;
		xhigh += ofs_x;

		yptr += ofs_y;
		ylow += ofs_y;
		yhigh += ofs_y;

		zptr += ofs_z;
		zlow += ofs_z;
		zhigh += ofs_z;
	}
	out = Vector3(sum_yz, sum_zx, sum_xy);
}


struct NGONEncoder
{
	NGONEncoder()
		:m_LastNGONFirstIndex((unsigned int) -1){}

	void ngonEncodeTriangle(RawFace* tri)
	{
		assert(tri->m_NumIndices == 3);

		if(IsConsideredSameAsLastNgon  (tri))
		{
			std::swap(tri->m_Indices[0], tri->m_Indices[2]);
			std::swap(tri->m_Indices[1], tri->m_Indices[2]);

		}

		m_LastNGONFirstIndex = tri->m_Indices[0];
		
	}

	void ngonEncodeQuad(RawFace* tri1, RawFace* tri2)
	{
		assert(tri1->m_NumIndices == 3);
		assert(tri2->m_NumIndices == 3);
		assert(tri1->m_Indices[0] == tri2->m_Indices[0]);

		if(IsConsideredSameAsLastNgon  (tri1))
		{
			// Right-rotate indices for tri1 (index 2 becomes the new fanning vertex)
			std::swap(tri1->m_Indices[0], tri1->m_Indices[2]);
			std::swap(tri1->m_Indices[1], tri1->m_Indices[2]);

			// Left-rotate indices for tri2 (index 2 becomes the new fanning vertex)
			std::swap(tri2->m_Indices[1], tri2->m_Indices[2]);
			std::swap(tri2->m_Indices[0], tri2->m_Indices[2]);

			assert(tri1->m_Indices[0] == tri2->m_Indices[0]);
		}

		m_LastNGONFirstIndex = tri2->m_Indices[0];
	}

	bool IsConsideredSameAsLastNgon(const RawFace * tri) const
	{
		assert(tri->m_NumIndices == 3);
		return tri->m_Indices[0] == m_LastNGONFirstIndex;
	}
private:
	unsigned int m_LastNGONFirstIndex;
};

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

void RawMesh::ExtractTriangles(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{

	uint32_t numOut = 0;
	uint32_t maxOut = 0;
	bool bGetNormals = true;
	for (unsigned int a = 0; a < m_NumFaces; a++)
	{
		RawFace& face = m_Faces[a];
		if (face.m_NumIndices <= 4)
		{
			bGetNormals = false;
		}
		if (face.m_NumIndices <= 3)
		{
			numOut++;
		}
		else
		{
			numOut += face.m_NumIndices - 2;
			maxOut = std::max(maxOut, face.m_NumIndices);
		}
	}

	assert(numOut != m_NumFaces);

	Vector3* nor_out = nullptr;

	m_Primitivetypes |= PrimitiveType::Triangle;
	m_Primitivetypes &= ~PrimitiveType::fPolygon;

	m_Primitivetypes |= PrimitiveType::NGONEncodingFlag;

	RawFace* out = new RawFace[numOut](), *curOut = out;
	std::vector<Vector3> temp_verts3d(maxOut + 2);
	std::vector<Vector2> temp_verts(maxOut + 2);

	NGONEncoder encoder;

	const Vector3* verts = m_Vertices.data();

	std::unique_ptr<bool[]> done(new bool[maxOut]);
	for(unsigned int a = 0; a < m_NumFaces; a++)
	{
		RawFace& face = m_Faces[a];
		unsigned int* idx = face.m_Indices.data();
		int num = (int)face.m_NumIndices;
		int ear = 0;
		int tmp;
		int prev = num - 2;
		int next = 0;
		int max = num;

		RawFace* const last_face = curOut;

		if(face.m_NumIndices <= 3)
		{
			RawFace& nFace = *curOut++;
			nFace.m_NumIndices = face.m_NumIndices;
			nFace.m_Indices = face.m_Indices;
			
			if(nFace.m_NumIndices == 3)
			{
				encoder.ngonEncodeTriangle  (&nFace);
			}
				continue;
		}
		else if(face.m_NumIndices == 4)
		{
			unsigned int start_vertex = 0;
			for(unsigned int i  = 0; i < 4; i++)
			{
				const Vector3& v0 = verts[face.m_Indices[(i + 3) % 4]];
				const Vector3& v1 = verts[face.m_Indices[(i + 2) % 4]];
				const Vector3& v2 = verts[face.m_Indices[(i + 1) % 4]];

				const Vector3& v = verts[face.m_Indices[i]];

				Vector3 left = (v0 - v);
				Vector3 diag = (v1 - v);
				Vector3 right = (v2 - v);

				left.Normalize  ();
				diag.Normalize  ();
				right.Normalize  ();
				const float angle = std::acos  (left.Dot  (diag)) + std::acos  (right.Dot(diag));

				if(angle > XM_PI)
				{
					start_vertex = i;
					break;
				}

			}

			const unsigned int temp[] = { face.m_Indices[0], face.m_Indices[1], face.m_Indices[2], face.m_Indices[3] };

			RawFace& nFace = *curOut++;
			nFace.m_NumIndices = 3;
			nFace.m_Indices = face.m_Indices;

			nFace.m_Indices[0] = temp[start_vertex];
			nFace.m_Indices[1] = temp[(start_vertex +1) % 4];
			nFace.m_Indices[2] = temp[(start_vertex +2) % 4];

			RawFace& sFace = *curOut++;
			sFace.m_NumIndices = 3;
			sFace.m_Indices.resize  (3);

			sFace.m_Indices[0] = temp[start_vertex];
			sFace.m_Indices[1] = temp[(start_vertex + 2) % 4];
			sFace.m_Indices[2] = temp[(start_vertex + 3) % 4];

			encoder.ngonEncodeQuad  (&nFace, &sFace);
			continue;

		}
		else
		{
			for(tmp = 0; tmp < max; tmp++)
			{
				temp_verts3d[tmp] = verts[idx[tmp]];
			}

			Vector3 n;
			NewellNormal <3, 3, 3> (n, max, &temp_verts3d.front().x, &temp_verts3d.front().y, &temp_verts3d.front().z);
			if(nor_out)
			{
				for(tmp = 0; tmp < max; tmp++)
				{
					nor_out[idx[tmp]] = n;
				}
			}

			const float ax = (n.x > 0 ? n.x : -n.x);
			const float ay = (n.y > 0 ? n.y : -n.y);
			const float az = (n.z > 0 ? n.z : -n.z);

			unsigned int ac = 0; 
			unsigned int bc = 1;

			float inv = n.z;
			if(ax > ay)
			{
				if(ax > az)
				{
					ac = 1; bc = 2;
					inv = n.x;
				}
			}
			else if(ay > az)
			{
				ac = 2;
				bc = 0;
				inv = n.y;
			}

			if(inv < 0.f)
			{
				std::swap(ac, bc);
			}

			for (tmp = 0; tmp < max; ++tmp)
			{
				temp_verts[tmp].x = GetVector3ComponentFromIndex  (verts[idx[tmp]], ac);
				temp_verts[tmp].y = GetVector3ComponentFromIndex(verts[idx[tmp]], bc);
				done[tmp] = false;
			}

			while(num > 3)
			{
				int num_fount = 0;
				for(ear = next;; prev = ear, ear = next)
				{
					for (next = ear + 1; done[(next >= max ? next = 0 : next)]; ++next);
					{
						if(next < ear)
						{
							if(++num_fount == 2)
							{
								break;
							}
						}
						const Vector2* pnt1 = &temp_verts[ear],
							*pnt0 = &temp_verts[prev],
							*pnt2 = &temp_verts[next];

						if(OnLeftSideOfLine2D(*pnt0, *pnt2, *pnt1) == 1)
						{
							continue;
						}
						Vector2 left = *pnt0 - *pnt1;
						Vector2 right = *pnt2 - *pnt1;

						left.Normalize  ();
						right.Normalize  ();
						auto mul = left.Dot  (right);

						if(std::abs(mul - 1.f) < EPSILON || std::abs(mul + 1.f) < EPSILON)
						{
							continue;
						}

						for(tmp = 0; tmp < max; tmp++)
						{
							const Vector2& vtmp = temp_verts[tmp];
							if(vtmp != *pnt1 && vtmp != *pnt2 && vtmp != *pnt0 && PointInTriangle2D  (*pnt0, *pnt1, *pnt2, vtmp))
							{
								break;
							}
							if(tmp != max)
							{
								continue;
							}

							break;
						}
						if(num_fount == 2)
						{
							num = 0;
							break;
						}
						RawFace& nFace = *curOut++;
						nFace.m_NumIndices = 3;

						if(nFace.m_Indices.size() == 0)
						{
							nFace.m_Indices.resize(3);
						}

						nFace.m_Indices[0] = prev;
						nFace.m_Indices[1] = ear;
						nFace.m_Indices[2] = next;

						done[ear] = true;
						--num;
					}
					if(num > 0)
					{
						// We have three indices forming the last 'ear' remaining. Collect them.
						RawFace& nface = *curOut++;
						nface.m_NumIndices = 3;
						if (nface.m_Indices.size() == 0)
						{
							nface.m_Indices.resize(3);
						}

						for (tmp = 0; done[tmp]; ++tmp);
						nface.m_Indices[0] = tmp;

						for (++tmp; done[tmp]; ++tmp);
						nface.m_Indices[1] = tmp;

						for (++tmp; done[tmp]; ++tmp);
						nface.m_Indices[2] = tmp;
					}
				}

				for(RawFace* f = last_face; f != curOut;)
				{
					unsigned int* i = f->m_Indices.data();

					i[0] = idx[i[0]];
					i[1] = idx[i[1]];
					i[3] = idx[i[2]];

					encoder.ngonEncodeTriangle  (f);
					f++;

				}
				
			}
			
		}
	}

	//pack it in, pack it in

	
	for(int i = 0; i < m_NumVertices; i++)
	{
		Vertex newVert = {};

		newVert.Position = m_Vertices[i];
		newVert.Normal = m_Normals[i];
		newVert.TexCoord = m_TexCoords[i];

		vertices.push_back  (newVert);
	}

	UINT numTris = (unsigned int)(curOut - out);
	for(int i = 0; i < numTris; i++)
	{
		RawFace f = out[i];
		for(int j = 0; j < f.m_NumIndices; j++)
		{
			indices.push_back  (f.m_Indices[j]);
		}
	}



}

PrimitiveType GetPrimitiveTypeForIndexCount(unsigned int n)
{
	
		return n > 3 ? PrimitiveType::fPolygon : (PrimitiveType)(1u << ((n)-1));
	


}
