#pragma once
#include "ReconstructingModel.h"
#include "SubFaceGroup.h"
#include <list>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include "Segment2D.h"
#include "GeometryElement2EigenVector.h"
using namespace std;
using namespace Eigen;
double scale = 1;//0.5
double d;

bool compFaceItmp(Face *f1, Face *f2){
	return f1->itmp < f2->itmp;
}
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
//gomi
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
//KIKUCHI THE GOD
void calcOverlapOrder3(Model *mod, double *a = NULL){
	// init q
	list<int> q;
	for (int i = 0; i < mod->faces.size(); ++i){
		q.push_back(i);
	}
	list<list<int>> lists;

	while (!q.empty()){ 
		// init l
		list<int> l;
		l.push_back(q.front());
		q.pop_front();
		int tmpQ = q.size();
		for (int i = 0; i < tmpQ; ++i){
			int tmpL = l.size();
			int lookAt = q.front();
			q.pop_front();
			int before = 2;
			for (list<int>::iterator it = l.begin(); it != l.end(); ++it){
				int now = mod->overlapRelation.coeff(lookAt, *it);
				if (now == 1){
					if (before == 2){
						//入れる
						l.insert(it, lookAt);
					}
					before = now;
					break;
				}
				before = now;
			}
			if (before == 2){
				l.push_back(lookAt);
			}
			if (tmpL == l.size()){
				q.push_back(lookAt);
			}
		}

		lists.push_back(l);
	}
	//Debug
	//cout << "lists = ";	for (list<list<int>>::iterator it = lists.begin(); it != lists.end(); ++it){cout << "{ ";for (list<int>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2){cout << *it2 << " ";}cout << " }, ";}cout << endl;

	list<int> mergeList = lists.front();
	lists.pop_front();
	while (!lists.empty()){
		list<int> targetList = lists.front();
		lists.pop_front();
		while (!targetList.empty()){
			int target = targetList.front();
			targetList.pop_front();
			int before = 2;
			int cntZero = 0;
			for (list<int>::iterator it = mergeList.begin(); it != mergeList.end(); ++it){
				int now = mod->overlapRelation.coeff(target, *it);
				if (now == 0){
					cntZero++;
				}
				if (now == 1){
					mergeList.insert(it, target);
					before = now;
					break;
				}
				if (now == 2){
					cntZero = 0;
				}
				before = now;
			}
			if (before != 1){
				list<int>::iterator it = mergeList.end();
				for (int i = 0; i < cntZero; ++i){
					it--;
				}
				mergeList.insert(it, target);
			}
		}
	}

	//Debug
	//cout << "{ ";for (list<int>::iterator it2 = mergeList.begin(); it2 != mergeList.end(); ++it2){cout << *it2 << " "; }cout << " }"<<endl;

	int i = 0;
	for (list<int>::iterator it = mergeList.begin(); it != mergeList.end(); ++it){
		mod->faceVector.at(*it)->itmp = i++;
	}

	//
	
	list<Face*> sortedFaces(mod->faces.begin(), mod->faces.end());
	sortedFaces.sort(compFaceItmp);
	for (list<Face*>::iterator it = sortedFaces.begin(); it != sortedFaces.end(); ++it){
		(*it)->itmp = 0;
	}
	//cout << "sortedFaces = \n";	for (list<Face*>::iterator it = sortedFaces.begin(); it != sortedFaces.end(); ++it){ cout << "id = " << (*it)->id << ", itmp = " << (*it)->itmp << endl; }

	for (list<Face*>::iterator iti = sortedFaces.begin(); iti != sortedFaces.end(); ++iti){
		Face *nearestLowwer = NULL;
		Face *firstZero = NULL;
		for (list < Face* >::iterator itj = sortedFaces.begin(); itj != iti; ++itj){
			if (mod->overlapRelation.coeff((*iti)->id, (*itj)->id) == 0){
				if (firstZero == NULL && nearestLowwer == NULL){
					firstZero = *itj;
				}
			}
			if (mod->overlapRelation.coeff((*iti)->id, (*itj)->id) == 2){
				if (nearestLowwer == NULL){
					nearestLowwer = *itj;
				}else{
					nearestLowwer = (nearestLowwer->itmp > (*itj)->itmp) ? nearestLowwer : (*itj);
				}
				firstZero = NULL;
			}
		}
		if (firstZero != NULL){
			(*iti)->itmp = firstZero->itmp;
		}else if (nearestLowwer != NULL){
			(*iti)->itmp = nearestLowwer->itmp + 1;
		}
	}
	int iMax = 0;
	for (list<Face*>::iterator it = sortedFaces.begin(); it != sortedFaces.end(); ++it){
		iMax = max(iMax, (*it)->itmp);
	}

	if (a != NULL){
		*a = iMax;
	}
	//scale = scale / (double)iMax;
	//cout << scale << endl;
	//Debug
	//cout << "sortedFaces = \n";for (list<Face*>::iterator it = sortedFaces.begin(); it != sortedFaces.end(); ++it){cout << "id = " << (*it)->id << ", itmp = " << (*it)->itmp<< endl;}
	

}

