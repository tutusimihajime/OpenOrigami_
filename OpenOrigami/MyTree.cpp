#include "MyTree.h"
#include "Model.h"
#include <list>
using namespace std;
extern Model *model;
bool *tmpflag = NULL;//	tmpflag[i] EEE i (= face->id)‚ª–Ø‚É“ü‚Á‚Ä‚é‚©‚Ç‚¤‚©
int N;

void initTmpFlag(){
	if (tmpflag == NULL){
		N = model->faces.size();
		tmpflag = new bool[N];
	}
	for (int i = 0; i < N; ++i){
		tmpflag[i] = false;
	}
}
MyTree::MyTree(int id, SparseMatrix<int> *_overlapRelation){
	root = new MyNode(id);
	overlapRelation = *_overlapRelation;
	initTmpFlag();
	tmpflag[id] = true;
}
bool MyTree::insert(MyNode *node_insert, MyNode *node_base){
	switch (overlapRelation.coeff(node_insert->id, node_base->id)){
	case 0:
		return false;
		break;
	case 1:
		if (node_base->left == NULL){
			node_base->left = node_insert;
			tmpflag[node_insert->id] = true;
		}else{

		}
		break;
	case 2:
		if (node_base->right == NULL){
			node_base->right = node_insert;
			tmpflag[node_insert->id] = true;
		}else{

		}
		break;
	default:
		break;
	}
	return true;
}
bool MyTree::insert(int id){
	MyNode *n = new MyNode(id);
	insert(n, root);
	
	for (int i = 0; i < N; ++i){
		if (tmpflag[i]){

		}
	}
	return true;
}
void MyTree::addOverlapOrder(MyNode *node, list<int> *overlapList){
	if (node->left != NULL){
		addOverlapOrder(node->left, overlapList);
	}

	overlapList->push_back( node->id);

	if (node->right != NULL){
		addOverlapOrder(node->right, overlapList);
	}
	
}
void MyTree::getOverlapOrder(int *overlapOrder){
	list<int> *overlapList = new list < int > ;
	addOverlapOrder(root, overlapList);
	int i = 0;
	for (list<int>::iterator it = overlapList->begin(); it != overlapList->end(); ++it){
		overlapOrder[i] = *it;
		++i;
	}
}