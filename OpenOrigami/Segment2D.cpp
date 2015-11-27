#pragma once
#include "Segment2D.h"

Segment2D::Segment2D(Vector2d *_s, Vector2d *_v){
	s = Vector3d(_s->x(), _s->y(), 0);
	v = Vector3d(_v->x(), _v->y(), 0);
}
Segment2D::Segment2D(Halfedge *he){
	s = Vector3d(he->vertex->x, he->vertex->y, 0);
	v = Vector3d(he->next->vertex->x - he->vertex->x, he->next->vertex->y - he->vertex->y, 0);
}

bool Segment2D::isParallel(Segment2D *seg){
	Vector3d vecA, vecB;
	vecA = v;
	vecB = seg->v;
	vecA.normalize();
	vecB.normalize();
	return (vecA - vecB).norm() < 0.00001 || (vecA + vecB).norm()< 0.00001;
}
bool Segment2D::isOn(Vector3d *vec){
	// 1-2, 3
	if ((s - *vec).norm() < 0.00001)return true;

	Vector3d vecA, vecB, vecA_, vecB_;
	vecA_ = vecA = v;
	vecB_ = vecB = *vec - s;
	vecA.normalize();
	vecB.normalize();
	// isOn line
	return (vecA - vecB).norm() < 0.00001 && vecA_.norm() >= vecB_.norm();

}
bool Segment2D::isOverlapSegment(Segment2D *seg){
	// 1-2, 3-4
	Vector3d s_v = s + v;
	Vector3d seg_s_v = seg->s + seg->v;
	return isParallel(seg) && (isOn(&seg->s) || isOn(&seg_s_v) || seg->isOn(&s) || seg->isOn(&s_v));
}
bool Segment2D::isIncludingSegment(Segment2D *seg){
	Vector3d seg_s_v = s + v;
	return isOn(&seg->s) && isOn(&seg_s_v);
}
void Segment2D::debugPrint(){
	cout << "s :\n" << s << "\nv :\n" << v << endl;
}