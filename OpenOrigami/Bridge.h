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
	Halfedge *he1, *he2;
	double h1, h2;
	list<Face*> faces;
	list<Vertex*> vertices;
	Bridge(Halfedge *_he1, Halfedge *_he2);
	void draw();
	void normalizeFaces();
	Vertex *createVertex(Vector3d vec);
	Halfedge *createHalfedge(Vertex *v);
	Face *createFace(Halfedge *he);
	Face *addFace(list<Vertex*>);
};