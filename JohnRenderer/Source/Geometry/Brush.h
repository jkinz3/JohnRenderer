#pragma once

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct Winding
{
	int NumPoints;
	int MaxPoints;
	Vector3 Points[8];
};

struct BrushFace
{
	Vector3 PlanePoints[3];
	Plane plane;
	BrushFace* next;
	BrushFace* prev;
	Winding* face_Winding;


};

class Brush
{
public:

	Vector3 Mins;
	Vector3 Maxs;
	BrushFace* BrushFaces;


};

