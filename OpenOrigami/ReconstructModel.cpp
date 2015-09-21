#include "ReconstructingModel.h"
#include <list>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace std;
using namespace Eigen;
const double scale = 1;

void calcOverlapOrder(Model *mod){
	int *overlapOrder;
	overlapOrder = new int[mod->faces.size()];

	for (int i = 0; i < mod->faces.size(); ++i){
		overlapOrder[i] = 0;
	}
	for (int i = 0; i < mod->faces.size(); ++i){
		for (int j = 0; j < i; ++j){
			if (mod->overlapRelation.coeff(i, j) == 1){
				overlapOrder[i]++;
			}
			else if (mod->overlapRelation.coeff(i, j) == 2){
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
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->itmp = overlapOrder[i];
	}

}
void calcOverlapOrder2(Model *mod){
	int *overlapOrder;
	overlapOrder = new int[mod->faces.size()];

	for (int i = 0; i < mod->faces.size(); ++i){
		overlapOrder[i] = 0;
	}

	for (int i = 0; i < mod->faces.size(); ++i){
		const int inf = 1024;
		int underMax = -inf;
		int upperMin = inf;
		for (int j = 0; j < i; ++j){
			switch (mod->overlapRelation.coeff(i, j)){
			case 1:
				// i < j
				upperMin = min(upperMin, overlapOrder[j]);
				break;
			case 2:
				// i > j
				underMax = max(underMax, overlapOrder[j]);
				break;
			default:
				break;
			}
		}
		if (underMax == -inf && upperMin == inf){
			overlapOrder[i] = 0;
		}
		if (underMax == -inf && upperMin != inf){
			overlapOrder[i] = upperMin - 1;
		}
		if (underMax != -inf && upperMin == inf){
			overlapOrder[i] = underMax + 1;
		}
		if (underMax != -inf && upperMin != inf){
			for (int j = 0; j < i; ++j){
				if (overlapOrder[j] > underMax+1){
					overlapOrder[j]++;
				}
			}
			overlapOrder[i] = underMax + 1;
		}
	}
	int mintmp = 0;
	for (int i = 0; i < mod->faces.size(); ++i){
		mintmp = min(mintmp, overlapOrder[i]);
	}
	if (mintmp < 0){
		for (int i = 0; i < mod->faces.size(); ++i){
			overlapOrder[i] -= mintmp;
		}
	}
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->itmp = overlapOrder[i];
	}
}
void relocationFaces(Model *mod)
{
	const double d = 2*scale;
	//重なり順を決定
	calcOverlapOrder(mod);
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
		mod->faceVector.at(i)->transPosition(d * mod->faceVector.at(i)->itmp * Vector3d(0, 0, 1));
	}

	//Debug
	
	//cout << mod->overlapRelation << endl;
	cout << "overlapOrder = "; for (int i = 0; i < mod->faces.size(); ++i){ cout << mod->faceVector.at(i)->itmp << ", "; }cout << endl;
	
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
	const double h = 1*scale;
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
	/*
	list<Vertex*> listtmp;
	listtmp.push_back(v1);
	listtmp.push_back(v2);
	listtmp.push_back(v3);
	listtmp.push_back(v4);
	listtmp.push_back(v5);
	listtmp.push_back(v6);
	listtmp.push_back(v7);
	listtmp.push_back(v8);
	for (list<Vertex*>::iterator it = listtmp.begin(); it != listtmp.end(); ++it){
		(*it)->transPosition(-h*vec_h.x(), -h*vec_h.y(), -h*vec_h.z());
	}
	*/
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
bool isParallel(Vector2d *vec1, Vector2d *vec2, Vector2d *vec3, Vector2d *vec4){
	Vector2d vecA, vecB;
	vecA = *vec2 - *vec1;
	vecB = *vec4 - *vec3;
	vecA.normalize();
	vecB.normalize();
	return (vecA - vecB).norm() < 0.00001 || (vecA + vecB).norm()< 0.00001;
}
bool isOnSegment(Vector2d *vec1, Vector2d *vec2, Vector2d *vec3){
	// 1-2, 3
	if ((*vec1 - *vec3).norm() < 0.00001)return true;

	Vector2d vecA, vecB, vecA_, vecB_;
	vecA_ = vecA = *vec2 - *vec1;
	vecB_ = vecB = *vec3 - *vec1;
	vecA.normalize();
	vecB.normalize();
	// isOn line
	return (vecA - vecB).norm() < 0.00001 && vecA_.norm() >= vecB_.norm();

}
bool isOverlapSegment(Vector2d *vec1, Vector2d *vec2, Vector2d *vec3, Vector2d *vec4){
	// 1-2, 3-4
	return isParallel(vec1, vec2, vec3, vec4) && (isOnSegment(vec1, vec2, vec3) || isOnSegment(vec1, vec2, vec4) || isOnSegment(vec3, vec4, vec1) || isOnSegment(vec3, vec4, vec2));
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
	sortedFaces.reverse();
	//Debug
	cout << "sortedFaces = \n";	for (list<Face*>::iterator it = sortedFaces.begin();it!= sortedFaces.end(); ++it){	cout << "id = " << (*it)->id << ", itmp = " << (*it)->itmp << endl;}

	vector<Face*> sortedFaceVecor(sortedFaces.begin(), sortedFaces.end());

	for (list<Face*>::iterator it_fi = sortedFaces.begin(); it_fi != sortedFaces.end(); ++it_fi){

		Face *f1 = (*it_fi);
		Halfedge *he_in_f = f1->halfedge;
		do{
			int maxHeight = 0;

			if (he_in_f->pair != NULL){

				Face *f2 = he_in_f->pair->face;
				if (mod->overlapRelation.coeff(f1->id, f2->id) == 2){
					for (list<Face*>::iterator it_fj = sortedFaces.begin(); it_fj != sortedFaces.end(); ++it_fj){
						Face *f3 = *it_fj;
						if (mod->overlapRelation.coeff(f3->id, f1->id)==1&&mod->overlapRelation.coeff(f3->id, f2->id)==2){
							// f3 on between f1 and f2
							Halfedge *he_in_f3 = f3->halfedge;
							do{
								if (he_in_f3->pair != NULL){
									Vector2d vec1, vec2, vec3, vec4;
									vec1 = Vector2d(he_in_f->vertex->x, he_in_f->vertex->y);
									vec2 = Vector2d(he_in_f->next->vertex->x, he_in_f->next->vertex->y);
									vec3 = Vector2d(he_in_f3->vertex->x, he_in_f3->vertex->y);
									vec4 = Vector2d(he_in_f3->next->vertex->x, he_in_f3->next->vertex->y);
									if (isOverlapSegment(&vec1, &vec2, &vec3, &vec4)){
										
										maxHeight = max(maxHeight, he_in_f3->itmp);
									}
								}
								he_in_f3 = he_in_f3->next;
							} while (he_in_f3 != f3->halfedge);
						}
					}
				}
				he_in_f->pair->itmp = maxHeight + 1;
			}

			he_in_f->itmp = maxHeight + 1;
			//cout << "he_in_f->itmp = " << he_in_f->itmp << endl;
			he_in_f = he_in_f->next;
		} while (he_in_f != f1->halfedge);

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