#pragma once
#include "SubFaceGroup.h"
#include <Eigen/Geometry>
using namespace Eigen;


bool isEqualVerticesOnXY(Vertex *v1, Vertex *v2){
	float cap = 0.01;
	return(fabs(v1->x - v2->x) + fabs(v1->y - v2->y)) < cap;
}
SubFaceGroup::SubFaceGroup(Face *_oldFace, list<Face*> _subfaces){
	oldFace = _oldFace;
	for (list < Face* > ::iterator it_f = _subfaces.begin(); it_f != _subfaces.end(); ++it_f){
		Face *sf = cpyFace(*it_f);
		sf->id = (*it_f)->id;
		sf->itmp = _oldFace->itmp;
		
		if (sf->nv.dot(oldFace->nv) < 0){
			sf->reverse();
		}
	}
	//set id_origin
	for (list<Vertex*>::iterator it_v = subvertices.begin(); it_v != subvertices.end(); ++it_v){
		Vertex *v = (*it_v);
		Halfedge *he = oldFace->halfedge;
		do{
			Vertex *v_origin = he->vertex;
			if (isEqualVerticesOnXY(v, v_origin)){
				v->id_origin = v_origin->id_origin;
			}
			he = he->next;
		} while (he!=oldFace->halfedge);
	}
}
void SubFaceGroup::initializeSubfacesZ(){

}
void SubFaceGroup::makeInnerPairing(){//merge後
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he1 = (*it_f)->halfedge;
		do{
			if (he1->pair == NULL){
				for (list<Face*>::iterator it_f2 = subfaces.begin(); it_f2 != subfaces.end(); ++it_f2){
					if (*it_f == *it_f2){
						continue;
					}
					Halfedge *he2 = (*it_f2)->halfedge;
					do{
						if (he2->pair == NULL){
							if (he1->vertex == he2->next->vertex && he1->next->vertex == he2->vertex){
								he1->setPair(he2);
								break;
							}
						}
						he2 = he2->next;
					} while (he2 != (*it_f2)->halfedge);
				}
			}
			he1 = he1->next;
		} while (he1 != (*it_f)->halfedge);
	}
}
void SubFaceGroup::calcSubFacesNormal(){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		(*it_f)->normalizeNormal();
	}
}
//cpy/create
Face *SubFaceGroup::cpyFace(Face *_f){
	Halfedge *he_in_f = _f->halfedge;
	vector<Halfedge*> hes;
	//create vertex and halfedge
	do{
		Vertex *v = createVertex(he_in_f->vertex->x, he_in_f->vertex->y, he_in_f->vertex->z);
		v->id = he_in_f->vertex->id;
		
		Halfedge *he = createHalfedge(v);
		hes.push_back(he);
		he_in_f = he_in_f->next;
	} while (he_in_f != _f->halfedge);
	//pairing edges by vector<Halfedge*>
	for (int i = 0; i < hes.size(); ++i){
		hes[i]->next = (i == hes.size() - 1) ? hes[0] : hes[i + 1];
		hes[i]->prev = (i == 0) ? hes[hes.size() - 1] : hes[i - 1];
	}
	//create Face
	Face *f = SubFaceGroup::createFace(hes[0]);
	return f;
}
Vertex *SubFaceGroup::createVertex(double x, double y, double z){
	Vertex *v = new Vertex(x, y, z);
	v->id_origin = -1;
	subvertices.push_back(v);
	return v;
}
Halfedge *SubFaceGroup::createHalfedge(Vertex *v){
	Halfedge *he = new Halfedge(v);
	return he;
}
Face *SubFaceGroup::createFace(Halfedge *he){
	Face *face;
	try{
		face = new Face(he);
	}catch (...){
		cout << "new 失敗\n";
	}
	subfaces.push_back(face);
	return face;
}
Vertex *SubFaceGroup::cpyVertex(Vertex *_v){
	Vertex *v = createVertex(_v->x, _v->y, _v->z);
	v->id_origin = _v->id_origin;
	return v;
}
float distanceVertices(Vertex *v1, Vertex *v2){
	Vector2f dv(v1->x - v2->x, v1->y - v2->y);
	return dv.norm();
}
//merge
void SubFaceGroup::mergeVertexPair(Vertex *v1, Vertex *v2){
	
	/*if (v2->z > v1->z){
		v1->z = v2->z;
	}*/
	v1->itmp = (v1->itmp > v2->itmp) ? v1->itmp : v2->itmp;
	v2->halfedge->vertex = v1;
	subvertices.remove(v2);
	delete v2;
}
void SubFaceGroup::mergeAllVertexPair(){
	const float cap = 0.001;
	// v->checked = false に初期化
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;
			v->checked = false;
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
	// ２重ループ走査でマージ
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;
			
			if (!v->checked){
				for (list<Face*>::iterator it_f2 = subfaces.begin(); it_f2 != subfaces.end(); ++it_f2){
					//同じサブフェースにはvとマージすべき頂点は存在しないので、continue
					if (*it_f == *it_f2){
						continue;
					}
					//
					Halfedge *he2 = (*it_f2)->halfedge;
					do{
						Vertex *v2 = he2->vertex;
						if (distanceVertices(v, v2) < cap){
							//マージ
							mergeVertexPair(v, v2);
						}
						he2 = he2->next;
					} while (he2 != (*it_f2)->halfedge);

				}
				v->checked = true;
			}

			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
//unify itmp
void SubFaceGroup::unifyVertexItmp(Vertex *v1, Vertex *v2){//高いほうに統一
	int higher_itmp = (v1->itmp > v2->itmp) ? v1->itmp : v2->itmp;
	v1->itmp = v2->itmp = higher_itmp;
}
void SubFaceGroup::unifyVertexItmp2(Vertex *v1, Vertex *v2){//itmp_faceを元に統一
	int better_itmp = (abs(v1->itmp_face-v1->itmp) < abs(v2->itmp_face-v2->itmp)) ? v1->itmp : v2->itmp;
	v1->itmp = v2->itmp = better_itmp;
}
void SubFaceGroup::unifyAllVertexItmp(){
	const float cap = 0.001;
	// v->checked = false に初期化
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;
			v->checked = false;
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
	// ２重ループ走査でマージ
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;

			if (!v->checked){
				for (list<Face*>::iterator it_f2 = subfaces.begin(); it_f2 != subfaces.end(); ++it_f2){
					//同じサブフェースにはvとマージすべき頂点は存在しないので、continue
					if (*it_f == *it_f2){
						continue;
					}
					//
					Halfedge *he2 = (*it_f2)->halfedge;
					do{
						Vertex *v2 = he2->vertex;
						if (distanceVertices(v, v2) < cap){
							//統一
							unifyVertexItmp(v, v2);
							v2->checked = true;
						}
						he2 = he2->next;
					} while (he2 != (*it_f2)->halfedge);

				}
				v->checked = true;
			}

			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
void SubFaceGroup::unifyAllVertexItmp2(){
	const float cap = 0.001;
	// v->checked = false に初期化
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;
			v->checked = false;
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
	// ２重ループ走査でマージ
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			Vertex *v = he->vertex;

			if (!v->checked){
				for (list<Face*>::iterator it_f2 = subfaces.begin(); it_f2 != subfaces.end(); ++it_f2){
					//同じサブフェースにはvとマージすべき頂点は存在しないので、continue
					if (*it_f == *it_f2){
						continue;
					}
					//
					Halfedge *he2 = (*it_f2)->halfedge;
					do{
						Vertex *v2 = he2->vertex;
						if (distanceVertices(v, v2) < cap){
							//統一
							unifyVertexItmp2(v, v2);
							v2->checked = true;
						}
						he2 = he2->next;
					} while (he2 != (*it_f2)->halfedge);

				}
				v->checked = true;
			}

			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
//draw
void SubFaceGroup::drawVertex(GLenum mode){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			he->vertex->draw();
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
void SubFaceGroup::drawEdge(GLenum mode){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		Halfedge *he = (*it_f)->halfedge;
		do{
			
			glLineWidth(1);
			glColor3f(.6, .6, .6);
			
			he->draw();
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
void SubFaceGroup::drawFace(GLenum mode, bool isEnableVisibleFlag){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		(*it_f)->draw(isEnableVisibleFlag);
	}
}
void SubFaceGroup::draw(bool isEnableVisibleFlag){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		if ((*it_f)->isDrawn||!isEnableVisibleFlag){
			Halfedge *he = (*it_f)->halfedge;
			do{
				//Vertex
				glDisable(GL_LIGHTING);
				glPointSize(5);
				glColor3f(.3, .3, .3);
				he->vertex->draw();
				//Halfedge
				glDisable(GL_LIGHTING);
				glEnable(GL_LINE_SMOOTH);
				glLineWidth(1);
				glColor3f(.6, .6, .6);
				he->draw();
				he = he->next;
			} while (he != (*it_f)->halfedge);
			//Face
			glEnable(GL_LIGHTING);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 30);
			GLfloat materialColor1[] = { 1, 0.2, 0.2, 1 };
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor1);
			GLfloat materialColor2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
			(*it_f)->draw(isEnableVisibleFlag);
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}
}
//debug
void SubFaceGroup::debugPrint(){
	cout << "Debug SubFaceGroup:\n";
	cout << "oldFace->id = " << oldFace->id;
	cout << ", subfaces.size() = " << subfaces.size() << endl;
	cout << "subfaces->vertex->id :\n";
	for (list < Face* >::iterator it = subfaces.begin(); it != subfaces.end(); ++it){
		Halfedge *he = (*it)->halfedge;
		do{
			printf("%d ", he->vertex);
			he = he->next;
		} while (he != (*it)->halfedge);
	}
}