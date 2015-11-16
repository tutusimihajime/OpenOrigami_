#include "glutwindow.h"
#include "SpringSimulator.h"
#include "Picking.h"
#include "ReconstructingModel.h"
#include <iostream>

extern Model *model;
extern SpringSimulator *spring_sim;
//select
extern int select;
//window size
int wx = 800, wy = 600;
//ズーム
MyVector3d zoom(0, 0, -500);
//回転用
#define pi  3.1415926
double scl = 1.0 / (double)512;              // マウスの絶対位置→ウィンドウ内での相対位置の換算係数
double cq[4] = { 1.0, 0.0, 0.0, 0.0 };  // 回転の初期値 (クォータニオン)
double tq[4];              // ドラッグ中の回転 (クォータニオン)
double rt[16];    // 回転の変換行列
//マウスドラッグ用ベクトル
MyVector3d cv, tv, sv, tmp_v;//cv:ドラッグ開始位置tv:平行移動行列用,sv:これまでの遷移を記録

//シミュレーション
bool sim_flag = false;

//test用
bool isDrawSubface = false;
bool isDrawSubFaceGroups = false;

//-------------------------

// クォータニオンの積 r <- p x q
static void qmul(double r[], const double p[], const double q[])
{
	r[0] = p[0] * q[0] - p[1] * q[1] - p[2] * q[2] - p[3] * q[3];
	r[1] = p[0] * q[1] + p[1] * q[0] + p[2] * q[3] - p[3] * q[2];
	r[2] = p[0] * q[2] - p[1] * q[3] + p[2] * q[0] + p[3] * q[1];
	r[3] = p[0] * q[3] + p[1] * q[2] - p[2] * q[1] + p[3] * q[0];
}
// 回転の変換行列 r <- クォータニオン q
void qrot(double r[], double q[]){
	double x2 = q[1] * q[1] * 2.0;
	double y2 = q[2] * q[2] * 2.0;
	double z2 = q[3] * q[3] * 2.0;
	double xy = q[1] * q[2] * 2.0;
	double yz = q[2] * q[3] * 2.0;
	double zx = q[3] * q[1] * 2.0;
	double xw = q[1] * q[0] * 2.0;
	double yw = q[2] * q[0] * 2.0;
	double zw = q[3] * q[0] * 2.0;

	r[0] = 1.0 - y2 - z2;
	r[1] = xy + zw;
	r[2] = zx - yw;
	r[4] = xy - zw;
	r[5] = 1.0 - z2 - x2;
	r[6] = yz + xw;
	r[8] = zx + yw;
	r[9] = yz - xw;
	r[10] = 1.0 - x2 - y2;
	r[3] = r[7] = r[11] = r[12] = r[13] = r[14] = 0.0;
	r[15] = 1.0;
}
//メジャー描画
void Line3D(float x1, float y1, float z1, float x2, float y2, float z2){
	//線幅
	glLineWidth(1.0);
	//線
	glBegin(GL_LINES);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}
