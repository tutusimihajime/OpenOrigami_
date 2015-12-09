#pragma once
#include "MyVector3d.h"
#include <Eigen/Geometry>
using namespace Eigen;
class Halfedge;

class Vertex
{
public:
	
	double x, y, z;
	int id;// 0 origin
	Halfedge *halfedge;

	bool checked;

	int itmp;//���_�Ĕz�u�Ŏg�p
	int itmp_face;//���_�Ĕz�u�Ŏg�p, �}�[�W�̍ۂ̗D��x�ɗp����
	int itmp4w;//OBJ�o�͂Ŏg�p
	int id_origin;//��ԍŏ��̏�Ԃł̒��_��ID, cpyVertex�Őݒ�, createVertex�ł�-1�ɏ�����
	Vector3d vtmp;//relocation bridge�Ŏg�p
	Vertex();
	~Vertex();

	Vertex(double _x, double _y, double _z);
	void setID(int n);
	void draw();
	inline void setPosition(double _x, double _y, double _z){ x = _x; y = _y; z = _z; }
	inline void setPosition(MyVector3d v){ setPosition(v.x, v.y, v.z); }
	inline void transPosition(double _x, double _y, double _z){ x += _x; y += _y; z += _z; }
	inline void transPosition(MyVector3d v){ transPosition(v.x, v.y, v.z); }
	inline void transPosition(Vector3d v){transPosition(v.x(), v.y(), v.z());}
	inline void scale(const double s) { x *= s; y *= s; z *= s; }
	void debugPrint();
};

