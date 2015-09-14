#pragma once
#include "Vertex.h"
#include "MyVector3d.h"

class SpringVertex{
public:
	Vertex *v;
	double m;
	MyVector3d velocity;
	bool mobility;

	SpringVertex(Vertex *_v, double _m = 1);
	void move(double dt);
};