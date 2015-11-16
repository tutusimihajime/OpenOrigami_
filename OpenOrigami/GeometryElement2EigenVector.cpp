#pragma once 
#include "GeometryElement2EigenVector.h"
Vector3d createVector3d(Halfedge *he){
	return Vector3d(he->next->vertex->x - he->vertex->x, he->next->vertex->y - he->vertex->y, he->next->vertex->z - he->vertex->z);
}
Vector3d createVector3d(Vertex *v){
	return Vector3d(v->x, v->y, v->z);
}
Vector3d createVector3d(MyVector3d v){
	return Vector3d(v.x, v.y, v.z);
}
