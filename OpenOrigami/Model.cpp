
#include "Model.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <cstdio>

#include "MyVector3d.h"

using namespace std;

extern int select;

Model::Model()
{
}
Model::Model(const char *filename)
{
	ifstream datafile(filename);
	vector<Vertex*> vertices4index; // インデックスで頂点を参照するためのvector

	vector<Face*> faces4layer; //フェースにレイヤー数を設定するためのvector
	int index_f=0;
	string str;

	while (datafile >> str)
	{
		if (str == "v")
		{// 頂点情報の読み込み

			double x, y, z;
			datafile >> x >> y >> z; // (1)頂点座標をファイルから読み込む
			Vertex *v = createVertex(x, y, z);
			vertices4index.push_back(v);

		}
		else if (str == "f")
		{// 面を構成する頂点インデックス情報の読み込み
			char buf[255];
			char *pbuf = buf;
			int n_elements = 0;
			datafile.getline(buf, sizeof(buf));
			while (*pbuf){
				if (*pbuf == ' ') {
					n_elements++;
				}
				pbuf++;
			}

			list<int> index;
			//(4)面を構成する頂点インデックスをファイルから読み込む
			pbuf = buf;
			for (int i = 0; i < n_elements; i++)
			{
				pbuf = strchr(pbuf, ' ');
				pbuf++;
				int i_tmp;
				//　構成要素の読み取り
				sscanf_s(pbuf, "%d", &i_tmp);
				index.push_back(i_tmp);
			}

			list<Vertex*> vlist;
			for (list<int>::iterator it_i = index.begin(); it_i != index.end(); ++it_i)
			{
				vlist.push_back(vertices4index[(*it_i) - 1]);
			}

			//頂点をZにずらす
			list<Vertex*>::iterator it_v;
			for (it_v = vlist.begin(); it_v != vlist.end(); ++it_v){
				//(*it_v)->z = faces.size()*10;
			}
			//頂点のリストを引数にして面の登録を行う
			faces4layer.push_back( addFace(vlist) );
		}
		else if (str == "m"){
			overlapRelation.resize(faces.size(), faces.size());
			overlapRelation.setZero();
			for (int i = 0; i < faces.size(); ++i)
			{
				for (int j = 0; j < faces.size(); ++j)
				{
					int tmp;
					datafile >> tmp;
					if (tmp != 0)
					{
						overlapRelation.insert(i, j) = tmp;
					}
				}
			}
		}
		else{
			datafile.ignore(1024, '\n');//行末まで読み飛ばし      
		}
	}
	normalizeVertices();
	setAllHalfedgePair_hash();

	//cout << overlapRelation;
}
Model::~Model()
{
}
//delete関係はNEED TO FIX 9/16
void Model::deleteVertex(Vertex *vertex) {
	vertices.remove(vertex);
	delete vertex;
}
void Model::deleteFace(Face* face) {
	faces.remove(face);
	delete face->halfedge->next;
	delete face->halfedge->prev;
	delete face->halfedge;
	delete face;
}

Face *Model::addFace(list<Vertex*> vlist){
	vector<Halfedge*> halfedges4pairing;
	list<Vertex*>::iterator it_v;
	for (it_v = vlist.begin(); it_v != vlist.end(); ++it_v){
		Halfedge *he = createHalfedge(*it_v);
		halfedges4pairing.push_back(he);
	}
	for (int i = 0; i < vlist.size(); ++i){
		int i_next, i_prev;
		i_next = (i == vlist.size() - 1) ? 0 : i + 1;
		i_prev = (i == 0) ? vlist.size() - 1 : i - 1;
		halfedges4pairing[i]->next = halfedges4pairing[i_next];
		halfedges4pairing[i]->prev = halfedges4pairing[i_prev];
	}
	return createFace(halfedges4pairing[0]);
}
void Model::draw(GLenum mode){
	glDisable(GL_LIGHTING);
	
	if (1){
		
		list<Vertex*>::iterator it_v;
		for (it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
			if (mode == GL_SELECT){
				glLoadName( (GLuint)(*it_v)->id );
				glPointSize(3);
			}
			else{
				if ((*it_v)->id != select){
					glPointSize(3);
					glColor3f(0.1, 0.1, 0.1);
				}else{
					glPointSize(5);
					glColor3f(0.8, 0, 0);
				}
				
			}
			(*it_v)->draw();
		}
	}
	if (mode != GL_SELECT){
		glColor3f(0.3, 0.3, 0.3);
		glLineWidth(1);
		list<Halfedge*>::iterator it_h;
		for (it_h = halfedges.begin(); it_h != halfedges.end(); ++it_h){

			(*it_h)->draw();
		}
	}
	if (mode != GL_SELECT){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1, 30);
		glColor4f(1, 1, 1, 0.1);
		
		list<Face*>::iterator it_f;
		for (it_f = faces.begin(); it_f != faces.end(); ++it_f){
			(*it_f)->draw();
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_BLEND);

	}

}
void Model::debugPrint(){
	cout << "vertices.size() = " << vertices.size() << endl;
	list<Vertex*>::iterator it_v;
	for (it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
		(*it_v)->debugPrint();
	}
	
	cout << "halfedges.size() = " << halfedges.size() << endl;
	list<Halfedge*>::iterator it_h;
	for (it_h = halfedges.begin(); it_h != halfedges.end(); ++it_h){
		(*it_h)->debugPrint();
	}
	cout << "faces.size() = " << faces.size() << endl;
	list<Face*>::iterator it_f;
	for (it_f = faces.begin(); it_f != faces.end(); ++it_f){
		(*it_f)->debugPrint();
	}

}
void Model::normalizeVertices(){
	list<Vertex*>::iterator it_v;
	MyVector3d m(0,0,0);
	for (it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
		m += MyVector3d((*it_v)->x, (*it_v)->y, (*it_v)->z);
	}

	m.scale(-1 / (double)vertices.size());

	for (it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
		(*it_v)->transPosition(m.x,m.y,m.z);
	}
}
Face *Model::cpyFace(Face *_f){
	Halfedge *he_in_f = _f->halfedge;
	vector<Halfedge*> hes;
	//create vertex and halfedge
	do{
		Vertex *v = createVertex(he_in_f->vertex->x, he_in_f->vertex->y, he_in_f->vertex->z); 
		Halfedge *he = createHalfedge(v);
		he_in_f = he_in_f->next;
	} while (he_in_f != _f->halfedge);
	
	//pairing edges by vector<Halfedge*>
	for (int i = 0; i < hes.size(); ++i){
		hes[i]->next = (i == hes.size() - 1) ? hes[0] : hes[i + 1];
		hes[i]->prev = (i == 0) ? hes[hes.size() - 1] : hes[i - 1];
		
	}

	//create Face
	return createFace(hes[0]);
}
Vertex *Model::cpyVertex(Vertex *_v){
	return createVertex(_v->x, _v->y, _v->z);
}


