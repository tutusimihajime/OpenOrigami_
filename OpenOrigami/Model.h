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
		
	// �s�b�L���O�Ŏg�����߂ɋ}篍쐬�����x�N�^�B���X�g����Ȃ��čŏ����炱�����ł悩��������
	vector<Face*> faceVector;
	vector<Halfedge*> halfedgeVector;
	vector<Vertex*> vertexVector;

	//�ʂ̏d�Ȃ����ۑ�
	SparseMatrix<int> overlapRelation;

	Model();
	Model(const char *filename);
	~Model();
	//���X�g���� vertex ���폜���� 
	void deleteVertex(Vertex *vertex);
	//���X�g���� face ���폜����
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
	Halfedge **hash_matrix_he;//hash��halfedege*���i�[����n*1�s��(���O��matrix�����Amatrix����Ȃ���vector)
	int Model::getHashKey(Vertex *v1, Vertex *v2, int M);
	void Model::insertHashMatrix(int M);
	void Model::setHalfedgePair_hash(int M);
	void Model::setAllHalfedgePair_hash();
};

