#include "ReconstructingModel.h"
#include <list>
#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>

using namespace std;
using namespace Eigen;

void relocationFaces(Model *mod)
{
	
	//d‚È‚è‡‚ðŒˆ’è
	int *overlapOrder = new int[mod->faces.size()];
	for (int i = 0; i < mod->faces.size(); ++i){
		overlapOrder[i] = 0;
	}
	for (int i = 0; i < mod->faces.size(); ++i){		
		for (int j = 0; j < i; ++j){
			if (mod->overlapRelation.coeff(i,j) == 1){
				overlapOrder[i]++;
			}else if (mod->overlapRelation.coeff(i,j) == 2){
				overlapOrder[j]++;
			}
		}
	}
	list<int> tmplist;
	for (int i = 0; i < mod->faces.size(); ++i){
		tmplist.push_back(overlapOrder[i]);
	}
	tmplist.sort();
	tmplist.unique();
	/* //Debug
	cout << "tmplist = ";
	for (list<int>::iterator it = tmplist.begin(); it != tmplist.end(); ++it){
		cout << *it << ", ";
	}
	cout << endl;*/
	for (int i = 0; i < mod->faces.size(); ++i){
		int j = 0;
		for (list<int>::iterator it = tmplist.begin(); it != tmplist.end(); ++it){
			if (*it == overlapOrder[i]){
				overlapOrder[i] = j;
			}
			j++;
		}
	}
	//–Ê‚ð•ª—£
	for (list<Vertex*>::iterator it_v = mod->vertices.begin(); it_v != mod->vertices.end(); ++it_v){
		(*it_v)->halfedge = NULL;
	}
	for (int i = 0; i < mod->faceVector.size(); ++i){
		Face *f = mod->faceVector.at(i);
		Halfedge *he_in_f = f->halfedge;
		do{
			if (he_in_f->vertex->halfedge == NULL){
				he_in_f->vertex->halfedge = he_in_f;
			}else{
				he_in_f->vertex = mod->cpyVertex(he_in_f->vertex);
				he_in_f->vertex->halfedge = he_in_f;
			}
			he_in_f = he_in_f->next;
		} while (he_in_f != f->halfedge);;
	}
	
	//–Ê‚ðˆÚ“®
	for (int i = 0; i < mod->faceVector.size(); ++i){
		mod->faceVector.at(i)->transPosition(10 * overlapOrder[i] * Vector3d(0, 0, 1));
	}

	//Debug
	/*cout << mod->overlapRelation << endl;
	cout << "overlapOrder = ";
	for (int i = 0; i < mod->faces.size(); ++i){
		cout <<overlapOrder[i]<<", ";
	}
	cout << endl;*/
}
void bridgeEdges(Model *mod)
{


}
void moveOverlappedVertices(Model *mod)
{

}
void reconstructModel(Model *mod)
{
	// relocation facs
	relocationFaces(mod);
	// bridge edges
	bridgeEdges(mod);
	// move overlapped vertices
	moveOverlappedVertices(mod);
}