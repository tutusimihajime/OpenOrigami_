#pragma once
#include <cmath>

class MyVector3d
{
public:
	double x, y, z;
	MyVector3d() { x = y = z = 0; }
	MyVector3d(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }
	void set(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }

	// 長さを1に正規化する
	void normalize() {
		double len = length();
		x /= len; y /= len; z /= len;
	}

	// 長さを返す
	double length() { return sqrt(x * x + y * y + z * z); }

	// s倍する
	void scale(const double s) { x *= s; y *= s; z *= s; }

	//対称のベクトルvとの内積を返す
	double dot(MyVector3d v){ return (x*v.x + y*v.y + z*v.z); }
	
	// 演算子のオーバーライド
	inline MyVector3d& operator+=(const MyVector3d& v) { x += v.x; y += v.y; z += v.z; return(*this); }
	inline MyVector3d& operator-=(const MyVector3d& v) { x -= v.x; y -= v.y; z -= v.z; return(*this); }
	inline MyVector3d operator+(const MyVector3d& v){ return(MyVector3d(x + v.x, y + v.y, z + v.z)); }
	inline MyVector3d operator-(const MyVector3d& v){ return(MyVector3d(x - v.x, y - v.y, z - v.z)); }
	inline MyVector3d operator*(const MyVector3d& v){ return(MyVector3d(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x)); }//（aybz-azby, azbx-axbz, axby-aybx）

};

