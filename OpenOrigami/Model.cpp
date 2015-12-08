#pragma once
#include "Model.h"
#include "Segment2D.h"
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <cstdio>
#include <Eigen/Geometry>
#include "MyVector3d.h"
#include <map>
using namespace std;
extern double d;
extern int select;

Model::Model()
{
}
Model::Model(const char *filename)
{
	ifstream datafile(filename);
	vector<Vertex*> vertices4index; // インデックスで頂点を参照するためのvector
	vector<Vertex*> vertices4index2; // subface

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
		{
			// 面を構成する頂点インデックス情報の読み込み
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

			//頂点のリストを引数にして面の登録を行う
			addFace(vlist);
		}
		else if (str == "sv"){

			double x, y, z;
			datafile >> x >> y >> z;
			Vertex *v = new Vertex(x, y, z);
			v->setID(vertices4index2.size());
			vertices4index2.push_back(v);
			subvertexVector.push_back(v);
		}
		else if (str == "sf"){
			// 面を構成する頂点インデックス情報の読み込み
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
				vlist.push_back(vertices4index2[(*it_i) - 1]);
			}

			//頂点のリストを引数にして面の登録を行う
			Face *f = addFace2(vlist);
			f->type = 0;
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
void Model::calcNormal(){
	for (list < Face*>::iterator it = faces.begin(); it != faces.end(); ++it){
		(*it)->normalizeNormal();
	}
}
//delete関係はNEED TO FIX 9/16 ---testしてないけど一応 10/20
void Model::deleteVertex(Vertex *vertex) {
	vertices.remove(vertex);
	vector<Vertex*>::iterator it_v = remove(vertexVector.begin(), vertexVector.end(), vertex);
	vertexVector.erase(it_v);
	delete vertex;
}
void Model::deleteHalfedge(Halfedge *he){
	halfedges.remove(he);
	vector<Halfedge*>::iterator it_e = remove(halfedgeVector.begin(), halfedgeVector.end(), he);
	halfedgeVector.erase(it_e);
	delete he;
}
void Model::deleteFace(Face* face) {
	faces.remove(face);
	vector<Face*>::iterator it_f = remove(faceVector.begin(), faceVector.end(), face);
	faceVector.erase(it_f);
	list<Halfedge*> deleteEdges;
	Halfedge *he = (*it_f)->halfedge;
	do{
		deleteEdges.push_back(he);
		he = he->next;
	} while (he != (*it_f)->halfedge);
	while (!deleteEdges.empty()){
		Halfedge *garbage = deleteEdges.front();
		deleteEdges.pop_front();
		deleteHalfedge(garbage);
	}
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
//subfaceのaddFace
Face *Model::addFace2(list<Vertex*> vlist){
	vector<Halfedge*> halfedges4pairing;
	list<Vertex*>::iterator it_v;
	for (it_v = vlist.begin(); it_v != vlist.end(); ++it_v){
		Halfedge *he = new Halfedge(*it_v);
		(*it_v)->halfedge = he;
		halfedges4pairing.push_back(he);
	}
	for (int i = 0; i < vlist.size(); ++i){
		int i_next, i_prev;
		i_next = (i == vlist.size() - 1) ? 0 : i + 1;
		i_prev = (i == 0) ? vlist.size() - 1 : i - 1;
		halfedges4pairing[i]->next = halfedges4pairing[i_next];
		halfedges4pairing[i]->prev = halfedges4pairing[i_prev];
	}
	Face *face = new Face(halfedges4pairing[0]);
	face->setID(subfaceVector.size());
	subfaceVector.push_back(face);
	return face;
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
		glEnable(GL_LIGHTING);
		GLfloat materialColor1[] = { 1, 0.2, 0.2, 1 };
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor1);
		GLfloat materialColor2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
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
void Model::drawSubFaces(){
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 30);

	for (int i = 0; i < subfaceVector.size(); ++i){
		float col = (float)i / subfaceVector.size();
		glColor3d(1-col, 1-col, 1);
		subfaceVector.at(i)->draw();
		glColor3d(1, 1, 0);
		glPointSize(10);
		glBegin(GL_POINTS);
		glVertex3d(subfaceVector.at(i)->g->x(), subfaceVector.at(i)->g->y(), subfaceVector.at(i)->g->z());
		glEnd();
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	

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
	for (int i = 0; i < subvertexVector.size(); ++i){
		subvertexVector.at(i)->transPosition(m.x, m.y, m.z);
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
	Vertex *v = createVertex(_v->x, _v->y, _v->z);
	v->id_origin = _v->id;
	return v;
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
	v->id_origin = -1;
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
//OBJ出力
void Model::exportOBJ(){
	cout << "input name( ***.obj) : ";
	string filename;
	cin>>filename;
	std::ofstream file(filename+".obj");
	std::list<Vertex*>::iterator it_v;
	std::list<Face*>::iterator it_f;
	for (it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
		file << "v " << (*it_v)->x << " " << (*it_v)->y << " " << (*it_v)->z << std::endl;
	}
	for (it_f = faces.begin(); it_f != faces.end(); ++it_f){
		file << "f ";
		Halfedge *he = (*it_f)->halfedge;
		do{
			file << he->vertex->id + 1<<" ";
			he = he->next;
		} while (he != (*it_f)->halfedge);
		file << endl;
	}
}
void Model::exportSubFaceGroupsOBJ(){
	cout << "input name( ***.obj) : ";
	string filename;
	cin >> filename;
	std::ofstream file(filename + ".obj");
	std::list<SubFaceGroup*>::iterator it_sfg;
	std::list<Bridge*>::iterator it_b;
	std::list<Vertex*>::iterator it_v;
	std::list<Face*>::iterator it_f;
	//v 
	int i = 1;
	for (it_sfg = subFaceGroups.begin(); it_sfg != subFaceGroups.end(); ++it_sfg){
		for (it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			file << "v " << (*it_v)->x << " " << (*it_v)->y << " " << (*it_v)->z << std::endl;
			(*it_v)->itmp4w = i++;
		}
	}
	for (it_b = bridges.begin(); it_b != bridges.end(); ++it_b){
		for (it_v = (*it_b)->vertices.begin(); it_v != (*it_b)->vertices.end(); ++it_v){
			file << "v " << (*it_v)->x << " " << (*it_v)->y << " " << (*it_v)->z << std::endl;
			(*it_v)->itmp4w = i++;
		}
	}
	//f
	for (it_sfg = subFaceGroups.begin(); it_sfg != subFaceGroups.end(); ++it_sfg){
		for (it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			file << "f ";
			Halfedge *he = (*it_f)->halfedge;
			do{
				file << he->vertex->itmp4w << " ";
				he = he->next;
			} while (he != (*it_f)->halfedge);
			file << endl;
		}
	}

	for (it_b = bridges.begin(); it_b != bridges.end(); ++it_b){
		for (it_f = (*it_b)->faces.begin(); it_f != (*it_b)->faces.end(); ++it_f){
			file << "f ";
			Halfedge *he = (*it_f)->halfedge;
			do{
				file << he->vertex->itmp4w << " ";
				he = he->next;
			} while (he != (*it_f)->halfedge);
			file << endl;
		}
	}
	cout << "exportOBJ END\n";
}
//サブ～削除
void Model::deleteSubVertex(Vertex *garbage){
	if (garbage == NULL){
		cout << "garbage vertex is NULL.\n";
		return;
	}
	vector<Vertex*>::iterator it_v = subvertexVector.begin();
	for (int i = 0; i < subvertexVector.size(); ++i){
		if (subvertexVector.at(i) == garbage){
			break;
		}
		++it_v;
	}
	subvertexVector.erase(it_v);
	delete garbage;
}
void Model::deleteSubFace(Face *garbage){
	if (garbage == NULL){
		cout << "garabage face is NULL.\n";
		return;
	}
	vector<Face*>::iterator it_f = subfaceVector.begin();
	for (int i = 0; i < subfaceVector.size(); ++i){
		if(subfaceVector.at(i)==garbage){
			break;
		}
		++it_f;
	}
	subfaceVector.erase(it_f);

	int n = 0;//エッジの数を記録
	Halfedge *he = garbage->halfedge;
	do{
		deleteSubVertex(he->vertex);
		n++;
		he = he->next;
	} while (he != garbage->halfedge);
	
	for (int i = 0; i < n; ++i){
		Halfedge *he_next = he->next;
		if (he->pair != NULL){
			(he->pair)->pair = NULL;
		}
		delete he;
		he = he_next;
	}

	delete garbage;
}
void Model::deleteGarbageSubface(){
	if (subfaceVector.size() == 0){
		cout << "This Model does NOT have subfaces.\n";
		return;
	}
	Face *garbage = NULL;
	int v_num_Max = 0;
	int i_garbage = -1;
	for (int i = 0; i < subfaceVector.size(); ++i){
		int v_num = 0;
		Halfedge *he = subfaceVector.at(i)->halfedge;
		do{
			v_num++;
			he = he->next;
		} while (he != subfaceVector.at(i)->halfedge);
		if (v_num > v_num_Max){
			v_num_Max = v_num;
			garbage = subfaceVector.at(i);
			i_garbage = i;
		}
	}
	vector<Face*>::iterator it = subfaceVector.begin();
	for (int i = 0; i < i_garbage; ++i){
		++it;
	}
	subfaceVector.erase(it);
	Halfedge *he = garbage->halfedge;
	do{
		vector<Vertex*>::iterator it_v = subvertexVector.begin();
		for (int i = 0; i < faceVector.size(); ++i){
			if (subvertexVector.at(i)->id == he->vertex->id){
				Vertex *v = subvertexVector.at(i);
				subvertexVector.erase(it_v);
				delete v;
				break;
			}
			++it_v;
		}
		he = he->next;
	} while (he!=garbage->halfedge);
	delete garbage;
}
//重なりチェック

bool isCrossSegment2D(Segment2D *seg1, Segment2D *seg2){
	
	Vector3d v = seg2->s-seg1->s;//始点の差
	Vector3d Crs_v1_v2 = seg1->v.cross(seg2->v);
	
	//parallelならfalse
	if ( Crs_v1_v2.norm() == 0.0f){
		return false;
	}

	Vector3d Crs_v_v1 = v.cross(seg1->v);
	Vector3d Crs_v_v2 = v.cross(seg2->v);
	int sign1 = ((Crs_v_v2.z() > 0) - (Crs_v_v2.z() < 0));
	int sign2 = ((Crs_v1_v2.z() > 0) - (Crs_v1_v2.z() < 0));
	int sign3 = ((Crs_v_v1.z() > 0) - (Crs_v_v1.z() < 0));

	float t1 = sign1 * sign2 *Crs_v_v2.norm() / Crs_v1_v2.norm();
	float t2 = sign2 * sign3 * Crs_v_v1.norm() / Crs_v1_v2.norm();

	const float eps = 0.00001f;
	if (t1 + eps<0||t1-eps>1||t2+eps<0||t2-eps>1){
		return false;
	}
	Vector3d crossPos = seg1->s + seg1->v * t1;
	return true;
	
}

bool isOverlap2D(Face *subface, Face *face){////要修正
	//subfaceの重心(2D)を求める
	Vector2d g(subface->g->x(), subface->g->y());

	//faceがgを含むかどうか調べる
	Segment2D g_halfline(&g, &Vector2d(1000,0));
	int cnt = 0;
	Halfedge *he = face->halfedge;
	do{
		Segment2D edge(&Vector2d(he->vertex->x, he->vertex->y), &Vector2d(he->next->vertex->x - he->vertex->x, he->next->vertex->y - he->vertex->y));
		
		if (isCrossSegment2D(&g_halfline, &edge)){
			cnt++;
		}
		he = he->next;
	} while (he != face->halfedge);
	return cnt%2==1;
}
void Model::calcAllSubfaceG(){
	for (int i = 0; i < subfaceVector.size(); ++i){
		subfaceVector.at(i)->calcCenterOfGravity();
	}
}
void Model::checkOverlapSubface(){

	calcAllSubfaceG();

	subfaceOverlapFace = new bool*[subfaceVector.size()];
	for (int i = 0; i < subfaceVector.size(); ++i){
		subfaceOverlapFace[i] = new bool[faces.size()];
	}
	for (int i = 0; i < subfaceVector.size(); ++i){
		for (int j = 0; j < faceVector.size(); ++j){
			subfaceOverlapFace[i][j] = isOverlap2D(subfaceVector.at(i), faceVector.at(j));
		}
	}
	//test
	/*for (int i = 0; i < subfaceVector.size(); ++i){
		cout << "G" << i << " = (" << subfaceVector.at(i)->g->x() << ", " << subfaceVector.at(i)->g->y() << ", " << subfaceVector.at(i)->g->z() << ")\n";
	}*/
	/*for (int i = 0; i < subfaceVector.size(); ++i){
		for (int j = 0; j < faceVector.size(); ++j){
			cout << subfaceOverlapFace[i][j]<<" ";
		}
		cout << endl;
	}*/
}
//SubFaceGroup
bool compVertexItmp(Vertex *v1, Vertex *v2){
	return v1->itmp < v2->itmp;
}
void Model::constructSubFaceGroup(){
	//SFGを作る, Face分割、subfacesのitmp,idまで
	for (int j = 0; j < faceVector.size(); ++j){
		list<Face*> subfaces;
		for (int i = 0; i < subfaceVector.size(); ++i){
			if (subfaceOverlapFace[i][j]){
				subfaces.push_back(subfaceVector.at(i));
			}
		}
		subFaceGroups.push_back(new SubFaceGroup(faceVector.at(j), subfaces));
	}

	//ベクタベクタ作成・・・IDによっては、サイズが0のベクタもあることに注意 → ベクタベクタの外側のベクタのサイズは、存在するsubfaceID+1
	vector<vector<Face*> >id_subfaceVectorVector;
	for (int i = 0; i < subfaceVector.size() + 1; ++i){
		vector<Face*> id_subfaceVector;
		for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
			for (list<Face*>::iterator it_f = (*it)->subfaces.begin(); it_f != (*it)->subfaces.end(); ++it_f){
				if (i == (*it_f)->id){
					id_subfaceVector.push_back(*it_f);
				}
			}
		}
		id_subfaceVectorVector.push_back(id_subfaceVector);
	}
	//---サブフェース再配置開始---//
	/*
	//サブフェースのitmpの圧縮, itmp2は、グループ内での順位
	int all_ave = 0;
	for (int i = 0; i < id_subfaceVectorVector.size(); ++i){
		int min_itmp = 0;
		int ave_itmp = 0;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			ave_itmp += id_subfaceVectorVector[i][j]->itmp;
		}
		if (id_subfaceVectorVector.at(i).size() == 0){
			continue;
		}
		ave_itmp /= id_subfaceVectorVector.at(i).size();
		all_ave += ave_itmp;
	}
	all_ave /= id_subfaceVectorVector.size();

	for (int i = 0; i < id_subfaceVectorVector.size(); ++i){
		int min_itmp = 0;
		int ave_itmp = 0;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			id_subfaceVectorVector[i][j]->itmp2 = 0;//itmp2初期化
			for (int k = 0; k < id_subfaceVectorVector.at(i).size(); ++k){
				if (id_subfaceVectorVector[i][j]->itmp > id_subfaceVectorVector[i][k]->itmp){
					id_subfaceVectorVector[i][j]->itmp2++;
				}
			}
			ave_itmp += id_subfaceVectorVector[i][j]->itmp;
			min_itmp = min(min_itmp, id_subfaceVectorVector[i][j]->itmp);
		}
		if (id_subfaceVectorVector.at(i).size() == 0){
			continue;
		}
		ave_itmp /= id_subfaceVectorVector.at(i).size();
		int mid = id_subfaceVectorVector.at(i).size() / 2;
		//cout << "ave_itmp =" << ave_itmp << ", mid = " << mid << endl;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			//id_subfaceVectorVector[i][j]->itmp = id_subfaceVectorVector[i][j]->itmp2 + min_itmp;//底寄せ
			id_subfaceVectorVector[i][j]->itmp = id_subfaceVectorVector[i][j]->itmp2 + all_ave - id_subfaceVectorVector.at(i).size()/2;//中寄せ??全体の基準を計算して、それに寄せる方法がよい?
		}
	}
	*/
	//---サブフェース再配置END---//
	//頂点のitmpの初期化
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		for (list < Face* > ::iterator it_f = (*it)->subfaces.begin(); it_f != (*it)->subfaces.end(); ++it_f){
			Halfedge *he = (*it_f)->halfedge;
			do{
				he->vertex->itmp = (*it_f)->itmp;
				he = he->next;
			} while (he != (*it_f)->halfedge);
		}
	}

	//id_subVertexListListの構築
	list<list<Vertex*> > id_subVertexListList;
	for (vector<Vertex*>::iterator it_sv = subvertexVector.begin(); it_sv != subvertexVector.end(); ++it_sv){
		int id = (*it_sv)->id;
		list<Vertex*> id_subVertexList;
		for (list<SubFaceGroup*>::iterator it_sfg = subFaceGroups.begin(); it_sfg != subFaceGroups.end(); ++it_sfg){
			for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
				Halfedge *he = (*it_f)->halfedge;
				do{
					if (id == he->vertex->id){
						id_subVertexList.push_back(he->vertex);
					}
					he = he->next;
				} while (he != (*it_f)->halfedge);
			}
		}
		if (!id_subVertexList.empty()){
			id_subVertexListList.push_back(id_subVertexList);
		}
	}

	//id_subVertexListListのソート
	for (list<list<Vertex*>>::iterator it = id_subVertexListList.begin(); it != id_subVertexListList.end(); ++it){
		(*it).sort(compVertexItmp);
	}
	
	//擬似頂点マージ（itmpだけ高いやつにそろえる）
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->unifyAllVertexItmp();
	}
	
	//頂点のitmp再配置
	for (list<list<Vertex*> >::iterator it = id_subVertexListList.begin(); it != id_subVertexListList.end(); ++it)
	{
		for (list<Vertex*>::iterator it_v = (*it).begin(); it_v != (*it).end(); ++it_v)
		{
			for (list<Vertex*>::iterator it_v2 = (*it).begin(); it_v2 != it_v; ++it_v2)
			{
				if ((*it_v)->itmp <= (*it_v2)->itmp)
				{
					(*it_v)->itmp = (*it_v2)->itmp + 1;
				}
			}
		}
	}
	
	//v->itmpによる頂点再配置	
	
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		for (list<Face*>::iterator it_f = (*it)->subfaces.begin(); it_f != (*it)->subfaces.end(); ++it_f){
			Halfedge *he = (*it_f)->halfedge;
			do{
				he->vertex->z = he->vertex->itmp * d;
				he = he->next;
			} while (he != (*it_f)->halfedge);
		}
	}
	
	//Z優先で頂点のマージ・・・deleteによってid_subVertexListListはぶっ壊れます!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->mergeAllVertexPair();
	}

	//ハーフエッジペアリング
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->makeInnerPairing();
	}
	makeOuterPairing2();

	//サブフェースの法線計算
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->calcSubFacesNormal();
	}

}
void Model::drawSubFaceGroups(){
	if (subFaceGroups.empty()){
		return;
	}
	//vertex
	glDisable(GL_LIGHTING);
	glPointSize(5);
	glColor3f(.3,.3,.3);
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->drawVertex();
	}
	//edge
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glColor3f(.6, .6, .6);
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->drawEdge();
	}
	//face
	glEnable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 30);
	GLfloat materialColor1[] = { 1, 0.2, 0.2, 1 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor1);
	GLfloat materialColor2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){	
		(*it)->drawFace();
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
}
void Model::drawSubFaceGroupBridges(){
	for (list<Bridge*>::iterator it_b = bridges.begin(); it_b != bridges.end(); ++it_b){
		(*it_b)->draw();
	}
}
void Model::debugPrintSFGs(){
	cout << "Debug SubFaceGroups:\n";
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->debugPrint();
	}
}
//Halfedge Pairing
void makeDtmp(Halfedge *parent, list<Halfedge*> *children){
	Vector2d vParent(parent->vertex->x, parent->vertex->y);
	for (list<Halfedge*>::iterator it_h = children->begin(); it_h != children->end(); ++it_h){
		Halfedge *child = *it_h;
		Vector2d vChild(child->vertex->x, child->vertex->y);
		child->dtmp = (vParent - vChild).norm();
	}
}
bool compHalfedgeDtmp(Halfedge *he1, Halfedge *he2){
	return he1->dtmp < he2->dtmp;
}

