#pragma once
#include <Eigen/Geometry>
#include "Bridge.h"
#include "GeometryElement2EigenVector.h"
extern double d;
using namespace Eigen;

Bridge::Bridge(Halfedge *he1, Halfedge *he2){
	//create vertex
	double h = 0.5 * d;
	double w = 0.3 * h;
	Vertex *v1, *v2, *v3, *v4;
	v1 = he1->vertex;
	v2 = he1->next->vertex;
	v3 = he2->vertex;
	v4 = he2->next->vertex;
	Vector3d vec1, vec2, vec3, vec4;
	vec1 = createVector3d(v1);
	vec2 = createVector3d(v2);
	vec3 = createVector3d(v3);
	vec4 = createVector3d(v4);
	Vector3d normal1, normal2;
	normal1 = createVector3d(he1->face->nv);
	normal2 = createVector3d(he2->face->nv);

	Vector3d vec_h = (vec2 - vec1).cross(normal1);
	vec_h.normalize();
	Vector3d vec5, vec6, vec7, vec8;
	vec5 = (1 - w) / 2.f*(vec4 - vec1) + he1->itmp*h * vec_h + vec1;
	vec6 = (1 - w) / 2.f*(vec3 - vec2) + he1->itmp*h * vec_h + vec2;
	vec7 = vec5 + w*(vec4 - vec1);
	vec8 = vec6 + w*(vec3 - vec2);

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
void Bridge::draw(){
	for (list<Face*>::iterator it_f = faces.begin(); it_f != faces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		
		//drawVertex
		
		glBegin(GL_POINTS);
		do{
			Vertex *v = he->vertex;
			glVertex3d(v->x, v->y, v->z);
			he = he->next;
		} while (he != (*it_f)->halfedge);
		glEnd();

		//drawEdge
		do{
			Vertex *v1 = he->vertex;
			Vertex *v2 = he->next->vertex;
			glBegin(GL_LINE);
			glVertex3d(v1->x, v1->y, v1->z);
			glVertex3d(v2->x, v2->y, v2->z);
			glEnd();

			he = he->next;
		} while (he != (*it_f)->halfedge);
		
		//drawFace
		glBegin(GL_POLYGON);
		do{
			Vertex *v = he->vertex;
			glVertex3d(v->x, v->y, v->z);
			
			he = he->next;
		} while (he != (*it_f)->halfedge);
		glEnd();

	}
	
}
Vertex *Bridge::createVertex(Vector3d vec){
	Vertex *v = new Vertex(vec.x(), vec.y(), vec.z());
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