#pragma once
#include "Vertex.h"

class Face;

class Halfedge
{
public:
	//いつもの
	Vertex *vertex;
	Face *face;
	Halfedge *pair;
	Halfedge *next;
	Halfedge *prev;
	
	int id;
	bool checked;//色々なところで、チェックしたかどうかに使う

	Halfedge(Vertex *v);
	~Halfedge();
	void draw();
	void debugPrint();
	void setID(int _id){ id = _id; }
};

