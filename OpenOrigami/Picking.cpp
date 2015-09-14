#include "Picking.h"
#include <cstdio>
#include <iostream>
#include "SpringSimulator.h"
#define SELECT_BUFFER_SIZE 512

int select = -1;
//window size
extern int wx, wy;
//ズーム
extern MyVector3d zoom;

extern SpringSimulator *spring_sim;

void draw(GLenum mode)
{
	glDisable(GL_LIGHTING);
	if (mode == GL_SELECT) glLoadName(1);

	if (select == 1) glColor3f(1., 0., 0.);
	else glColor3f(0.5, 0.5, 0.5);

	glPushMatrix();
	glTranslatef(-30, -10, 10.);
	glBegin(GL_TRIANGLES);
	glVertex2f(-50, -50);
	glVertex2f(50, -50);
	glVertex2f(0., 50);
	glEnd();
	glPopMatrix();

	if (mode == GL_SELECT) glLoadName(2);

	if (select == 2) glColor3f(1., 0., 0.);
	else glColor3f(0.3, 0.3, 0.3);

	glPushMatrix();
	glTranslatef(30, 10, 0.);
	glBegin(GL_QUADS);
	glVertex2f(-50, -50);
	glVertex2f(50, -50);
	glVertex2f(50, 50);
	glVertex2f(-50, 50);
	glEnd();
	glPopMatrix();
}

int selectHits(GLint hits, const GLuint *buffer)
{
	int i;
	const GLuint *p;
	GLuint names;
	GLuint depth1;
	GLuint depth2;
	GLuint depth = -1;
	int select = -1;

	if (hits == 0) return -1;

	p = buffer;

	for (i = 0; i < hits; i++){
		names  = *p;
		depth1 = *(p+1);
		depth2 = *(p+2);

		if (depth > depth1){
			depth = depth1;
			select = *(p+3);

		}
		p += 4;
	}

	return select;
}

void pick(int x, int y)
{
	GLuint selectBuffer[SELECT_BUFFER_SIZE];
	GLint hits;
	GLint vp[4];

	glGetIntegerv(GL_VIEWPORT, vp);

	glSelectBuffer(SELECT_BUFFER_SIZE, selectBuffer);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();             // 単位行列を設定
	gluPickMatrix(x, vp[3] - y, 10., 10., vp);
	gluPerspective(60.0, wx / (float)wy, 1.0, 10000.0); // 透視投影変換行列を掛ける
	glTranslatef(0., 0., zoom.z);   // 平行移動行列を掛ける

	glMatrixMode(GL_MODELVIEW);
	//ここにモデルを描画
	//draw(GL_SELECT);
	spring_sim->draw(GL_SELECT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	hits = glRenderMode(GL_RENDER);

	//printf("hits = %d\n", hits);

	select = selectHits(hits, selectBuffer);

	//cout << "select = " << select << endl;
}



