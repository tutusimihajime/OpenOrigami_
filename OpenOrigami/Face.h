#pragma once
#include "Halfedge.h"
#include "MyVector3d.h"

class Face
{
public:
	Halfedge *halfedge;
	MyVector3d nv;
	int id;

	Face(Halfedge *he);
	~Face();
	void normalizeNormal();
	void draw();
	void debugPrint();
	void transPosition(MyVector3d vec);
	void transPosition(double x, double y, double z);
	void setID(int _id){ id = _id; }
};