void DrawMeasure(int measure, float size){
	glDisable(GL_LIGHTING);
	glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
	for (int x = 0; x <= measure; x++){ Line3D(x*size - (size*measure / 2), 0, -(size*measure / 2), x*size - (size*measure / 2), 0, measure*size - (size*measure / 2)); }
	for (int y = 0; y <= measure; y++){ Line3D(-(size*measure / 2), 0, y*size - (size*measure / 2), measure*size - (size*measure / 2), 0, y*size - (size*measure / 2)); }
	glDisable(GL_DEPTH_TEST);
	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	Line3D(0, 0, 0, (measure / 2 + 2)*size, 0, 0);
	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
	Line3D(0, 0, 0, 0, (measure / 2 + 2)*size, 0);
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	Line3D(0, 0, 0, 0, 0, (measure / 2 + 2)*size);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}
//いつもの
void glutReady(int argc,  char *argv[]){
	glutInit(&argc, argv);
	// ダブルバッファ、画素毎に色指定、デプスバッファを使うモードを指定
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow(argv[0]);
	myinit();
	glutDisplayFunc(display); // 表示
	glutReshapeFunc(reshape); // 再描画
	glutMouseFunc(mouse);//マウスクリック
	glutSpecialFunc(special_input);//矢印 
	glutKeyboardFunc(key_input);//キー
	glutMouseWheelFunc(MouseWheel); //マウスホイール
	glutPostRedisplay();
	glutIdleFunc(Idle);
	glutMainLoop();
}
void myinit() //初期化
{
	// 回転行列の初期化
	qrot(rt, cq);
	//平行移動ベクトルの初期化
	cv.set(0, 0, 0);
	tv = sv = cv;
	float light_pos0[] = { -5., 10.0, 50.0, 0. }; // 光源位置
	float ambient[] = { 0.2, 0.2, 0.2, 1.0 };             // 環境光の色と強さ

	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DOUBLE | GLUT_RGB);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // 表裏両面を照らす
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);    // 光源位置を指定
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);        // 環境光を指定
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 背景色で初期化、奥行き情報初期化
	glClearColor(.6, .6, .6, 0.);   // 消去色（背景色）
	glEnable(GL_DEPTH_TEST);     // 奥行き深さ情報を更新する
	glEnable(GL_NORMALIZE);      // 面を描く時に面の法線データを正規化する
	glEnable(GL_LIGHTING);       // 光源による照明を有効化する
	glEnable(GL_LIGHT0);         // 0番目の光源を有効にする
	//glEnable(GL_CULL_FACE);

}
void reshape(int w, int h)        // 再描画関数
{
	wx = w; wy = h;
	h = (h == 0) ? 1 : h;
	glViewport(0, 0, w, h);   // 画面の表示領域を指定（全画面を使用可にする）

	glMatrixMode(GL_PROJECTION);  // 以降は投影変換行列を操作する

	glLoadIdentity();             // 単位行列を設定
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 10000.0); // 透視投影変換行列を掛ける
	glTranslatef(0., 0., zoom.z);   // 平行移動行列を掛ける

	glMatrixMode(GL_MODELVIEW);   // 以降はモデリング視野変換行列を操作する
}
void display(void)
{
	glMatrixMode(GL_PROJECTION);  // 以降は投影変換行列を操作する

	glLoadIdentity();             // 単位行列を設定
	gluPerspective(60.0, wx/(float)wy, 1.0, 10000.0); // 透視投影変換行列を掛ける
	glTranslatef(0., 0., zoom.z);   // 平行移動行列を掛ける

	glMatrixMode(GL_MODELVIEW);   // 以降はモデリング視野変換行列を操作する

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 画面と奥行き情報を初期化
	// 視点位置の計算
	glLoadIdentity();        // 単位行列をモデルリング視野変換行列にセットする
	
	glTranslated(tv.x, -tv.y, 0); //平行移動
	
	glMultMatrixd(rt);//回転

	//モデルの拡大/縮小
	glScalef(1, 1, 1);    // 拡大
	//モデルの描画

	glEnable(GL_LINE_SMOOTH);
	DrawMeasure(16, 40);
	glDisable(GL_LINE_SMOOTH);
	if (isDrawSubFaceGroups){
		model->drawSubFaceGroups();
		model->drawSubFaceGroupBridges();
	}else if (isDrawSubface){
		model->drawSubFaces();
	}else{
		if (spring_sim == NULL)model->draw();
		if (spring_sim != NULL)spring_sim->draw();
	}
	glutSwapBuffers();              // 表示用の画面領域と書きこみ用画面領域を交換して表示する
}
void key_input(unsigned char key, int x, int y)
{
	
	switch (key){
	case 'q':
		if (select != -1){
			spring_sim->svertexVector[select]->mobility = !spring_sim->svertexVector[select]->mobility;
		}
		break;
	case 's':
		if (spring_sim == NULL){
			reconstructModel(model);
			spring_sim = new SpringSimulator(model);
		}
		break;
	case 'v':
		spring_sim->v_flag = !spring_sim->v_flag;
		break;
	case 'e':
		spring_sim->e_flag = !spring_sim->e_flag;
		break;
	case 'f':
		spring_sim->f_flag = !spring_sim->f_flag;
		break;
	case 't':
		spring_sim->f_type++;
		if (spring_sim->f_type > 2)spring_sim->f_type = 0;
		break;
	case 'p':
		isDrawSubface = !isDrawSubface;
		break;
	case 'g':
		isDrawSubFaceGroups = !isDrawSubFaceGroups;
		break;
	case'W':
		model->exportOBJ();
		break;
	case 27:
		exit(0);
		break;
	case ' ':
		sim_flag = !sim_flag;
	default:
		break;
	}
}
void special_input(int key, int x, int y)
{
	switch (key){
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	}
}

