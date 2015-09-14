#pragma once
#include <gl/freeglut.h>

void draw(GLenum mode);

void setOrtho(int w, int h);
void reshape(int w, int h);
int selectHits(GLint hits, const GLuint *buffer);
void pick(int x, int y);
void init(void);
