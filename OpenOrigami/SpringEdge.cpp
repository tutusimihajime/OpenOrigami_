#include "SpringEdge.h"

SpringEdge::SpringEdge(Halfedge *he){
	sv = new SpringVertex(he->vertex);
	ev = new SpringVertex(he->next->vertex);
	calculateLength();
	naturalLength = length;
}
SpringEdge::SpringEdge(SpringVertex *_sv, SpringVertex *_ev){
	sv = _sv;
	ev = _ev;
	calculateLength();
	naturalLength = length;
	C = 1;
}

void SpringEdge::calculateLength(){
	length = MyVector3d(sv->v->x - ev->v->x, sv->v->y - ev->v->y, sv->v->z - ev->v->z).length();
}

void SpringEdge::calculateSpringPower(double dt){
	
	calculateLength();

	double dl = length - naturalLength;

	double F_ = C * dl;
	MyVector3d sf = MyVector3d(ev->v->x - sv->v->x, ev->v->y - sv->v->y, ev->v->z - sv->v->z);// sv->ev‚ÉŒü‚©‚¤—Í‚±‚ÌŒü‚«‚ð³‚Æ‚·‚é
	sf.normalize();
	sf.scale(F_);
	sf.scale(dt);
	sv->velocity += sf;
	ev->velocity -= sf;
}
