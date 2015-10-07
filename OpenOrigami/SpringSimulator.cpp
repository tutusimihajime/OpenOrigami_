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
SpringSimulator::SpringSimulator(Model *model){
	m_model = model;
	v_flag = e_flag = f_flag = true;
	xray_flag = false;
	generateSpringEdges();
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
	//CKF
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
			if (mode == GL_SELECT){
				glLoadName((GLuint)(*it_v)->id);
				glPointSize(4);

			}else{
				
				glPointSize(4);
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
				if ((*it_h)->pair == NULL){

					glColor3f(0.3, 0.3, 0.3);
					glLineWidth(2);

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