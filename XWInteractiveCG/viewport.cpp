//-------------------------------------------------------------------------------
///
/// \file viewport.cpp
///	\author Xipeng Wang
/// \version 1.0
/// \date 01/18/2020
///
///	\rendering viewport using openGL & freeglut
///
//-------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//-------------------------------------------------------------------------------

# include <GL/glew.h>

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

#include "cyVector.h";
#include "cyTriMesh.h";
#include "cyGL.h"
using namespace cy;

//-------------------------------------------------------------------------------

Vec3f* bgColor = nullptr;
cyTriMesh* targetObject = nullptr;
char vertexShaderPath[30] = "Data\\VertexShader.glsl";
char fragmentShaderPath[30] = "Data\\FragmentShader.glsl";
GLSLShader* vertexShader;
GLSLShader* fragmentShader;
GLSLProgram* baseProgram;

//-------------------------------------------------------------------------------

GLuint baseVertexArrayObjectID;
GLuint baseVertexBufferID;
GLuint baseIndexBufferID;
GLuint baseNumIndices;

//-------------------------------------------------------------------------------

void GlutDisplay();
void GlutIdle();
void GlutKeyboard(unsigned char key, int x, int y);

//-------------------------------------------------------------------------------

void SendDataToOpenGL(char objName[]);
void InstallShaders();
void CompileShaders();

//-------------------------------------------------------------------------------

void ShowViewport(int argc, char* argv[]) {
	// basic settings for the viewport
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(800, 600);
	glutCreateWindow("XW Renderer - CS6610");
	glutDisplayFunc(GlutDisplay);
	glutIdleFunc(GlutIdle);
	glutKeyboardFunc(GlutKeyboard);
	bgColor = new Vec3f(0, 0, 0);
	glClearColor(bgColor->x, bgColor->y, bgColor->z, 0);
	
	glewInit();

	// prepare data for opengl
	if (argc <= 1) {
		printf("Obj filename missing!\n");
		return;
	}
	else {
		SendDataToOpenGL(argv[1]);
	}

	// install shaders to opengl
	InstallShaders();

	glutMainLoop();
}

//-------------------------------------------------------------------------------

void GlutDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElements(GL_TRIANGLES, baseNumIndices, GL_UNSIGNED_INT, 0);

	glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void GlutIdle() {
	clock_t t;
	t = clock();

	float tFrac = t / 1000.0f;
	//printf("%f\n", tFrac);

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

void SendDataToOpenGL(char objName[]) {
	// read teapot data
	char objPath[30] = "Data\\";
	strcat(objPath, objName);
	
	targetObject = new cyTriMesh();
	if (targetObject->LoadFromFileObj(objPath, false)) {
		printf("Successfully read object %s\n", objName);

		// generate buffer-ready data
		Vec3f* objVertices = new Vec3f[targetObject->NV()];
		for (int i = 0; i < targetObject->NV(); i++) {
			objVertices[i] = targetObject->V(i);
		}

		//GLushort* indices = new GLushort[targetObject->NF() * 3];
		unsigned int* objIndices = new unsigned int[targetObject->NF() * 3];
		for (int i = 0; i < targetObject->NF(); i++) {
			TriMesh::TriFace currentFace = targetObject->F(i);
			objIndices[i * 3] = currentFace.v[0];
			objIndices[i * 3 + 1] = currentFace.v[1];
			objIndices[i * 3 + 2] = currentFace.v[2];
		}
		baseNumIndices = targetObject->NF() * 3;

		// send vertex data & index data to buffers
		glGenBuffers(1, &baseVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, targetObject->NV(), objVertices, GL_STATIC_DRAW);

		glGenBuffers(1, &baseIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, baseNumIndices, objIndices, GL_STATIC_DRAW);

		delete[] objVertices;
		delete[] objIndices;
		
		// setup vertex arrays
		glGenVertexArrays(1, &baseVertexArrayObjectID);
		glBindVertexArray(baseVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
	}
	else {
		return;
	}
}
//-------------------------------------------------------------------------------

void InstallShaders() {
	vertexShader = new GLSLShader();
	fragmentShader = new GLSLShader();
	baseProgram = new GLSLProgram();

	CompileShaders();
}

//-------------------------------------------------------------------------------

void CompileShaders() {
	if (vertexShader == NULL) {
		vertexShader = new GLSLShader();
	}
	if (fragmentShader == NULL) {
		fragmentShader = new GLSLShader();
	}

	if (!vertexShader->CompileFile(vertexShaderPath, GL_VERTEX_SHADER)) {
		return;
	}
	if (!fragmentShader->CompileFile(fragmentShaderPath, GL_FRAGMENT_SHADER)) {
		return;
	}

	baseProgram->CreateProgram();
	baseProgram->AttachShader(vertexShader->GetID());
	baseProgram->AttachShader(fragmentShader->GetID());
	baseProgram->Link();
	baseProgram->Bind();

	vertexShader->Delete();
	fragmentShader->Delete();
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------