MyVector3d rotateMyVector3d(MyVector3d vec){
	return MyVector3d(rt[0] * vec.x + rt[1] * vec.y + rt[2] * vec.z,
		rt[4] * vec.x + rt[5] * vec.y + rt[6] * vec.z,
		rt[8] * vec.x + rt[9] * vec.y + rt[10] * vec.z);
}
//NEED TO FIX 
void mouse_motion_left(int x, int y){
	if (select != -1){
		MyVector3d dv(x, y, 0);
		dv -= cv;
		dv.scale(scl);
		dv.scale(fabs(zoom.z));
		dv.y *= -1;
		dv = rotateMyVector3d(dv);		
		model->vertexVector[select]->transPosition(dv);
		cv.set(x, y, 0);
	}
}
void mouse_motion_middle(int x, int y){
	MyVector3d dv(x, y, 0);
	dv -= cv;
	tv = dv + sv;
	tv.scale(1);
}
void mouse_motion_right(int x, int y)
{
	//ドラッグ開始位置からの現在の位置へのベクトル
	MyVector3d dv(x, y, 0);
	dv -= cv;
	dv.scale(scl);
	double len = dv.length();
	if (len != 0.){
		// マウスのドラッグに伴う回転のクォータニオン dq を求める
		double dq[4] = { cos(len*pi), dv.y*sin(len*pi) / len, dv.x*sin(len*pi) / len, 0. };

		// 回転の初期値 cq に dq を掛けて回転を合成
		qmul(tq, dq, cq);

		// クォータニオンから回転の変換行列を求める
		qrot(rt, tq);
	}
	glutPostRedisplay(); //再描画
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			if (spring_sim != NULL){
				pick(x, y);
				cv.set(x, y, 0);
			}
			glutMotionFunc(mouse_motion_left);
			break;
		case GLUT_UP:
			glutMotionFunc(NULL);
			break;
		default:
			break;
		}
		
		break;
	case GLUT_MIDDLE_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			// ドラッグ開始位置ベクトル(画面上)を記録
			cv.set(x, y, 0);
			glutMotionFunc(mouse_motion_middle);//マウスドラッグ
			break;
		case GLUT_UP:
			tmp_v.set(x, y, 0);
			sv += tmp_v - cv;
			glutMotionFunc(NULL);
			break;
		default:
			break;
		}
		
		break;
	case GLUT_RIGHT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			// ドラッグ開始位置ベクトル(画面上)を記録
			cv.set(x, y, 0);
			glutMotionFunc(mouse_motion_right);
			break;
		case GLUT_UP:
			// 回転の保存
			cq[0] = tq[0];
			cq[1] = tq[1];
			cq[2] = tq[2];
			cq[3] = tq[3];
			glutMotionFunc(NULL);
			break;
		default:
			break;
		}break;
		break;
	default:
		break;
	}

}

void Idle(){
	if(sim_flag)spring_sim->simulate(0.5);
	model->calcNormal();
	Sleep(0.001);
	glutPostRedisplay();
}

void MouseWheel(int wheel_number, int direction, int x, int y){
	if (direction == 1){
		zoom += MyVector3d(0,0,10);
	} else{
		zoom -= MyVector3d(0, 0, 10);
	}
}