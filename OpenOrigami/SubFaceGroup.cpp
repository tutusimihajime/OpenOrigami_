#pragma once
#include "SubFaceGroup.h"
#include <Eigen/Geometry>
using namespace Eigen;

SubFaceGroup::SubFaceGroup(Face *_oldFace, list<Face*> _subfaces){
	oldFace = _oldFace;
	for (list < Face* > ::iterator it_f = _subfaces.begin(); it_f != _subfaces.end(); ++it_f){
		Face *sf = cpyFace(*it_f);
		subfaces.push_back(sf);
	}

}
Face *SubFaceGroup::cpyFace(Face *_f){
	Halfedge *he_in_f = _f->halfedge;
	vector<Halfedge*> hes;
	//create vertex and halfedge
	do{
		Vertex *v = createVertex(he_in_f->vertex->x, he_in_f->vertex->y, he_in_f->vertex->z);
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
	return face;
}
Vertex *SubFaceGroup::cpyVertex(Vertex *_v){
	return (new Vertex(_v->x, _v->y, _v->z));
}
float distanceVertices(Vertex *v1, Vertex *v2){
	Vector2f dv(v1->x - v2->x, v1->y - v2->y);
	return dv.norm();
}

void SubFaceGroup::mergeVertexPair(Vertex *v1, Vertex *v2){
	v2->halfedge->vertex = v1;
	delete v2;
}
void SubFaceGroup::mergeAllVertexPair(){
	const float cap = 0.00001;
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
					//同じ面にはvとマージすべき頂点は存在しないので、continue
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

void SubFaceGroup::draw(GLenum mode){
	for (list<Face*>::iterator it_f = subfaces.begin(); it_f != subfaces.end(); ++it_f){
		(*it_f)->draw();
	}
}
void SubFaceGroup::debugPrint(){
	cout << "Debug SubFaceGroup:\n";
	cout << "oldFace->id = " << oldFace->id << endl;
	cout << "subfaces.size() = " << subfaces.size() << endl;
}