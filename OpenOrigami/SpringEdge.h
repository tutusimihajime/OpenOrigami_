#pragma once
#include "SpringVertex.h"
#include "Halfedge.h"

class SpringEdge{
public:
	double naturalLength;//自然長
	double length;//今の長さ
	double C;//バネ定数
	SpringVertex *sv, *ev;
	
	SpringEdge(Halfedge *he);
	SpringEdge::SpringEdge(SpringVertex *_sv, SpringVertex *_ev);
	void calculateLength();
	void calculateSpringPower(double dt);
};