//create and push list and vertex
Face *Model::createFace(Halfedge *he){
	Face *face = new Face(he);
	face->setID(faces.size());
	faces.push_back(face);
	faceVector.push_back(face);
	return face;
}
Halfedge *Model::createHalfedge(Vertex *v){
	Halfedge *he = new Halfedge(v);
	he->setID(halfedges.size());
	halfedges.push_back(he);
	halfedgeVector.push_back(he);
	return he;
}
Vertex *Model::createVertex(double _x, double _y, double _z){
	Vertex *v = new Vertex(_x, _y, _z);// (2)読みこんだ x, y, z 座標に基づいて新しい Vertexを生成する
	v->setID(vertices.size());
	vertices.push_back(v); // (3)生成した Vertexを Modelに追加する 
	vertexVector.push_back(v);
	return v;
}
Vertex *Model::createVertex(Vector3d vec){
	return createVertex(vec.x(), vec.y(), vec.z());
}

//ハッシュ関連

// 二つのハーフエッジを互いに pair とする 
void setHalfedgePair(Halfedge *he0, Halfedge *he1) {
	he0->pair = he1;
	he1->pair = he0;
}
int Model::getHashKey(Vertex *v1, Vertex *v2, int M){
	int a, b;
	a = v1->id;
	b = v2->id;
	return (2 * a + 3 * b) % M;
}
void Model::insertHashMatrix(int M){
	std::list<Face*>::iterator it_f;
	int hashkey;
	Halfedge *he;
	bool insert_flag;
	for (it_f = faces.begin(); it_f != faces.end(); ++it_f){
		he = (*it_f)->halfedge;
		do{
			insert_flag = false;
			hashkey = getHashKey(he->vertex, he->next->vertex, M);
			while (!insert_flag){
				if (hash_matrix_he[hashkey] == NULL){
					hash_matrix_he[hashkey] = he;
					insert_flag = true;
				}
				else{
					hashkey += 3;
				}
			}
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
//hashでpairをset(すべてのpairを結ぶ)
void Model::setHalfedgePair_hash(int M){
	int hashkey;
	bool getflag;
	Halfedge *he;
	std::list<Face*>::iterator it_f;
	for (it_f = faces.begin(); it_f != faces.end(); ++it_f){
		he = (*it_f)->halfedge;
		do{
			hashkey = getHashKey(he->next->vertex, he->vertex, M);
			if (he->pair == NULL){
				getflag = false;
				while (!getflag){
					getflag = true;
					if (hash_matrix_he[hashkey] == NULL){

					}
					else if (hash_matrix_he[hashkey]->vertex->id == he->next->vertex->id&&hash_matrix_he[hashkey]->next->vertex->id == he->vertex->id){
						setHalfedgePair(he, hash_matrix_he[hashkey]);
					}
					else{
						hashkey += 3;
						getflag = false;
					}
				}
			}
			he = he->next;
		} while (he != (*it_f)->halfedge);
	}
}
//ハッシュでpairを結ぶ処理をまとめておこなう
void Model::setAllHalfedgePair_hash(){
	int M = 3 * (2 * (vertices.size() + faces.size() + 2)) + 1;
	//生成と初期化
	hash_matrix_he = new Halfedge*[M];
	for (int i = 0; i<M; i++){
		hash_matrix_he[i] = NULL;
	}
	//挿入
	insertHashMatrix(M);
	//pairを結ぶ
	setHalfedgePair_hash(M);
	//削除
	delete[] hash_matrix_he;
}