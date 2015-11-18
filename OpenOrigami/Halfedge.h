#pragma once
#include "Vertex.h"
#include <Eigen/Geometry>
using namespace Eigen;
class Face;

class Halfedge
{
public:
	//������
	Vertex *vertex;
	Face *face;
	Halfedge *pair;
	Halfedge *next;
	Halfedge *prev;
	
	int id;
	bool checked;//�F�X�ȂƂ���ŁA�`�F�b�N�������ǂ����Ɏg��
	int itmp;//bridge����
	int itmpMax;//����MAX
	double dtmp;//OuterPairng�Ŏg�p
	Vector3d vtmp;//BridgeSFG�Ŏg�p
	Halfedge(Vertex *v);
	~Halfedge();
	void draw();
	void debugPrint();
	void setID(int _id){ id = _id; }
	void setPair(Halfedge *he){ this->pair = he; he->pair = this; }
	void move(double d, Vector3d vec){
		vec.normalize();
		this->vertex->transPosition(MyVector3d(d*vec.x(), d*vec.y(), d*vec.z()));
		this->next->vertex->transPosition(MyVector3d(d*vec.x(), d*vec.y(), d*vec.z()));
	}
	void move(Vector3d vec){
		this->vertex->transPosition(MyVector3d(vec.x(), vec.y(), vec.z()));
		this->next->vertex->transPosition(MyVector3d(vec.x(), vec.y(), vec.z()));

	}
};

