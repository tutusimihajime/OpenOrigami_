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
	void transPosition(double _x, double _y, double _z){
		v->transPosition(_x, _y, _z); 
	}
};