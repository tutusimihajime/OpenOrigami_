#include "Face.h"
#include "Vertex.h"
#include <Windows.h>
#include <gl/freeglut.h>
#include <list>
#include <iostream>

using namespace std;

Face::Face(Halfedge *he) {
	halfedge = he; // Face ¨ Halfedge ‚ÌƒŠƒ“ƒN
	normalizeNormal();
	Halfedge *he_in_f = he;
	do{
		he_in_f->face = this;
		he_in_f = he_in_f->next;
	} while (he_in_f != he);
}
Face::~Face()
{
}
void Face::normalizeNormal(){
	MyVector3d v1(halfedge->next->vertex->x - halfedge->vertex->x, halfedge->next->vertex->y - halfedge->vertex->y, halfedge->next->vertex->z - halfedge->vertex->z);
	MyVector3d v2(halfedge->next->next->vertex->x - halfedge->vertex->x, halfedge->next->next->vertex->y - halfedge->vertex->y, halfedge->next->next->vertex->z - halfedge->vertex->z);
	nv = v1*v2;
	nv.normalize();
}
void Face::draw(){
	glNormal3d(nv.x, nv.y, nv.z);
	glBegin(GL_POLYGON);

	Halfedge *halfedge_in_face = this->halfedge;
	do{
		glVertex3d(halfedge_in_face->vertex->x, halfedge_in_face->vertex->y, halfedge_in_face->vertex->z);
		halfedge_in_face = halfedge_in_face->next;
	} while (halfedge_in_face!= this->halfedge);
	glEnd();
}
void Face::drawBack(){
	glNormal3d(-nv.x, -nv.y, -nv.z);
	glBegin(GL_POLYGON);

	Halfedge *halfedge_in_face = this->halfedge;
	do{
		glVertex3d(halfedge_in_face->vertex->x, halfedge_in_face->vertex->y, halfedge_in_face->vertex->z);
		halfedge_in_face = halfedge_in_face->prev;
	} while (halfedge_in_face != this->halfedge);
	glEnd();
}
void Face::debugPrint(){

	cout << "f : ";
	
	Halfedge *halfedge_in_face = this->halfedge;
	do{
		cout << halfedge_in_face->vertex->id << " ";
		halfedge_in_face = halfedge_in_face->next;
	} while (halfedge_in_face != this->halfedge);
	cout << endl;
}
void Face::transPosition(MyVector3d vec){
	transPosition(vec.x, vec.y, vec.z);
}
void Face::transPosition(double x, double y, double z){
	Halfedge *he_in_face = this->halfedge;
	do{
		he_in_face->vertex->transPosition(x, y, z);
		he_in_face = he_in_face->next;
	} while (he_in_face != this->halfedge);
}
void Face::transPosition(Vector3d vec){
	transPosition(vec.x(), vec.y(), vec.z());
}