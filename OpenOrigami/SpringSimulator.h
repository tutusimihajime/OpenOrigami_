#pragma once
#include "Model.h"
#include "SpringEdge.h"
#include "SpringVertex.h"

#include <list>
#include <vector>
using namespace std;

class SpringSimulator{
private:
	Model *m_model;
	list<SpringEdge*> sedges;
	list<SpringVertex*> svertices;
	double K;

public:
	//ピッキングのために以下略
	vector<SpringEdge*> sedgeVector;
	vector<SpringVertex*> svertexVector;
	bool v_flag;
	bool e_flag;
	bool f_flag;
	int f_type;//0:通常, 1:間引く, 2:色分け
	bool xray_flag;
	SpringSimulator();
	SpringSimulator(Model *model);
	SpringSimulator *subfaceSpring(Model *model);
	void generateSpringEdges();
	void simulate(double dt);
	void draw(GLenum mode = GL_RENDER);
	void debugPrint();
	void setImmobilty(int id = 0);
	void gainFirstVelocity(int id=0, MyVector3d vec = MyVector3d(0,0,1));
	void caluculateKineticEnergy();
};