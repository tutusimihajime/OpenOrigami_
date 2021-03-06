#pragma once
#include "Face.h"
#include "Vertex.h"
#include <Windows.h>
#include <gl/freeglut.h>
#include <list>
#include <vector>
#include <iostream>

using namespace std;

Face::Face(Halfedge *he) {
	halfedge = he; // Face → Halfedge のリンク
	normalizeNormal();
	isDrawn = true;
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
void Face::draw(bool isEnableVisibleFlag){
	if (isDrawn || !isEnableVisibleFlag){
		glNormal3d(nv.x, nv.y, nv.z);
		glBegin(GL_POLYGON);

		Halfedge *halfedge_in_face = this->halfedge;
		do{
			glVertex3d(halfedge_in_face->vertex->x, halfedge_in_face->vertex->y, halfedge_in_face->vertex->z);
			halfedge_in_face = halfedge_in_face->next;
		} while (halfedge_in_face != this->halfedge);
		glEnd();
	}
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
void Face::calcCenterOfGravity(){
	
	g = new Vector3d(0, 0, 0);
	int n = 0;//Faceの中の頂点・辺の数
	Halfedge *he = halfedge;
	do{
		*g += Vector3d(he->vertex->x, he->vertex->y, he->vertex->z);
		n++;
		he = he->next;
	} while (he != halfedge);
	*g /= (float)n;

}
void Face::setZ(double z){
	Halfedge *he = halfedge;
	do{
		he->vertex->z = z;
		he = he->next;
	} while (he != halfedge);
}
void Face::reverse(){
	Halfedge *he = halfedge;
	vector<Vertex*> vList;
	vector<Halfedge*> hList;
	do{
		vList.push_back(he->vertex);
		hList.push_back(he);
		he = he->next;
	} while (he != halfedge);
	for (int i = 0; i < vList.size(); ++i){
		hList.at(i)->next = (i == 0) ? hList.at(vList.size() - 1) : hList.at(i - 1);
		hList.at(i)->prev = (i == vList.size() - 1) ? hList.at(0) : hList.at(i + 1);
		hList.at(i)->vertex = (i == vList.size() - 1) ? vList.at(0) : vList.at(i + 1);
		vList.at(i)->halfedge = (i == 0) ? hList.at(vList.size() - 1) : hList.at(i - 1);
	}
	normalizeNormal();
}
void Face::setInvisible(){
	this->isDrawn = false;
}
void Face::setVisible(){
	this->isDrawn = true;
}