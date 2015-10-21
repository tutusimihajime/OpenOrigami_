
#include "Model.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <cstdio>
#include <Eigen/Geometry>
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
class Segment{
public:
	Vector3d s;//始点
	Vector3d v;//方向
	Segment(Vector2d *_s, Vector2d *_v){
		s = Vector3d(_s->x(), _s->y(), 0);
		v = Vector3d(_v->x(), _v->y(), 0);
	}
	void debugPrint(){
		cout << "s :\n" << s<<"\nv :\n"<<v<<endl;
	}
};
bool isCrossSegment(Segment *seg1, Segment *seg2){
	
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
bool isOverlap2D(Face *subface, Face *face){
	//subfaceの重心(2D)を求める
	Vector2d g(subface->g->x(), subface->g->y());

	//faceがgを含むかどうか調べる
	Segment g_halfline(&g, &Vector2d(1000,0));
	int cnt = 0;
	Halfedge *he = face->halfedge;
	do{
		Segment edge(&Vector2d(he->vertex->x, he->vertex->y), &Vector2d(he->next->vertex->x - he->vertex->x, he->next->vertex->y - he->vertex->y));
		
		if (isCrossSegment(&g_halfline, &edge)){
			cnt++;
		}
		he = he->next;
	} while (he != face->halfedge);
	return cnt % 2 == 1;
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
		for (int j = 0; j < faceVector.size(); ++j){
			cout << subfaceOverlapFace[i][j]<<" ";
		}
		cout << endl;
	}*/
}
//SubFaceGroup
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
	//ベクタベクタ作成・・・IDによっては、サイズが0のベクタもあることに注意 → ベクタベクタの外側のベクタのサイズは存在するsubfaceID+1
	vector<vector<Face*> >id_subfaceVectorVector;
	for (int i = 0; i < subfaceVector.size()+1; ++i){
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

	//test
	/*cout << "id_subfaceVectorVector.size() = "<<id_subfaceVectorVector.size()<<endl;
	for (int i = 0; i < id_subfaceVectorVector.size(); ++i){
		cout << "  id_subfaceVectorVector.at("<<i<<").size() = " << id_subfaceVectorVector.at(i).size() << endl;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			cout <<"  "<< id_subfaceVectorVector[i][j]->itmp << " ";
		}
		cout << endl;
	}*/

	//サブフェースのitmpの圧縮, itmp2は、グループ内での順位
	
	for (int i = 0; i < id_subfaceVectorVector.size(); ++i){
		int min_itmp = 0;
		int ave_itmp = 0;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			id_subfaceVectorVector[i][j]->itmp2 = 0;
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
			id_subfaceVectorVector[i][j]->itmp = id_subfaceVectorVector[i][j]->itmp2 + min_itmp;//底寄せ
			//id_subfaceVectorVector[i][j]->itmp = id_subfaceVectorVector[i][j]->itmp2 + ave_itmp -mid;//中寄せ
		}
	}
	//test
	/*cout << "subfaceVector.size() = " << subfaceVector.size() << endl;
	cout << "id_subfaceVectorVector.size() = " << id_subfaceVectorVector.size() << endl;
	for (int i = 0; i < id_subfaceVectorVector.size(); ++i){
		cout << "  id_subfaceVectorVector.at(" << i << ").size() = " << id_subfaceVectorVector.at(i).size() << endl;
		for (int j = 0; j < id_subfaceVectorVector.at(i).size(); ++j){
			cout << "  " << id_subfaceVectorVector[i][j]->itmp << " ";
		}
		cout << endl;
	}*/
	//itmpによるsubfacesの再配置
	const double d = 2;
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		for (list<Face*>::iterator it_f = (*it)->subfaces.begin(); it_f != (*it)->subfaces.end(); ++it_f){
			(*it_f)->setZ((*it_f)->itmp*d);
		}
	}


	//頂点のマージ
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->mergeAllVertexPair();
	}
	
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
void Model::debugPrintSFGs(){
	cout << "Debug SubFaceGroups:\n";
	for (list<SubFaceGroup*>::iterator it = subFaceGroups.begin(); it != subFaceGroups.end(); ++it){
		(*it)->debugPrint();
	}
}