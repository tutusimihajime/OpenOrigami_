#pragma once
#include <Eigen/Geometry>
#include "Bridge.h"
#include "GeometryElement2EigenVector.h"
extern double d;
using namespace Eigen;

Bridge::Bridge(Halfedge *_he1, Halfedge *_he2){
	this->setVisible();
	he1 = _he1;
	he2 = _he2;
	he1->bridge = he2->bridge = this;

	//create vertex
	double h = 0.5 * d;
	double w = 0.6;
	Vertex *v1, *v2, *v3, *v4;
	v1 = he1->vertex;
	v2 = he1->next->vertex;
	v3 = he2->vertex;
	v4 = he2->next->vertex;
	Vector3d vec1, vec2, vec3, vec4, vec1_5, vec3_5;
	vec1 = createVector3d(v1);
	vec1_5 = createVector3d(he1->prev->vertex);
	vec2 = createVector3d(v2);
	vec3 = createVector3d(v3);
	vec3_5 = createVector3d(he2->prev->vertex);
	vec4 = createVector3d(v4);
	Vector3d normal1,normal1_tmp, normal2, normal2_tmp;
	
	//normal1 = createVector3d(he1->face->nv);//要修正
	normal1 = vec1_5 - vec1;
	normal1.z() = 0;
	normal1.normalize();
	normal1_tmp = vec2 - vec1;
	normal1_tmp.z() = 0;
	normal1_tmp.normalize();
	normal1 = normal1_tmp.cross(normal1);
	normal1.normalize();
	
	//normal2 = createVector3d(he2->face->nv);//要修正
	normal2 = vec3_5 - vec3;
	normal2.z() = 0;
	normal2.normalize();
	normal2_tmp = vec4 - vec3;
	normal2_tmp.z() = 0;
	normal2_tmp.normalize();
	normal2 = normal2_tmp.cross(normal2);
	normal2.normalize();

	Vector3d vec_h1 = vec2 - vec1;
	vec_h1.z() = 0;
	vec_h1.normalize();
	vec_h1 = vec_h1.cross(normal1);
	vec_h1.normalize();

	Vector3d vec_h2 = vec4 - vec3;
	vec_h2.z() = 0;
	vec_h2.normalize();
	vec_h2 = vec_h2.cross(normal2);
	vec_h2.normalize();
	
	//this->h1 = 0.5*fabs(vec4.z() - vec1.z());
	//this->h2 = 0.5*fabs(vec3.z() - vec2.z());
	this->h1 = this->h2 =d* 0.5*he1->itmp;
	//cout << "(he1->itmp, he2->itmp, Max1, Max2) = (" << he1->itmp << ", " << he2->itmp <<", "<<he1->itmpMax<<", "<<he2->itmpMax << ")" << endl;
	//cout << "h1 = " << h1 << ", h2 = " << h2 << endl;
	//
	Vector3d vec5, vec6, vec7, vec8;
	vec5 = vec1 + 0.5*(1-w)*(vec4-vec1) + h1* vec_h1;//he1->itmp*
	vec6 = vec2 + 0.5*(1-w)*(vec3-vec2) + h2* vec_h2;//he1->itmp*
	vec7 = vec5 + w *(vec4-vec1);//(vec4.z() - vec1.z())*Vector3d(0, 0, 1);
	vec8 = vec6 + w *(vec3-vec2);// (vec3.z() - vec2.z())*Vector3d(0, 0, 1);

	Vertex *v5, *v6, *v7, *v8;
	v5 = createVertex(vec5);
	v6 = createVertex(vec6);
	v7 = createVertex(vec7);
	v8 = createVertex(vec8);

	//create face
	list<list<Vertex*>> vListList;
	list<Vertex*> vList;
	vList.push_back(v1);
	vList.push_back(v5);
	vList.push_back(v6);
	vList.push_back(v2);
	vListList.push_back(vList);
	vList.clear();
	vList.push_back(v5);
	vList.push_back(v7);
	vList.push_back(v8);
	vList.push_back(v6);
	vListList.push_back(vList);
	vList.clear();
	vList.push_back(v7);
	vList.push_back(v4);
	vList.push_back(v3);
	vList.push_back(v8);
	vListList.push_back(vList);
	Face *f[3];
	int i = 0;
	for (list<list<Vertex*>>::iterator it = vListList.begin(); it != vListList.end(); ++it){
		f[i] = addFace(*it);
		f[i]->type = 2;
		++i;
	}

	//make pairing 10/6 バグ・・・直った
	Halfedge *he_p = he2;
	he1->setPair(f[0]->halfedge->prev);
	f[0]->halfedge->next->setPair(f[1]->halfedge->prev);
	f[1]->halfedge->next->setPair(f[2]->halfedge->prev);
	f[2]->halfedge->next->setPair(he_p);
}
void Bridge::draw(bool isEnableVisibleFlag){
	if (this->isDrawn||!isEnableVisibleFlag){
		for (list<Face*>::iterator it_f = faces.begin(); it_f != faces.end(); ++it_f){

			Halfedge *he = (*it_f)->halfedge;

			//drawVertex
			glDisable(GL_LIGHTING);
			glPointSize(5);
			glColor3f(.3, .3, .3);
			glBegin(GL_POINTS);
			do{
				Vertex *v = he->vertex;
				glVertex3d(v->x, v->y, v->z);
				he = he->next;
			} while (he != (*it_f)->halfedge);
			glEnd();

			//drawEdge
			glDisable(GL_LIGHTING);
			glEnable(GL_LINE_SMOOTH);
			glLineWidth(1);
			glColor3f(.6, .6, .6);
			do{
				Vertex *v1 = he->vertex;
				Vertex *v2 = he->next->vertex;
				glBegin(GL_LINES);
				glVertex3d(v1->x, v1->y, v1->z);
				glVertex3d(v2->x, v2->y, v2->z);
				glEnd();

				he = he->next;
			} while (he != (*it_f)->halfedge);

			//drawFace
			glEnable(GL_LIGHTING);
			GLfloat materialColor1[] = { 1, 0.2, 0.2, 1 };
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor1);
			GLfloat materialColor2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 30);
			MyVector3d nv = (*it_f)->nv;
			glNormal3d(nv.x, nv.y, nv.z);
			glBegin(GL_POLYGON);
			do{
				Vertex *v = he->vertex;
				glVertex3d(v->x, v->y, v->z);

				he = he->next;
			} while (he != (*it_f)->halfedge);
			glEnd();
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
	
}
void Bridge::normalizeFaces(){
	for (list<Face*>::iterator it_f = faces.begin(); it_f != faces.end(); ++it_f){
		(*it_f)->normalizeNormal();
	}
}
Vertex *Bridge::createVertex(Vector3d vec){
	Vertex *v = new Vertex(vec.x(), vec.y(), vec.z());
	vertices.push_back(v);
	return v;
}
Halfedge *Bridge::createHalfedge(Vertex *v){
	Halfedge *he = new Halfedge(v);
	return he;
}
Face *Bridge::createFace(Halfedge *he){
	Face *f = new Face(he);
	faces.push_back(f);
	return f;
}
Face *Bridge::addFace(list<Vertex*> vlist){
	vector<Halfedge*> halfedges4pairing;
	list<Vertex*>::iterator it_v;
	for (it_v = vlist.begin(); it_v != vlist.end(); ++it_v){
		Halfedge *he = createHalfedge(*it_v);
		halfedges4pairing.push_back(he);
	}
	for (int i = 0; i < vlist.size(); ++i){
		int i_next, i_prev;
		i_next = (i == vlist.size() - 1) ? 0 : i + 1;
		i_prev = (i == 0) ? vlist.size() - 1 : i - 1;
		halfedges4pairing[i]->next = halfedges4pairing[i_next];
		halfedges4pairing[i]->prev = halfedges4pairing[i_prev];
	}
	return createFace(halfedges4pairing[0]);
}
void Bridge::reCalc(){

	//create vertex
	double h = 0.5 * d;
	double w = 0.6;
	Vertex *v1, *v2, *v3, *v4;
	v1 = he1->vertex;
	v2 = he1->next->vertex;
	v3 = he2->vertex;
	v4 = he2->next->vertex;
	Vector3d vec1, vec2, vec3, vec4, vec1_5, vec3_5;
	vec1 = createVector3d(v1);
	vec1_5 = createVector3d(he1->prev->vertex);
	vec2 = createVector3d(v2);
	vec3 = createVector3d(v3);
	vec3_5 = createVector3d(he2->prev->vertex);
	vec4 = createVector3d(v4);
	Vector3d normal1, normal1_tmp, normal2, normal2_tmp;

	normal1 = vec1_5 - vec1;
	normal1.z() = 0;
	normal1.normalize();
	normal1_tmp = vec2 - vec1;
	normal1_tmp.z() = 0;
	normal1_tmp.normalize();
	normal1 = normal1_tmp.cross(normal1);
	normal1.normalize();

	normal2 = vec3_5 - vec3;
	normal2.z() = 0;
	normal2.normalize();
	normal2_tmp = vec4 - vec3;
	normal2_tmp.z() = 0;
	normal2_tmp.normalize();
	normal2 = normal2_tmp.cross(normal2);
	normal2.normalize();

	Vector3d vec_h1 = vec2 - vec1;
	vec_h1.z() = 0;
	vec_h1.normalize();
	vec_h1 = vec_h1.cross(normal1);
	vec_h1.normalize();

	Vector3d vec_h2 = vec4 - vec3;
	vec_h2.z() = 0;
	vec_h2.normalize();
	vec_h2 = vec_h2.cross(normal2);
	vec_h2.normalize();

	//this->h1 = 0.5*fabs(vec4.z() - vec1.z());
	//this->h2 = 0.5*fabs(vec3.z() - vec2.z());
	this->h1 = this->h2 = d* 0.5*he1->itmp;
	//cout << "(he1->itmp, he2->itmp, Max1, Max2) = (" << he1->itmp << ", " << he2->itmp <<", "<<he1->itmpMax<<", "<<he2->itmpMax << ")" << endl;
	//cout << "h1 = " << h1 << ", h2 = " << h2 << endl;
	//
	Vector3d vec5, vec6, vec7, vec8;
	vec5 = vec1 + 0.5*(1 - w)*(vec4 - vec1) + h1* vec_h1;//he1->itmp*
	vec6 = vec2 + 0.5*(1 - w)*(vec3 - vec2) + h2* vec_h2;//he1->itmp*
	vec7 = vec5 + w *(vec4 - vec1);//(vec4.z() - vec1.z())*Vector3d(0, 0, 1);
	vec8 = vec6 + w *(vec3 - vec2);// (vec3.z() - vec2.z())*Vector3d(0, 0, 1);

	//ここから↓が、配列使ってんのと一緒だから、リストの中身を変えちゃダメ
	Vertex *v5, *v6, *v7, *v8;
	v6 = he1->pair->prev->vertex;
	v5 = he1->pair->next->next->vertex;
	v8 = he2->pair->next->next->vertex;
	v7 = he2->pair->prev->vertex;
	v5->setPosition(vec5.x(), vec5.y(), vec5.z());
	v6->setPosition(vec6.x(), vec6.y(), vec6.z());
	v7->setPosition(vec7.x(), vec7.y(), vec7.z());
	v8->setPosition(vec8.x(), vec8.y(), vec8.z());
}
void Bridge::calculateM_Z(){
	m_z = 0.25*(he1->vertex->z +he1->next->vertex->z +he2->vertex->z + he2->next->vertex->z);
}
void Bridge::setInvisible(){
	this->isDrawn = false;
}
void Bridge::setVisible(){
	this->isDrawn = true;
}