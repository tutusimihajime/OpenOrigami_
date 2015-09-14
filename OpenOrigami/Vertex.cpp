#include "Vertex.h"
#include <iostream>
#include <gl/freeglut.h>

#define NULL 0
using namespace std;
Vertex::Vertex()
{
	//num_of_vertex++;
}
Vertex::Vertex(double _x, double _y, double _z) {
	x = _x;
	y = _y;
	z = _z;
	halfedge = NULL;
	//id = num_of_vertex + 1;
}
void Vertex::setID(int n){
	id = n;
}
void Vertex::draw(){
	glBegin(GL_POINTS);
	glVertex3d(x, y, z);
	glEnd();
}
Vertex::~Vertex()
{
	//num_of_vertex--;
}
void Vertex::debugPrint(){
	cout << "v : " << id << "( " << x << ", " << y << ", " << z << ")" << endl;
}