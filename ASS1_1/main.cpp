#include <iostream>
#include <fstream>

#include "GL/glew.h"
#include "GL/glut.h"
#include "mesh.h"
#include "Camera.h"
#include "Scene.h"

using namespace std;

Scene *scene;

void display() {
	scene->renderScene();
	glFlush();
}

void reshape(int w, int h) {
	scene->reshapeWindow(w, h);
}

int isPressed = 0;
int lastX, lastY;
void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		isPressed = 1;
		lastX = x;
		lastY = y;
	}
	else {
		isPressed = 0;
	}
}

void motion(int x, int y) {
	if (isPressed) {
		scene->panObject(x - lastX, lastY - y);
		lastX = x;
		lastY = y;
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case 'n':
		scene->toggleShowNormal();
		break;
	case 'f':
		scene->addLightY(-100);
		break;
	case 'r':
		scene->addLightY(100);
		break;
	case 'w':
		scene->zoomIn(1);
		break;
	case 'a':
		scene->moveLeft(1);
		break;
	case 's':
		scene->zoomIn(-1);
		break;
	case 'd':
		scene->moveLeft(-1);
		break;
	case '1':
		scene->setFocus(-10, 12, 0);
		break;
	case '2':
		scene->setFocus(-50, 12, 0);
		break;
	case '3':
		scene->setFocus(-400, 12, 0);
		break;
	}

	if (key >= '1' && key <= '9')
		scene->setCurrentObjectIndex(key - '1');

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH | GLUT_ACCUM);
	glutCreateWindow("Assignment 2, Jeff Chang, 0316314");

	if (glewInit() != GLEW_OK) {
		cout << "GLEW: failed to init" << endl;
		return 0;
	}

	//scene = new Scene("scene.scene", "light.light", "view.view");
	scene = new Scene("as3.scene", "as3.light", "as3.view");
	glutReshapeWindow(scene->getWindowWidth(), scene->getWindowHeight());

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	delete scene;
	return 0;
}