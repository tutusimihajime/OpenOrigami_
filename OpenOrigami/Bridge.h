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
	Halfedge *he1, *he2;//bridgeと接続する相手側のHalfedge
	double h1, h2;
	float m_z;
	list<Face*> faces;//変えちゃダメ
	list<Vertex*> vertices;//変えちゃダメ
	Bridge(Halfedge *_he1, Halfedge *_he2);
	void draw();
	void normalizeFaces();
	Vertex *createVertex(Vector3d vec);
	Halfedge *createHalfedge(Vertex *v);
	Face *createFace(Halfedge *he);
	Face *addFace(list<Vertex*>);
	void calculateM_Z();
	void reCalc();
};