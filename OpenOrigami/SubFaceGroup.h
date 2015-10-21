#pragma once 
#include <list>
#include <vector>
#include <iostream>
#include <GL/freeglut.h>
#include "Face.h"
#include "Halfedge.h"
#include "Vertex.h"

using namespace std;

class SubFaceGroup{
public:
	Face *oldFace;
	list<Vertex*> subvertices;
	list<Face*> subfaces;
	
	SubFaceGroup(Face *_oldFace, list<Face*> _subfaces);
	Face *SubFaceGroup::cpyFace(Face *_f);
	Vertex *SubFaceGroup::cpyVertex(Vertex *_v);
	void SubFaceGroup::mergeVertexPair(Vertex *v1, Vertex *v2);
	void SubFaceGroup::mergeAllVertexPair();
	Vertex *SubFaceGroup::createVertex(double x, double y, double z);
	Halfedge *SubFaceGroup::createHalfedge(Vertex *v);
	Face *SubFaceGroup::createFace(Halfedge *he);
	void SubFaceGroup::initializeSubfacesZ();
	void calcSubFacesNormal();
	void drawVertex(GLenum mode = GL_RENDER);
	void drawEdge(GLenum mode = GL_RENDER);
	void drawFace(GLenum mode = GL_RENDER);
	void debugPrint();
};