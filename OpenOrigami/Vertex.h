#pragma once
#include "MyVector3d.h"

class Halfedge;

class Vertex
{
public:
	
	double x, y, z;
	int id;// 0 origin
	Halfedge *halfedge;

	bool checked;

	Vertex();
	~Vertex();

	Vertex(double _x, double _y, double _z);
	void setID(int n);
	void draw();
	inline void setPosition(double _x, double _y, double _z){ x = _x; y = _y; z = _z; }
	inline void setPosition(MyVector3d v){ setPosition(v.x, v.y, v.z); }
	inline void transPosition(double _x, double _y, double _z){ x += _x; y += _y; z += _z; }
	inline void transPosition(MyVector3d v){ transPosition(v.x, v.y, v.z); }
	inline void scale(const double s) { x *= s; y *= s; z *= s; }
	void debugPrint();
};

