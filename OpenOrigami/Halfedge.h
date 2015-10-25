#pragma once
#include "Vertex.h"

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
	int itmp;//�ėptmp
	double dtmp;//OuterPairng�Ŏg�p
	Halfedge(Vertex *v);
	~Halfedge();
	void draw();
	void debugPrint();
	void setID(int _id){ id = _id; }
	void setPair(Halfedge *he){ this->pair = he; he->pair = this; }
};

