#pragma once
#include "MyNode.h"
#include <Eigen/Sparse>
#include <list>
using namespace Eigen;
using namespace std;
class MyTree{
public:
	MyNode *root;
	SparseMatrix<int> overlapRelation;
	MyTree(int id, SparseMatrix<int> *_overlapRelation);
	bool insert(MyNode *node_insert, MyNode *node_base);
	bool insert(int id);
	void getOverlapOrder(int *overlapOrder);
	void addOverlapOrder(MyNode *node, list<int> *overlapList);
};
