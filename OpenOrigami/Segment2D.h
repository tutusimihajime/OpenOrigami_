#pragma once
#include <iostream>
#include "Halfedge.h"
#include <Eigen/Geometry>
using namespace std;
using namespace Eigen;

class Segment2D{
public:
	Vector3d s;//Žn“_
	Vector3d v;//•ûŒü

	Segment2D(Vector2d *_s, Vector2d *_v);
	Segment2D(Halfedge *he);

	bool isParallel(Segment2D *seg);
	bool isOn(Vector3d *vec);
	bool isOverlapSegment(Segment2D *seg);
	bool isIncludingSegment(Segment2D *seg);
	void debugPrint();
};