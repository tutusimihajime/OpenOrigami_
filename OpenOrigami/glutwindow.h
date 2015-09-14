#include <gl/freeglut.h>
#include "Model.h"

void glutReady(int argc, char **argv);
void myinit();
void reshape(int w, int h);
void display(void);
void key_input(unsigned char key, int x, int y);
void special_input(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void MouseWheel(int wheel_number, int direction, int x, int y);
void Idle();
