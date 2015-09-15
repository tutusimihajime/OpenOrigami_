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
SpringSimulator *spring_sim;
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
	reconstructModel(model);
	
	spring_sim = new SpringSimulator(model);
	//spring_sim->debugPrint();
	//model->debugPrint();
	glutReady(argc, argv);
}