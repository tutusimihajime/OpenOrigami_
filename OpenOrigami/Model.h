#pragma once
#include <list>
#include <vector>
#include <gl/freeglut.h>
#include <Eigen/Sparse>
#include "Face.h"
#include "Halfedge.h"
#include "Vertex.h"
using namespace std;
using namespace Eigen;
class Model
{
private:
	//AAAAAAAAAAAA
public:
	list<Face*> faces;
	list<Halfedge*> halfedges;
	list<Vertex*> vertices;
		
	// ピッキングで使うために急遽作成したベクタ。リストじゃなくて最初からこっちでよかったかも
	vector<Face*> faceVector;
	vector<Halfedge*> halfedgeVector;
	vector<Vertex*> vertexVector;

	//面の重なり情報を保存
	SparseMatrix<int> overlapRelation;

	Model();
	Model(const char *filename);
	~Model();
	//リストから vertex を削除する 
	void deleteVertex(Vertex *vertex);
	//リストから face を削除する
	void deleteFace(Face* face);
	Face *addFace(list<Vertex*> vlist);
	void draw(GLenum mode = GL_RENDER_MODE);
	void debugPrint();
	void normalizeVertices();
	
	Face *cpyFace(Face *_f);
	Vertex *cpyVertex(Vertex *_v);
	
	//create and push list and vertex
	Face *createFace(Halfedge *he);
	Halfedge *createHalfedge(Vertex *v);
	Vertex *createVertex(double _x, double _y, double _z);

	//first pairing
	Halfedge **hash_matrix_he;//hashでhalfedege*を格納するn*1行列(名前はmatrixだが、matrixじゃなくてvector)
	int Model::getHashKey(Vertex *v1, Vertex *v2, int M);
	void Model::insertHashMatrix(int M);
	void Model::setHalfedgePair_hash(int M);
	void Model::setAllHalfedgePair_hash();
};

