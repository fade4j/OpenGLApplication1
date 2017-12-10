#ifndef IOSTREAM
#include <iostream>
#endif // !IOSTREAM

#ifndef GLEW
#include "GL\glew.h"
#endif // !GLEW

#ifndef GLUT
#include "GL\glut.h"
#endif // !GLUT


using namespace std;
static void initGlut(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	//glutInitContextVersion(4, 3);
	//glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitWindowPosition(540, 200);
	glutCreateWindow("OpenGL");
	//glutShowWindow();
	if (glewInit())
	{
		cerr << "Unable to initialize GLEW..." << endl;
		exit(EXIT_FAILURE);
	}

	//init();
	
}

static void startLoop(void(*callback)()) {
	glutDisplayFunc(callback);
	glutMainLoop();
}