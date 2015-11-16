#pragma once
#include "Face.h"
#include "Halfedge.h"
#include "Vertex.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
using namespace Eigen;

Vector3d createVector3d(Halfedge *he);
Vector3d createVector3d(Vertex *v);
Vector3d createVector3d(MyVector3d v);