#include "ReconstructingModel.h"
#include <list>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace std;
using namespace Eigen;


void relocationFaces(Model *mod)
{
	const double d = 10;
	int *overlapOrder;
	overlapOrder = new int[mod->faces.size()];
	//重なり順を決定
	
	for (int i = 0; i < mod->faces.size(); ++i){
		overlapOrder[i] = 0;
	}
	for (int i = 0; i < mod->faces.size(); ++i){		
		for (int j = 0; j < i; ++j){
			if (mod->overlapRelation.coeff(i,j) == 1){
				overlapOrder[i]++;
			}else if (mod->overlapRelation.coeff(i,j) == 2){
				overlapOrder[j]++;
			}
		}
	}
	list<int> tmplist;
	for (int i = 0; i < mod->faces.size(); ++i){
		tmplist.push_back(overlapOrder[i]);
	}
	tmplist.sort();
	tmplist.unique();
	/* //Debug
	cout << "tmplist = ";
	for (list<int>::iterator it = tmplist.begin(); it != tmplist.end(); ++it){
		cout << *it << ", ";
	}
	cout << endl;*/
	for (int i = 0; i < mod->faces.size(); ++i){
		int j = 0;
		for (list<int>::iterator it = tmplist.begin(); it != tmplist.end(); ++it){
			if (*it == overlapOrder[i]){
				overlapOrder[i] = j;
			}
			j++;
		}
	}
	//面を分離
	for (list<Vertex*>::iterator it_v = mod->vertices.begin(); it_v != mod->vertices.end(); ++it_v){
		(*it_v)->halfedge = NULL;
	}
	for (int i = 0; i < mod->faceVector.size(); ++i){
		Face *f = mod->faceVector.at(i);
		Halfedge *he_in_f = f->halfedge;
		do{
			if (he_in_f->vertex->halfedge == NULL){
				he_in_f->vertex->halfedge = he_in_f;
			}else{
				he_in_f->vertex = mod->cpyVertex(he_in_f->vertex);
				he_in_f->vertex->halfedge = he_in_f;
			}
			he_in_f = he_in_f->next;
		} while (he_in_f != f->halfedge);;
	}
	
	//面を移動
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->transPosition(d * overlapOrder[i] * Vector3d(0, 0, 1));
		mod->faceVector.at(i)->itmp = overlapOrder[i];
	}

	//Debug
	
	//cout << mod->overlapRelation << endl;cout << "overlapOrder = ";for (int i = 0; i < mod->faces.size(); ++i){	cout <<overlapOrder[i]<<", ";	}cout << endl;
	
}
bool compFaceItmp(Face *f1, Face *f2){
	return f1->itmp < f2->itmp;
}
Vector3d createVector3d(Halfedge *he){
	return Vector3d(he->next->vertex->x - he->vertex->x, he->next->vertex->y - he->vertex->y, he->next->vertex->z - he->vertex->z);
}
Vector3d createVector3d(Vertex *v){
	return Vector3d(v->x, v->y, v->z);
}
Vector3d createVector3d(MyVector3d v){
	return Vector3d(v.x, v.y, v.z);
}

void createBridge(Model *mod, Halfedge *he){
	
	//create vertex
	const double h = 2;
	const double w = 0.6;
	Vertex *v1, *v2, *v3, *v4;
	v1 = he->vertex;
	v2 = he->next->vertex;
	v3 = he->pair->vertex;
	v4 = he->pair->next->vertex;
	Vector3d vec1, vec2, vec3, vec4;
	vec1 = createVector3d(v1);
	vec2 = createVector3d(v2);
	vec3 = createVector3d(v3);
	vec4 = createVector3d(v4);
	Vector3d normal1, normal2;
	normal1 = createVector3d(he->face->nv);
	normal2 = createVector3d(he->pair->face->nv);

	Vector3d vec_h = (vec2 - vec1).cross(normal1);
	vec_h.normalize();
	Vector3d vec5, vec6, vec7, vec8;
	vec5 = (1-w)/2.f*(vec4 - vec1) + he->itmp*h * vec_h + vec1;
	vec6 = (1-w)/2.f*(vec3 - vec2) + he->itmp*h * vec_h + vec2;
	vec7 = vec5 + w*(vec4 - vec1);
	vec8 = vec6 + w*(vec3 - vec2);

	Vertex *v5, *v6, *v7, *v8;
	v5 = mod->createVertex(vec5);
	v6 = mod->createVertex(vec6);
	v7 = mod->createVertex(vec7);
	v8 = mod->createVertex(vec8);
	
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
		f[i] = mod->addFace(*it);
		++i;
	}
	return;
	//make pairing
	Halfedge *he_p = he->pair;
	he->pair->setPair(f[0]->halfedge->prev);
	f[0]->halfedge->next->setPair(f[1]->halfedge->prev);
	f[1]->halfedge->next->setPair(f[2]->halfedge->prev);
	f[2]->halfedge->next->setPair(he_p);


}
void bridgeEdges(Model *mod)
{
	//face -> itmp　は、overlapOrder
	//halfedge -> itmp は、 connectionHeight
	for (list<Halfedge*>::iterator it_e = mod->halfedges.begin(); it_e != mod->halfedges.end(); ++it_e){
		(*it_e)->itmp = 0;
	}
	int faces_num = mod->faces.size();
	list<Face*> sortedFaces(mod->faces.begin(), mod->faces.end());
	sortedFaces.sort(compFaceItmp);
	for (list<Face*>::iterator it_f = sortedFaces.begin(); it_f != sortedFaces.end(); ++it_f){
	
		Face *f1 = (*it_f);
		Halfedge *he_in_f = f1->halfedge;
		do{
			int maxHeight = 0;
			if (he_in_f->pair != NULL){
				Face *f2 = he_in_f->pair->face;
				if (mod->overlapRelation.coeff(f1->id, f2->id) == 2){
					maxHeight = max(maxHeight, he_in_f->pair->itmp);
				}
				he_in_f->itmp = maxHeight + 1;
			}
			he_in_f = he_in_f->next;
		} while (he_in_f != (*it_f)->halfedge);

	}
	//Debug
	cout << "halfedges.itmp = ";for (list<Halfedge*>::iterator it_e = mod->halfedges.begin(); it_e != mod->halfedges.end(); ++it_e){cout << (*it_e)->itmp << ", ";	}cout << endl;
	
	//create bridge
	for (list<Halfedge*>::iterator it_e = mod->halfedges.begin(); it_e != mod->halfedges.end(); ++it_e){
		(*it_e)->checked = false;
	}
	for (list<Halfedge*>::iterator it_e = mod->halfedges.begin(); it_e != mod->halfedges.end(); ++it_e){
		if (!(*it_e)->checked){			
			if ((*it_e)->pair != NULL){
				if ((*it_e)->next->vertex != (*it_e)->pair->vertex){
					createBridge(mod, *it_e);
				}
				(*it_e)->pair->checked = true;
			}
			(*it_e)->checked = true;
		}
	}
}
void moveOverlappedVertices(Model *mod)
{

}
void reconstructModel(Model *mod)
{
	// relocation facs
	relocationFaces(mod);
	// bridge edges
	bridgeEdges(mod);
	// move overlapped vertices
	moveOverlappedVertices(mod);
}