void Model::makeOuterPairing(){
	
	// create mapHeHeList
	map < Halfedge*, list<Halfedge*> > mapHeHeList;

	for (list<SubFaceGroup*>::iterator it_sfg = subFaceGroups.begin(); it_sfg != subFaceGroups.end(); ++it_sfg)
	{
		
		Halfedge *parent = (*it_sfg)->oldFace->halfedge;
		do{
			Segment2D segParent(parent);
			list<Halfedge*> children;
			for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f)
			{
				Halfedge *child = (*it_f)->halfedge;
				do{
					Segment2D segChild(child);
					Vector3d vParent = segParent.v;
					vParent.normalize();
					Vector3d vChild = segChild.v;
					vChild.normalize();
					double dot = vParent.dot(vChild);
					double cap = 0.00001;
					if (segParent.isIncludingSegment(&segChild)&&fabs(1-dot)<cap){
						children.push_back(child);
					}
					child = child->next;
				} while (child != (*it_f)->halfedge);
			}

			mapHeHeList.insert(pair<Halfedge*, list<Halfedge*>>(parent, children));
			parent = parent->next;
		} while (parent != (*it_sfg)->oldFace->halfedge);
	}

	//debug
	/*for (map < Halfedge*, list<Halfedge*> >::iterator it = mapHeHeList.begin(); it != mapHeHeList.end(); ++it){
		Halfedge *parent = (*it).first;
		printf("parent:s(%f, %f), v(%f, %f)\n", parent->vertex->x, parent->vertex->y, parent->next->vertex->x - parent->vertex->x, parent->next->vertex->y - parent->vertex->y);
		for (list<Halfedge*>::iterator it_f = (*it).second.begin(); it_f != (*it).second.end(); ++it_f){
			Halfedge *child = *it_f;
			printf(" child:s(%f, %f), v(%f, %f)\n", child->vertex->x, child->vertex->y, child->next->vertex->x - child->vertex->x, child->next->vertex->y - child->vertex->y);
		}
		cout << endl;
	}*/
	int cnt0 = 0;
	int cnt1 = 0;
	for (map < Halfedge*, list<Halfedge*> >::iterator it = mapHeHeList.begin(); it != mapHeHeList.end(); ++it){
		Halfedge *parent = (*it).first;
		if (parent->pair != NULL){
			cnt1++;
		}
		else{
			cnt0++;
		}
		makeDtmp( parent, &((*it).second) );
		(*it).second.sort(compHalfedgeDtmp);
		parent->checked = false;
	}
	cout << "mapHeHeList.size() = " << mapHeHeList.size() << "\n";
	cout << "cnt0 = " << cnt0 << endl;
	cout << "cnt1 = " << cnt1 << endl;
	//pairing・・・ここのどこかで落ちる10/25 19:47
	for (map < Halfedge*, list<Halfedge*> >::iterator it = mapHeHeList.begin(); it != mapHeHeList.end(); ++it){
		Halfedge *parent1 = (*it).first;
		if (parent1->pair != NULL && !parent1->checked){
			list<Halfedge*> children1 = (*it).second;
			Halfedge *parent2 = parent1->pair;
			map < Halfedge*, list<Halfedge*> >::iterator it2 = mapHeHeList.find(parent2);
			list<Halfedge*> children2 = (*it2).second;
			children2.reverse();
			list<Halfedge*>::iterator it_h2 = children2.begin();
			for (list<Halfedge*>::iterator it_h1 = children1.begin(); it_h1 != children1.end(); ++it_h1){
				(*it_h1)->setPair(*it_h2);
				(*it_h1)->checked = (*it_h2)->checked = true;
				++it_h2;
			}
			parent2->checked = true;
		}
		parent1->checked = true;
	}
}
void Model::makeOuterPairing2(){
	
	//hePairList作成

	list<pair<Halfedge*, list<Halfedge* > > > hePairList;

	for (list<SubFaceGroup*>::iterator it_sfg = subFaceGroups.begin(); it_sfg != subFaceGroups.end(); ++it_sfg){
		Halfedge *he_parent = (*it_sfg)->oldFace->halfedge;
		do{
			Segment2D segParent(he_parent); 
			Vector3d vParent = segParent.v;
			vParent.normalize();
			list<Halfedge*> he_children;
			for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f)
			{
				Halfedge *he_child = (*it_f)->halfedge;
				do{
					Segment2D segChild(he_child);
					Vector3d vChild = segChild.v;
					vChild.normalize();
					double dot = vParent.dot(vChild);
					double cap = 0.01;
					if (segParent.isIncludingSegment(&segChild) && fabs(1 - dot)<cap){
						he_children.push_back(he_child);
					}
					he_child = he_child->next;
				} while (he_child != (*it_f)->halfedge);
			}
			hePairList.push_back(pair < Halfedge*, list<Halfedge*>>(he_parent, he_children));
			he_parent = he_parent->next;
		} while (he_parent != (*it_sfg)->oldFace->halfedge);
	}

	//sort
	cout << "hePairList.size() = " << hePairList.size() << endl;
	for (list<pair<Halfedge *, list<Halfedge * > > >::iterator it = hePairList.begin(); it != hePairList.end(); ++it){
		Halfedge *parent = (*it).first;
		makeDtmp(parent, &((*it).second));
		(*it).second.sort(compHalfedgeDtmp);
		parent->checked = false;
	}
	
	//hePairList-test
	/*cout << "test\n";
	for (list<pair<Halfedge *, list<Halfedge * > > >::iterator it = hePairList.begin(); it != hePairList.end(); ++it){
		printf("first = %d\n", (*it).first);
		cout << "second = {";
		for (list < Halfedge* >::iterator it_h = (*it).second.begin(); it_h != (*it).second.end(); ++it_h){
			printf(" %d", *it_h);
		}
		cout << " }\n";
	}*/
	
	//pairing
	for (list<pair<Halfedge *, list<Halfedge * > > >::iterator it = hePairList.begin(); it != hePairList.end(); ++it){
		Halfedge *parent1 = (*it).first;
		if (parent1->pair != NULL && !parent1->checked){
			list<Halfedge*> children1 = (*it).second;
			if (children1.size() != 0){
				Halfedge *parent2 = parent1->pair;
				list<pair<Halfedge *, list<Halfedge * > > >::iterator it2;
				for (it2 = hePairList.begin(); it2 != hePairList.end(); ++it2){
					if ((*it2).first == parent2){
						break;
					}
				}
				
				list<Halfedge*> children2 = (*it2).second;
				if (children2.size()==children1.size()){
					/*cout << endl;
					cout << "children1.size() = " << children1.size() << endl;
					cout << "children2.size() = " << children2.size() << endl;
					*/
					children2.reverse();
					list<Halfedge*>::iterator it_h2 = children2.begin();
					for (list<Halfedge*>::iterator it_h1 = children1.begin(); it_h1 != children1.end(); ++it_h1){
						(*it_h1)->setPair(*it_h2);
						(*it_h1)->checked = (*it_h2)->checked = true;
						++it_h2;
					}
					parent2->checked = true;
				}
				
			}
			parent1->checked = true;
		}
		
	}
}