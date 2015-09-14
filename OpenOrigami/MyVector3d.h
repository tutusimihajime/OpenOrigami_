#pragma once
#include <cmath>

class MyVector3d
{
public:
	double x, y, z;
	MyVector3d() { x = y = z = 0; }
	MyVector3d(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }
	void set(double _x, double _y, double _z) { x = _x; y = _y; z = _z; }

	// ������1�ɐ��K������
	void normalize() {
		double len = length();
		x /= len; y /= len; z /= len;
	}

	// ������Ԃ�
	double length() { return sqrt(x * x + y * y + z * z); }

	// s�{����
	void scale(const double s) { x *= s; y *= s; z *= s; }

	//�Ώ̂̃x�N�g��v�Ƃ̓��ς�Ԃ�
	double dot(MyVector3d v){ return (x*v.x + y*v.y + z*v.z); }
	
	// ���Z�q�̃I�[�o�[���C�h
	inline MyVector3d& operator+=(const MyVector3d& v) { x += v.x; y += v.y; z += v.z; return(*this); }
	inline MyVector3d& operator-=(const MyVector3d& v) { x -= v.x; y -= v.y; z -= v.z; return(*this); }
	inline MyVector3d operator+(const MyVector3d& v){ return(MyVector3d(x + v.x, y + v.y, z + v.z)); }
	inline MyVector3d operator-(const MyVector3d& v){ return(MyVector3d(x - v.x, y - v.y, z - v.z)); }
	inline MyVector3d operator*(const MyVector3d& v){ return(MyVector3d(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x)); }//�iaybz-azby, azbx-axbz, axby-aybx�j

};

