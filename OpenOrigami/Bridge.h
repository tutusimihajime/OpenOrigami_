#pragma once
#include <list>
#include <vector>
#include <iostream>
#include <GL/freeglut.h>
#include "Face.h"
#include "Halfedge.h"
#include "Vertex.h"
using namespace std;

class Bridge{
public:
	list<Face*> faces;
	Bridge(Halfedge *he1, Halfedge *he2);
	void draw();
	void normalizeFaces();
	Vertex *createVertex(Vector3d vec);
	Halfedge *createHalfedge(Vertex *v);
	Face *createFace(Halfedge *he);
	Face *addFace(list<Vertex*>);
};