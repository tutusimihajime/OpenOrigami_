#include "SpringSimulator.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>

extern int select;

inline void initRand(){
	srand((unsigned int)time(NULL));
}

void SpringSimulator::generateSpringEdges(){
	//test 10/6
	//initRand();

	vector<SpringVertex*> svertices4index;
	//まずSpringVertexを作る
	list<Vertex*>::iterator it_v;
	for (it_v = m_model->vertices.begin(); it_v != m_model->vertices.end(); ++it_v){
		SpringVertex *spv = new SpringVertex( (*it_v) );
		svertices.push_back(spv);
		svertices4index.push_back(spv);
	}
	//次にSpringEdgeを作る
	list<Halfedge*>::iterator it_h;

	for (it_h = m_model->halfedges.begin(); it_h != m_model->halfedges.end(); ++it_h){
		(*it_h)->checked = false;
	}

	for (it_h = m_model->halfedges.begin(); it_h != m_model->halfedges.end(); ++it_h){
		if (!(*it_h)->checked){
			SpringVertex *sv, *ev;
			sv = svertices4index[(*it_h)->vertex->id];
			ev = svertices4index[(*it_h)->next->vertex->id];
			SpringEdge *spe = new SpringEdge(sv, ev);
			
			//test 10/6
/*
			float d = 3;
			float Max = 100+d;
			float min = 100-d;
			spe->naturalLength *= 0.01 * (min+( rand() * (Max-min+1) / (1.0 + RAND_MAX) ));
*/
			sedges.push_back(spe);
			(*it_h)->checked = true;
			if ((*it_h)->pair != NULL){
				(*it_h)->pair->checked = true;
			}
		}
	}

	sedgeVector = vector<SpringEdge*>(sedges.begin(), sedges.end());
	svertexVector = vector<SpringVertex*>(svertices.begin(), svertices.end());
}
SpringSimulator::SpringSimulator(){
	v_flag = e_flag = f_flag = true;
	xray_flag = false;
	f_type = 0;
}
SpringSimulator::SpringSimulator(Model *model){
	m_model = model;
	v_flag = e_flag = f_flag = true;
	xray_flag = false;
	f_type = 0;
	generateSpringEdges();
}
// 2016.2.13
bool compareVertex(Vertex *v1, Vertex *v2){
	return v1->id < v2->id;//id が正しく設定されているかチェックする必要あり
}
SpringSimulator *SpringSimulator::subfaceSpring(Model *model){
	//init
	m_model = new Model();
	list<Face*> faces;
	list<Halfedge*> halfedges;
	list<Vertex*> vertices;
	
	//convert subfaceGroups and bridges -> model 
	list<SubFaceGroup*> subfaceGroups = model->subFaceGroups;
	list<Bridge*> bridges = model->bridges;
	
	// sfg->model
	for (list<SubFaceGroup*>::iterator it_sfg = subfaceGroups.begin(); it_sfg != subfaceGroups.end(); ++it_sfg){
		for (list<Vertex*>::iterator it_v = (*it_sfg)->subvertices.begin(); it_v != (*it_sfg)->subvertices.end(); ++it_v){
			vertices.push_back(*it_v);
		}
		for (list<Face*>::iterator it_f = (*it_sfg)->subfaces.begin(); it_f != (*it_sfg)->subfaces.end(); ++it_f){
			faces.push_back(*it_f);
			Halfedge *he_in_face = (*it_f)->halfedge;
			do{
				halfedges.push_back(he_in_face);

				he_in_face = he_in_face->next;
			} while (he_in_face != (*it_f)->halfedge);
		}
	}

	// bridge->model
	for (list < Bridge* >::iterator it_b = bridges.begin(); it_b != bridges.end(); ++it_b){
		for (list<Vertex*>::iterator it_v = (*it_b)->vertices.begin(); it_v != (*it_b)->vertices.end(); ++it_v){
			vertices.push_back(*it_v);
		}
		for (list<Face*>::iterator it_f = (*it_b)->faces.begin(); it_f != (*it_b)->faces.end(); ++it_f){
			faces.push_back(*it_f);
			Halfedge *he_in_face = (*it_f)->halfedge;
			do{
				halfedges.push_back(he_in_face);

				he_in_face = he_in_face->next;
			} while (he_in_face != (*it_f)->halfedge);
		}
	}
	
	
	//リスト重複チェック
	cout << "Face : \n";
	for (list < Face* > ::iterator it_f = faces.begin(); it_f != faces.end(); ++it_f){
		cout << (*it_f)->id << " ";
	}
	cout << endl << "Halfedge : \n";
	for (list < Halfedge* > ::iterator it_h = halfedges.begin(); it_h != halfedges.end(); ++it_h){
		cout << (*it_h)->id << " ";
	}
	cout << endl << "Vertex : \n";
	for (list < Vertex* > ::iterator it_v = vertices.begin(); it_v != vertices.end(); ++it_v){
		cout << (*it_v)->id << " ";
	}
	cout << endl;
	// 代入
	m_model->faces = faces;
	m_model->halfedges = halfedges;
	m_model->vertices = vertices;

	// SpringSimulatorへ...
	SpringSimulator *s = new SpringSimulator(m_model);
	return s;
}
MyVector3d createMyVector3d(SpringVertex *v1, SpringVertex *v2){
	return MyVector3d(v2->v->x - v1->v->x, v2->v->y - v1->v->y, v2->v->z - v1->v->z);
}
MyVector3d changeDirection(MyVector3d vec, MyVector3d dir, double rate)
{
	dir.normalize();
	dir.scale(vec.length());
	dir.scale(rate);
	vec.scale(1 - rate);
	MyVector3d v = dir + vec;
	return v;
}
void SpringSimulator::simulate(double dt)
{
	//頂点間の弱い斥力
	
	list<SpringVertex*>::iterator it_v;
	for (it_v = svertices.begin(); it_v != svertices.end(); ++it_v)
	{
		(*it_v)->v->checked = false;
	}
	if (0){
		list<SpringVertex*>::iterator it_v1, it_v2;
		for (it_v1 = svertices.begin(); it_v1 != svertices.end(); ++it_v1)
		{
			for (it_v2 = svertices.begin(); it_v2 != svertices.end(); ++it_v2)
			{
				if (!(*it_v2)->v->checked)
				{
					if ((*it_v1) != (*it_v2))
					{
						MyVector3d vec = createMyVector3d((*it_v1), (*it_v2));
						double l = log(vec.length());
						vec.normalize();
						vec.scale(l);
						double c = 0.0001;
						vec.scale(c);

						(*it_v1)->velocity -= vec;
						(*it_v2)->velocity += vec;
					}
				}
			}
			(*it_v1)->v->checked = true;
		}
	}
	//ばねの力 -> 頂点の速度
	list<SpringEdge*>::iterator it_e;
	for (it_e = sedges.begin(); it_e != sedges.end(); ++it_e)
	{
		(*it_e)->calculateSpringPower(dt);
	}

	//頂点の速度 -> 頂点の座標
	for (it_v = svertices.begin(); it_v != svertices.end(); ++it_v)
	{
		if ((*it_v)->mobility)
		{
			//(*it_v)->velocity = changeDirection((*it_v)->velocity, MyVector3d(0, 0, 1), 0.9);
			(*it_v)->move(dt);
			(*it_v)->velocity.scale(0.2);//減衰
		}
	}
	// Collision Detection
	//CKE
	caluculateKineticEnergy();
}
void SpringSimulator::debugPrint(){
	cout << "SpringEdges : " << sedges.size() << endl;
	cout << "SpringVertices : " << svertices.size() << endl;
}
void SpringSimulator::setImmobilty(int id){
	svertexVector[id]->mobility = false;
}
void SpringSimulator::gainFirstVelocity(int id, MyVector3d vec){
	list<SpringVertex*>::iterator it_v;
	for (it_v = svertices.begin(); it_v != svertices.end(); ++it_v){
		if ((*it_v)->v->id == id){
			(*it_v)->velocity = vec;
		}
	}
}
void SpringSimulator::caluculateKineticEnergy(){
	K = 0;
	list<SpringVertex*>::iterator it_v;
	for (it_v = svertices.begin(); it_v != svertices.end(); ++it_v){
		K += 0.5 * (*it_v)->m * (*it_v)->velocity.dot((*it_v)->velocity);
	}
	//cout << "K = " << K << endl;
}
void SpringSimulator::draw(GLenum mode){
	glDisable(GL_LIGHTING);

	if (v_flag){
		list<Vertex*>::iterator it_v;
		for (it_v = m_model->vertices.begin(); it_v != m_model->vertices.end(); ++it_v){
			/*
			if (f_type == 1 && (*it_v)->halfedge->face->type == 1){
				continue;
			}
			if (f_type == 2 && (*it_v)->halfedge->face->type == 1){
				continue;
			}
			if (f_type != 2 && (*it_v)->halfedge->face->type == 2){
				continue;
			}
			*/
			glPointSize(6);
			if (mode == GL_SELECT){
				glLoadName((GLuint)(*it_v)->id);
				//glPointSize(4);

			}else{
				
				//glPointSize(4);
				glColor3f(0.1, 0.1, 0.1);
				
				if (svertexVector[(*it_v)->id]->mobility == false && (*it_v)->id != select){
					glPointSize(6);
					glColor3f(0, 0, 0.8);
				}

				if (svertexVector[(*it_v)->id]->mobility != false && (*it_v)->id == select){
					glPointSize(6);
					glColor3f(0.8, 0, 0);

				}
				if (svertexVector[(*it_v)->id]->mobility == false && (*it_v)->id == select){
					glPointSize(6);
					glColor3f(0, 0.8, 0);

				}

				// Draw sphere
				/*
				glEnable(GL_LIGHTING);
				GLUquadric *sphere;
				sphere = gluNewQuadric();
				glPushMatrix();
				glTranslatef((*it_v)->x, (*it_v)->y, (*it_v)->z);
				gluSphere(sphere, 0.2, 8, 8);
				glPopMatrix();
				gluDeleteQuadric(sphere);
				glDisable(GL_LIGHTING);
				*/
			}
			(*it_v)->draw();
		}
	}
	if (e_flag){
		if (mode != GL_SELECT){
			glEnable(GL_LINE_SMOOTH);
			list<Halfedge*>::iterator it_h;
			for (it_h = m_model->halfedges.begin(); it_h != m_model->halfedges.end(); ++it_h){
				/*
				if (f_type == 1 && (*it_h)->face->type == 1){
					continue;
				}
				if (f_type == 2 && (*it_h)->face->type == 1){
					continue;
				}
				if (f_type != 2 && (*it_h)->face->type == 2){
					continue;
				}
				*/
				if ((*it_h)->pair == NULL){

					//glColor3f(0.9, 0, 0);
					//glLineWidth(3);

					glColor3f(0.6, 0.6, 0.6);
					glLineWidth(1);
				}else{
					glColor3f(0.6, 0.6, 0.6);
					glLineWidth(1);
				}
				(*it_h)->draw();
			}

			glDisable(GL_LINE_SMOOTH);
		}
	}
	if (f_flag){
		if (mode != GL_SELECT){
			glEnable(GL_POLYGON_SMOOTH);
			if (xray_flag){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else{
				glEnable(GL_LIGHTING);
			}
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1, 30);
			
			list<Face*>::iterator it_f;
			for (it_f = m_model->faces.begin(); it_f != m_model->faces.end(); ++it_f){
				/*
				if (f_type == 1 && (*it_f)->type == 1){
					continue;
				}
				if (f_type == 2 && (*it_f)->type == 1){
					continue;
				}
				if (f_type != 2 && (*it_f)->type == 2){
					continue;
				}
				*/
				if (xray_flag){
					glColor4f(1, 1, 1, 0.1);
				}
				else{
					GLfloat materialColor1[] = { 1, 0.2, 0.2, 1 };
					glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor1);
					GLfloat materialColor2[] = { 1.0f, 1.0f, 1.0f, 1.0f };
					glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, materialColor2);

				
				}
				(*it_f)->draw();
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
			
			if (xray_flag){
				glDisable(GL_BLEND);
			}
			glDisable(GL_POLYGON_SMOOTH);
		}
	}
}