void relocationFaces(Model *mod)
{
	//double d = 2 * scale;
	double k1 = scale, k2 = 0.8, a;
	//cout << d << endl;
	//重なり順を決定
	calcOverlapOrder3(mod, &a);

	d = k1 * powf(a, k2-1);
	cout << "d = " << d << endl;
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
			}
			else{
				he_in_f->vertex = mod->cpyVertex(he_in_f->vertex);
				he_in_f->vertex->halfedge = he_in_f;
			}
			he_in_f = he_in_f->next;
		} while (he_in_f != f->halfedge);
	}

	//面を移動
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->transPosition(d * mod->faceVector.at(i)->itmp * Vector3d(0, 0, 1));
	}

	//cout << mod->overlapRelation << endl;
	//cout << "itmp = "; for (int i = 0; i < mod->faces.size(); ++i){ cout << mod->faceVector.at(i)->itmp << ", "; }cout << endl;
	
}

void createBridge(Model *mod, Halfedge *he){
	
	//create vertex
	double h = 0.5 * d;
	double w = 0.6 * h;
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
		f[i]->type = 2;
		++i;
	}
	
	//make pairing 10/6 バグ・・・直った
	Halfedge *he_p = he->pair;
	he->setPair(f[0]->halfedge->prev);
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
	//sortedFaces.reverse();
	//Debug
	//cout << "sortedFaces = \n";	for (list<Face*>::iterator it = sortedFaces.begin();it!= sortedFaces.end(); ++it){	cout << "id = " << (*it)->id << ", itmp = " << (*it)->itmp << endl;}

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
	//cout << "halfedges.itmp = ";for (list<Halfedge*>::iterator it_e = mod->halfedges.begin(); it_e != mod->halfedges.end(); ++it_e){cout << (*it_e)->itmp << ", ";	}cout << endl;
	
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
void setIsDraw(Model *mod){
	// まず、すべてを間引くことにする
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->type = 0;
	}
	// 間引かないものを選ぶ
	for (int i = 0; i < mod->subfaceVector.size(); ++i){
		Face *fTop = NULL, *fBot = NULL;
		for (int j = 0; j < mod->faceVector.size(); ++j){
			if (mod->subfaceOverlapFace[i][j]){
				if (fTop == NULL){
					fTop = mod->faceVector.at(j);
				}else if (fTop->itmp < mod->faceVector.at(j)->itmp){
					fTop = mod->faceVector.at(j);
				}
				if (fBot == NULL){
					fBot = mod->faceVector.at(j);
				}else if(fBot->itmp > mod->faceVector.at(j)->itmp){
					fBot = mod->faceVector.at(j);
				}
			}
		}
		fTop->isDrawn = fBot->isDrawn = true;
		fTop->type = fBot->type = 0;
	}

	//test
	/*cout << "isDrawn\n";
	for (int i = 0; i < mod->faceVector.size(); ++i){
		cout << mod->faceVector.at(i)->isDrawn << " ";
	}*/
}
void calculateEdgeRelocationVector4Bridge(Halfedge *he){
	Vector3d v = createVector3d(he), vprev = createVector3d(he->prev);
	v.z() = vprev.z() = 0;
	v.normalize();
	vprev.normalize();
	Vector3d n = v.cross(-vprev);

	//Vector3d vhe = createVector3d(he);
	he->vtmp = n.cross(v);
	he->vtmp.normalize();
	double distance = he->itmpMax*0.5*d + 0.1*d;
	Vector3d vh = he->vtmp*distance;
	Vector3d vnext2 = createVector3d(he->next), vprev2 = createVector3d(he->prev);
	vnext2.z() = vprev2.z() = 0;

	//重複移動チェック
	const double cap = 0.01;
	Vector3d v0, v1, v2, v3;
	v0 = he->vertex->vtmp;
	v1 = vprev2;
	if (v0.norm() < cap){//初期値か、ほとんど動いてないなら
		he->vertex->vtmp = vh.dot(vh) / vh.dot(vprev2)*vprev2;
	}else{
		//要修正
		/*
		if (1 - fabs(v0.dot(v1)) > cap){
			he->vertex->vtmp += vh.dot(vh) / vh.dot(vprev2)*vprev2;
			//cout << v0.dot(v1) << endl;
		}
		*/
		v0.normalize(); v1.normalize();
		Vector3d vtmp2 = vh.dot(vh) / vh.dot(vprev2)*vprev2;
		if (fabs(v0.dot(vh))<cap){
			//違う方向->同一フェース内で２回のとき
			he->vertex->vtmp += vtmp2;
		}else{
			//同じ方向への重複移動->でかいほうを選択
			he->vertex->vtmp = (he->vertex->vtmp.norm() > vtmp2.norm()) ? he->vertex->vtmp : vtmp2;
		}
	}

	v2 = he->next->vertex->vtmp;
	v3 = vnext2;
	if (v2.norm() < cap){//初期値か、ほとんど動いてないなら
		he->next->vertex->vtmp = vh.dot(vh) / vh.dot(vnext2)*vnext2;
	}
	else{
		v2.normalize(); v3.normalize();
		Vector3d vtmp2 = vh.dot(vh) / vh.dot(vnext2)*vnext2;
		if (fabs(v2.dot(vh)) < cap){
			//違う方向->同一フェース内で２回のとき
			he->next->vertex->vtmp += vtmp2;
		}
		else{
			//同じ方向への重複移動->でかいほうを選択
			he->next->vertex->vtmp = (he->next->vertex->vtmp.norm() > vtmp2.norm()) ? he->next->vertex->vtmp : vtmp2;
		}
	}
	/*
	if (v2.norm() < cap){
		he->next->vertex->vtmp = vh.dot(vh) / vh.dot(vnext2)*vnext2;
	}else{
		v2.normalize(); v3.normalize();
		if (1 - fabs(v2.dot(v3)) > cap || v0.norm() < cap){
			he->next->vertex->vtmp += vh.dot(vh) / vh.dot(vnext2)*vnext2;
		}
	}*/
/*
	cout << "vtmp = \n" << he->vtmp << endl;
	cout << "vh = \n" << vh << endl;
	cout << "v_prev = \n" << vprev << endl;
	cout << "vd = \n" << (vh.dot(vh) / vh.dot(vprev)*vprev) << endl;*/
}
void relocationVerticesForBridge(Model *mod){

	//init
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Vertex*>::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			(*it_v)->vtmp = Vector3d(0, 0, 0);
		}
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{
				//cout << "(itmp, itmpMax) = (" <<he_in_f->itmp<<", "<< he_in_f->itmpMax << ")\n";
				he_in_f->checked = false;
				he_in_f->vtmp = Vector3d();
				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
	//calculate relocating direction every edges to bridge
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{

				if (!he_in_f->checked){
					/*
					if (he_in_f->pair != NULL){
						if (he_in_f->next->vertex != he_in_f->pair->vertex){
							//bridge edges
							calculateEdgeRelocationVector4Bridge(he_in_f);
							calculateEdgeRelocationVector4Bridge(he_in_f->pair);
						}
						he_in_f->pair->checked = true;
						
					}*/
					if (he_in_f->bridge != NULL){
						Bridge *bridge = he_in_f->bridge;
						calculateEdgeRelocationVector4Bridge(bridge->he1);
						calculateEdgeRelocationVector4Bridge(bridge->he2);
						bridge->he2->checked = true;
					}
				}
				he_in_f->checked = true;

				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
	//relocation VERTICES
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Vertex*>::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			(*it_v)->transPosition((*it_v)->vtmp);
		}
	}
}
void bridgeSFG(Model *mod){
	
	//initialize he->itmp = 0
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{
				he_in_f->itmp = 0;
				he_in_f->itmpMax = 0;
				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
	//リストリスト作成 ある１つのIDのリストはサイズが0なので、除く
	list<list<Face*> >subfaceListList;
	for (int i = 0; i < mod->subfaceVector.size() + 1; ++i){

		vector<Face*> id_subfaceVector;
		list<Face*> id_subfaceList;
		for (list<SubFaceGroup*>::iterator it = mod->subFaceGroups.begin(); it != mod->subFaceGroups.end(); ++it){
			for (list<Face*>::iterator it_f = (*it)->subfaces.begin(); it_f != (*it)->subfaces.end(); ++it_f){
				if (i == (*it_f)->id){
					id_subfaceList.push_back(*it_f);
				}
			}
		}
		id_subfaceList.sort(compFaceItmp);
		if (id_subfaceList.size() != 0){
			subfaceListList.push_back(id_subfaceList);
		}
	}
	//calculate he->itmp
	for (list<list<Face*>>::iterator it = subfaceListList.begin(); it != subfaceListList.end(); ++it){
		list<Face*> subfaceList = *it;
		for (list<Face*>::iterator it_fi = subfaceList.begin(); it_fi != subfaceList.end(); ++it_fi){
			Face *f1 = (*it_fi);
			Halfedge *he_in_f = f1->halfedge;
			do{
				int maxHeight = 0;
				if (he_in_f->pair != NULL){

					Face *f2 = he_in_f->pair->face;
					//if (f1->itmp>f2->itmp){
					if (he_in_f->next->vertex->z>he_in_f->pair->vertex->z){
						for (list<Face*>::iterator it_fj = subfaceList.begin(); it_fj != subfaceList.end(); ++it_fj){
							Face *f3 = *it_fj;
							if (f3->itmp<f1->itmp&&f3->itmp>f2->itmp){
								//cout << "(f1, f2, f3) = (" << f1->itmp << ", " << f2->itmp << ", " << f3->itmp << ")\n";
								// f3 on between f1 and f2
								Halfedge *he_in_f3 = f3->halfedge;
								do{
									if (he_in_f3->pair != NULL){
										if (he_in_f3->vertex!=he_in_f3->next->vertex){
											Vector2d vec1, vec2, vec3, vec4;
											vec1 = Vector2d(he_in_f->vertex->x, he_in_f->vertex->y);
											vec2 = Vector2d(he_in_f->next->vertex->x, he_in_f->next->vertex->y);
											vec3 = Vector2d(he_in_f3->vertex->x, he_in_f3->vertex->y);
											vec4 = Vector2d(he_in_f3->next->vertex->x, he_in_f3->next->vertex->y);
											if (isOverlapSegment(&vec1, &vec2, &vec3, &vec4)){
												maxHeight = max(maxHeight, he_in_f3->itmp);
											}
										}
									}
									he_in_f3 = he_in_f3->next;
								} while (he_in_f3 != f3->halfedge);
							}
						}
						he_in_f->pair->itmp = he_in_f->pair->itmpMax = maxHeight + 1;
						he_in_f->itmp = he_in_f->itmpMax = maxHeight + 1;

					}
					
				}
				

				//itmpMax
				
				if (he_in_f->pair != NULL){
					Face *f2 = he_in_f->pair->face;
					//if (f1->itmp>f2->itmp){
					if (he_in_f->next->vertex->z>he_in_f->pair->vertex->z){
						for (list<Face*>::iterator it_fj = subfaceList.begin(); it_fj != subfaceList.end(); ++it_fj){
							Face *f3 = *it_fj;
							if (f3->itmp<f1->itmp&&f3->itmp>f2->itmp){
								// f3 on between f1 and f2
								Halfedge *he_in_f3 = f3->halfedge;
								do{
									if (he_in_f3->pair != NULL){
										if (he_in_f3->vertex != he_in_f3->next->vertex){
											Vector2d vec1, vec2, vec3, vec4;
											vec1 = Vector2d(he_in_f->vertex->x, he_in_f->vertex->y);
											vec2 = Vector2d(he_in_f->next->vertex->x, he_in_f->next->vertex->y);
											vec3 = Vector2d(he_in_f3->vertex->x, he_in_f3->vertex->y);
											vec4 = Vector2d(he_in_f3->next->vertex->x, he_in_f3->next->vertex->y);
											if (isOverlapSegment(&vec1, &vec2, &vec3, &vec4)){
												he_in_f3->itmpMax = he_in_f3->pair->itmpMax = he_in_f->itmp;//max(he_in_f3->pair->itmpMax,max(he_in_f->itmp, he_in_f3->itmpMax));
											}
										}
									}
									he_in_f3 = he_in_f3->next;
								} while (he_in_f3 != f3->halfedge);
							}
						}
					}
				}

				//cout << "he_in_f->itmp = " << he_in_f->itmp << endl;
				he_in_f = he_in_f->next;
			} while (he_in_f != f1->halfedge);
		}
	}
	
	
	//init
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{
				he_in_f->checked = false;
				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
	//create bridge 
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{

				if (!he_in_f->checked){
					if (he_in_f->pair != NULL){
						if (he_in_f->next->vertex != he_in_f->pair->vertex){
							Bridge *b = new Bridge(he_in_f, he_in_f->pair);
							mod->bridges.push_back(b);
						}
						he_in_f->pair->checked = true;
					}
				}
				he_in_f->checked = true;

				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
	

}
float calculateAverageZ(Face *f){
	float m = 0;
	int i = 0;
	Halfedge *he = f->halfedge;
	do{
		m += he->vertex->z;
		++i;
		he = he->next;
	} while (he != f->halfedge);
	return m / (float)i;
}
float ListVertexMaxZ(list<Vertex*> listv){
	float Max = 0;
	for (list < Vertex* >::iterator it_v = listv.begin(); it_v != listv.end(); ++it_v){
		Max = max((float)(*it_v)->z, Max);
	}
	return Max;
}
bool compListPairListVertexFloat(pair<list<Vertex*>, FLOAT> plf1, pair<list<Vertex*>, FLOAT> plf2){
	return ListVertexMaxZ(plf1.first) < ListVertexMaxZ(plf2.first);
}
void resetID(Model *mod){
	// subvertices id 振りなおし
	int id = 0;
	for (list<SubFaceGroup* >::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list < Vertex* >::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			(*it_v)->id = id;
			++id;
		}
	}
	for (list < Bridge* >::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		for (list < Vertex* >::iterator it_v = (*it_b)->vertices.begin(); it_v != (*it_b)->vertices.end(); ++it_v){
			(*it_v)->id = id;
			++id;
		}
	}
}
void compressionBridge(Model *mod){

	//Caluculate m_z
	for (list<Bridge*>::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		(*it_b)->calculateM_Z();
	}

	//Initialize Vertex check flag
	for (list<SubFaceGroup* >::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Vertex*>::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			(*it_v)->checked = false;
		}
	}
	list<pair<list<Vertex*>, FLOAT> >listCompressionVertices;
	//
	for (list<Bridge*>::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		if ((*it_b)->he1->itmp == (*it_b)->he1->itmpMax){
			//最大ブリッジについて
			float z1 = calculateAverageZ((*it_b)->he1->face), z2 = calculateAverageZ((*it_b)->he2->face);
			
			Face *topFace = (z1 > z2) ? (*it_b)->he1->face : (*it_b)->he2->face;//zの大きい方のフェース
			Face *botFace = (z1 < z2) ? (*it_b)->he1->face : (*it_b)->he2->face;

			
			Segment2D seg0((*it_b)->he1);
			list<Vertex*> compressionVertices;//圧縮対象の頂点
			compressionVertices.push_back((*it_b)->he1->vertex);
			compressionVertices.push_back((*it_b)->he1->next->vertex);
			compressionVertices.push_back((*it_b)->he2->vertex);
			compressionVertices.push_back((*it_b)->he2->next->vertex);
			
			if (compressionVertices.size()!=0){
				for (list<Vertex*>::iterator it_v = (*it_b)->vertices.begin(); it_v != (*it_b)->vertices.end(); ++it_v){
					compressionVertices.push_back(*it_v);
				}
				// Initialize Halfedge check flag
				for (list<SubFaceGroup* >::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
					for (list < Face* >::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
						Halfedge *he = (*it_f)->halfedge;
						do{
							he->checked = false;
							he = he->next;
						} while (he != (*it_f)->halfedge);
					}
					//Initialize Vertex check flag
					for (list<Vertex*>::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
						(*it_v)->checked = false;
					}

				}


				//重なっている面を取得
				for (list<SubFaceGroup* >::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){

					for (list < Face* >::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){

						if ((topFace->id == (*it_f)->id) && (*it_f)->itmp<topFace->itmp && (*it_f)->itmp>botFace->itmp){
							//重なっている辺を取得
							Halfedge *he = (*it_f)->halfedge;
							do{
								if (!he->checked){
									Segment2D seg1(he);
									if (seg0.isOverlapSegment(&seg1)){
										if (he->bridge != NULL){
											compressionVertices.push_back(he->bridge->he1->vertex);
											compressionVertices.push_back(he->bridge->he1->next->vertex);
											compressionVertices.push_back(he->bridge->he2->vertex);
											compressionVertices.push_back(he->bridge->he2->next->vertex);
											he->bridge->he1->checked = he->bridge->he2->checked = true;

										}
										else{

											compressionVertices.push_back(he->vertex);
											compressionVertices.push_back(he->next->vertex);
											he->checked = true;
										}
									}
									he->checked = true;
								}
								he = he->next;
							} while (he != (*it_f)->halfedge);

						}
					}
				}
				listCompressionVertices.push_back(pair<list<Vertex*>, FLOAT>(compressionVertices, (*it_b)->m_z));
			}
		}
	}
	
	//圧縮(ブリッジ内の頂点はあとでreCalcする)
	const float alpha = 0.3;
	listCompressionVertices.sort(compListPairListVertexFloat);
	listCompressionVertices.reverse();
	for (list<pair<list<Vertex*>, FLOAT >>::iterator it_lv = listCompressionVertices.begin(); it_lv != listCompressionVertices.end(); ++it_lv){
		list<Vertex*>compressionVertices = (*it_lv).first;
		float m_z = (*it_lv).second;
		for (list<Vertex*>::iterator it_v = compressionVertices.begin(); it_v != compressionVertices.end(); ++it_v){
			if (!(*it_v)->checked){
				(*it_v)->z = alpha*((*it_v)->z - m_z) + m_z;
				
				//(*it_v)->checked = true;
			}
		}
	}
}
void compressionBridge2(Model *mod){
	//---test---//
	for (list < SubFaceGroup* >::iterator it = mod->subFaceGroups.begin(); it != mod->subFaceGroups.end(); ++it){
		for (list<Vertex*>::iterator it_v = (*it)->subvertices.begin(); it_v != (*it)->subvertices.end(); ++it_v){
		//cout << "id_origin = " << (*it_v)->id_origin << endl;
		}
	}
	//---END---test---END---//
	
	//頂点圧縮リストリストを宣言
	list<list<Vertex*>> listListCompressionVertices;

	//---サブフェース分割前からあった頂点について---//

	//id_originのMaxを求める
	int id_origin_Max = 0;
	for (list < SubFaceGroup* >::iterator it = mod->subFaceGroups.begin(); it != mod->subFaceGroups.end(); ++it){
		for (list<Vertex*>::iterator it_v = (*it)->subvertices.begin(); it_v != (*it)->subvertices.end(); ++it_v){
			id_origin_Max = max(id_origin_Max, (*it_v)->id_origin);
		}
	}
	//id_originごとの頂点のリストを作成し、頂点圧縮リストリストに追加
	for (int id = 0; id <= id_origin_Max; ++id){
		list < Vertex* > listCompressionVertices;
		for (list < SubFaceGroup* >::iterator it = mod->subFaceGroups.begin(); it != mod->subFaceGroups.end(); ++it){
			for (list<Vertex*>::iterator it_v = (*it)->subvertices.begin(); it_v != (*it)->subvertices.end(); ++it_v){
				Vertex *v = (*it_v);
				if (v->id_origin == id){
					//cout << v->id_origin << endl;

					listCompressionVertices.push_back(v);
				}
			}
		}
		if (listCompressionVertices.size() != 0){
			listListCompressionVertices.push_back(listCompressionVertices);
		}
	}

	//---END---サブフェース分割前からあった頂点について---END---//
	
	//---圧縮---//
	const float alpha = 0;
	for (list<list<Vertex*>>::iterator it_lv = listListCompressionVertices.begin(); it_lv != listListCompressionVertices.end(); ++it_lv){
		list<Vertex*> listCompressionVertices = (*it_lv);
		float z_average = 0;
		float z_Max=0, z_min=1000;
		for (list < Vertex* >::iterator it_v = listCompressionVertices.begin(); it_v != listCompressionVertices.end(); ++it_v){
			z_average += (*it_v)->z;
			z_Max = max(z_Max, (float)(*it_v)->z);
			z_min = min(z_min, (float)(*it_v)->z);
		}
		z_average /= (float)listCompressionVertices.size();
		float z_mid = 0.5*(z_Max + z_min);
		
		for (list < Vertex* >::iterator it_v = listCompressionVertices.begin(); it_v != listCompressionVertices.end(); ++it_v){
			Vertex *v = *it_v;
			//cout << v->z<<", "<<z_Max<<", "<<z_mid<<", "<<z_min ;
			v->z = alpha*((*it_v)->z - z_mid) + z_mid;
			//cout << ", " << v->z << endl;
		}
	}
	//---END---圧縮---END---//

	//---サブフェース分割で生まれた頂点について---//
	
	//---END---サブフェース分割で生まれた頂点について---END---//

}
void compressionBridge3(Model *mod){
	for (list<Bridge*>::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		Bridge *bridge = *it_b;
		if (bridge->he1->itmp == bridge->he1->itmpMax){

		}

	}
}
void invisiblizeFaces(Model *mod){
	for (list<SubFaceGroup*>::iterator it_sfg = mod->subFaceGroups.begin(); it_sfg != mod->subFaceGroups.end(); ++it_sfg){
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			Halfedge *he_in_f = (*it_f)->halfedge;
			do{
				//すべてのheを巡回
				if (he_in_f->bridge!=NULL){
					if (he_in_f->itmp != he_in_f->itmpMax){
						he_in_f->bridge->setInvisible();
						he_in_f->face->setInvisible();
					}else{

					}
				}

				he_in_f = he_in_f->next;
			} while (he_in_f != (*it_f)->halfedge);
		}
	}
}
void relocationSubFaceGroupVertices(Model *mod){
	// subdivison Model
	mod->constructSubFaceGroup();//10/21 14:23抜けない -> 10/21 15:53 hesにpushしてなかった...
	//mod->debugPrintSFGs();
	cout << "create bridge sub-model\n";
	bridgeSFG(mod);
	cout << "invisiblizeFaces\n";
	invisiblizeFaces(mod);
	cout << "reset ID\n";
	resetID(mod);
	cout << "compressionBridge\n";
	//compressionBridge2(mod);
	cout << "relocationVerticesForBridge\n";
	relocationVerticesForBridge(mod);
	cout << "Re-Calculate Bridge\n";
	for (list<Bridge*>::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		(*it_b)->reCalc();
	}
	cout << "normalize Normal\n";
	for (list<Bridge*>::iterator it_b = mod->bridges.begin(); it_b != mod->bridges.end(); ++it_b){
		(*it_b)->normalizeFaces();
	}
}
void reconstructModel(Model *mod)
{
	// relocation facs
	cout << "relocationFaces\n";
	relocationFaces(mod);
	// SFG
	cout << "relocationFaceGroupVertices\n";
	relocationSubFaceGroupVertices(mod);
	
	
	// set isDraw
	cout << "setIsDraw\n";
	//setIsDraw(mod);

	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->type = 0;
	}
	// bridge edges
	cout << "bridgeEdges\n";
	bridgeEdges(mod);
	// move overlapped vertices
	moveOverlappedVertices(mod);
	cout << "reconstruct end\n";
}