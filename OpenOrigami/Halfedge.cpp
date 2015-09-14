#include "Halfedge.h"
#include <iostream>
#include <Windows.h>
#include <gl/freeglut.h>

using namespace std;
Halfedge::Halfedge(Vertex *v) {
	vertex = v; // Halfedge ¨ Vertex ‚ÌƒŠƒ“ƒN 
	pair = NULL;
	if (v->halfedge == NULL) {
		v->halfedge = this; // Vertex ¨ Halfedge ‚ÌƒŠƒ“ƒN 
	}
}

Halfedge::~Halfedge()
{
}
void Halfedge::draw(){
	if (this->next == NULL){
		cout << "Halfedge::draw() : next == NULL" << endl;
		return;
	}
	glBegin(GL_LINES);
	glVertex3d(vertex->x, vertex->y, vertex->z);
	glVertex3d(next->vertex->x, next->vertex->y, next->vertex->z);
	glEnd();
}
void Halfedge::debugPrint(){
	cout << "he : " << vertex->id << " -> " << next->vertex->id << endl;
}
