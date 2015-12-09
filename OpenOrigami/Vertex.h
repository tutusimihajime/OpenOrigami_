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

	int itmp;//頂点再配置で使用
	int itmp_face;//頂点再配置で使用, マージの際の優先度に用いる
	int itmp4w;//OBJ出力で使用
	int id_origin;//一番最初の状態での頂点のID, cpyVertexで設定, createVertexでは-1に初期化
	Vector3d vtmp;//relocation bridgeで使用
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

