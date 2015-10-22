#pragma once
#include "Halfedge.h"
#include "MyVector3d.h"
#include <Eigen/Dense>
using namespace Eigen;
class Face
{
public:
	Halfedge *halfedge;
	MyVector3d nv;
	int id;
	int itmp;//�ėptmp, �ʂ̍Ĕz�u�Ɏg�p
	int itmp2;//�T�u���p�`���k�Ɏg�p
	bool isDrawn;
	int type;//0:�ʏ�, 1:�Ԉ�����, 2:�u���b�W
	Vector3d *g;//�d�S
	Face(Halfedge *he);
	~Face();
	void normalizeNormal();
	void draw();
	void drawBack();
	void debugPrint();
	void transPosition(MyVector3d vec);
	void transPosition(double x, double y, double z);
	void transPosition(Vector3d vec);
	void setID(int _id){ id = _id; }
	void calcCenterOfGravity();
	void setZ(double z);
	void reverse();
};

