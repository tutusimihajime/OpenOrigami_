#pragma once
#include "SpringVertex.h"
#include "Halfedge.h"

class SpringEdge{
public:
	double naturalLength;//©‘R’·
	double length;//¡‚Ì’·‚³
	double C;//ƒoƒl’è”
	SpringVertex *sv, *ev;
	
	SpringEdge(Halfedge *he);
	SpringEdge::SpringEdge(SpringVertex *_sv, SpringVertex *_ev);
	void calculateLength();
	void calculateSpringPower(double dt);
};