#include <Windows.h>
#include <gl/freeglut.h>
#include <iostream>
#include <Vcclr.h>
#include "Model.h"
#include "SpringSimulator.h"
#include "glutwindow.h"
#include "ReconstructingModel.h"

using namespace std;
using namespace System;
using namespace System::Windows::Forms;
SpringSimulator *spring_sim = NULL;
Model *model;
[STAThreadAttribute]
void main(int argc, char *argv[]){
	if (argc != 2){
		System::Windows::Forms::OpenFileDialog^ ofd = gcnew OpenFileDialog;
		
		if (ofd->ShowDialog() == DialogResult::OK){
			using namespace Runtime::InteropServices;
			model = new Model((const char*)(Marshal::StringToHGlobalAnsi(ofd->FileName)).ToPointer());
		}else{
			exit(1);
		}
	}else{
		model = new Model(argv[1]);
	}

	//testing
	model->deleteGarbageSubface();
	cout << "face " << model->faces.size() << endl;
	cout << "subface " << model->subfaceVector.size() << " : " << endl;
	
	model->checkOverlapSubface();

	//GLUT ON STAGE
	glutReady(argc, argv);
}