#include "SpringVertex.h"

SpringVertex::SpringVertex(Vertex *_v, double _m){
	v = _v;
	velocity = MyVector3d(0, 0, 0);
	m = _m;
	mobility = true;
}

void SpringVertex::move(double dt){
	MyVector3d p = velocity;
	p.scale(dt);
	v->transPosition(p);
}