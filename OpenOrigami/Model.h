#pragma once
#include <list>
#include <vector>
#include <gl/freeglut.h>
#include <Eigen/Sparse>
#include "Face.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "SubFaceGroup.h"

using namespace std;
using namespace Eigen;
class Model
{
private:
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

	//subface
	vector<Vertex*> subvertexVector;
	vector<Face*> subfaceVector;
	bool **subfaceOverlapFace;

	//SubFaceGroup
	list<SubFaceGroup*> subFaceGroups;

	//----------
	Model();
	Model(const char *filename);
	~Model();

	void calcNormal();
	//リストから vertex を削除する 
	void deleteVertex(Vertex *vertex);
	void deleteHalfedge(Halfedge *he);
	//リストから face を削除する
	void deleteFace(Face* face);
	Face *addFace(list<Vertex*> vlist);

	Face *addFace2(list<Vertex*> vlist);//subface用
	void draw(GLenum mode = GL_RENDER_MODE);
	void drawSubFaces();
	void debugPrint();
	void normalizeVertices();
	
	Face *cpyFace(Face *_f);
	Vertex *cpyVertex(Vertex *_v);
	
	//create and push list and vertex
	Face *createFace(Halfedge *he);
	Halfedge *createHalfedge(Vertex *v);
	Vertex *createVertex(double _x, double _y, double _z);
	Vertex *createVertex(Vector3d vec);
	
	//first pairing
	Halfedge **hash_matrix_he;//hashでhalfedege*を格納するn*1行列(名前はmatrixだが、matrixじゃなくてvector)
	int Model::getHashKey(Vertex *v1, Vertex *v2, int M);
	void Model::insertHashMatrix(int M);
	void Model::setHalfedgePair_hash(int M);
	void Model::setAllHalfedgePair_hash();
	
	//export
	void Model::exportOBJ();

	//subface
	void Model::deleteGarbageSubface();
	void Model::checkOverlapSubface();
	void Model::deleteSubVertex(Vertex *garbage);
	void Model::deleteSubFace(Face *garbage);

	// Calculate Center of Gravity
	void calcAllFaceG();
	void calcAllSubfaceG();

	//SubFaceGroup
	void Model::constructSubFaceGroup();
	void Model::drawSubFaceGroups();
	void Model::debugPrintSFGs();
};

