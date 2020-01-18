#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "cyVector.h";
using namespace cy;

#ifdef USE_GLUT
# ifdef __APPLE__
# include <GLUT/glut.h>
# else
# include <GL/glut.h>
# endif
#else
# include <GL/freeglut.h>
#endif

//-------------------------------------------------------------------------------

Vec3f* bgColor = nullptr;

//-------------------------------------------------------------------------------

void GlutDisplay();
void GlutIdle();
void GlutKeyboard(unsigned char key, int x, int y);

//-------------------------------------------------------------------------------

void ShowViewport() {
	int argc = 1;
	char argstr[] = "canvas";
	char* argv = argstr;
	glutInit(&argc, &argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(800, 600);
	
	glutCreateWindow("XW Renderer - CS6610");
	glutDisplayFunc(GlutDisplay);
	glutIdleFunc(GlutIdle);
	glutKeyboardFunc(GlutKeyboard);

	bgColor = new Vec3f(.985f, .278f, .157f);
	glClearColor(bgColor->x, bgColor->y, bgColor->z, 0);

	glutMainLoop();
}

//-------------------------------------------------------------------------------

void GlutDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void GlutIdle() {
	clock_t t;
	t = clock();

	float tFrac = t / 1000.0f;
	//printf("%f\n", tFrac);
	bgColor->x = .5f * sinf(tFrac) + .5f;
	bgColor->y = .5f * cosf(2 * tFrac - 2) + .5f;
	bgColor->z = .5f * sinf(-3 * tFrac + 1) + .5f;

	glClearColor(bgColor->x, bgColor->y, bgColor->z, 0);
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------

void GlutKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------