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
	int itmp;//汎用tmp, 面の再配置に使用
	int itmp2;//サブ多角形圧縮に使用
	bool isDrawn;
	int type;//0:通常, 1:間引かれ, 2:ブリッジ
	Vector3d *g;//重心
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

