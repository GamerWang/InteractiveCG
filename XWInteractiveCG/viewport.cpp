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
#include "cyMatrix.h";
using namespace cy;

//-------------------------------------------------------------------------------

#include "xwCamera.h";

//-------------------------------------------------------------------------------

char vertexShaderPath[30] = "Data\\VertexShader.glsl";
char fragmentShaderPath[30] = "Data\\FragmentShader.glsl";
GLSLShader* vertexShader;
GLSLShader* fragmentShader;
GLSLProgram* baseProgram;
GLuint objectToClampMatrixUniformLocation;
GLuint dynamicColorUniformLocation;

//-------------------------------------------------------------------------------

GLuint baseVertexArrayObjectID;
GLuint baseVertexBufferID;
GLuint baseIndexBufferID;
GLuint baseNumIndices;

//-------------------------------------------------------------------------------

int mouseStates[3];
int mousePos[2];
int screenSize[2];
int lastTickMousePos[2];

//-------------------------------------------------------------------------------

Vec3f* bgColor = nullptr;
cyTriMesh* targetObject = nullptr;

Vec3f baseObjectPosition;
Vec3f baseObjectRotation;
Vec3f baseObjectScale;
Vec3f baseObjectCenter;

Vec3f baseObjectColor;

Camera* baseCamera = nullptr;
#define CAMERA_ROTATION_SPEED 0.001f
#define CAMERA_MOVE_SPEED 0.03f

//-------------------------------------------------------------------------------

void GlutDisplay();
void GlutIdle();
void GlutKeyboard(unsigned char key, int x, int y);
void GlutSpecialKey(int key, int x, int y);
void GlutMouseClick(int button, int state, int x, int y);
void GlutMouseDrag(int x, int y);
void GlutReshapeWindow(int width, int height);

//-------------------------------------------------------------------------------

void SendDataToOpenGL(char objName[]);
void InstallShaders();
void CompileShaders();

//-------------------------------------------------------------------------------

bool IsMouseDown() { return !(mouseStates[0] * mouseStates[1] * mouseStates[2]); }

//-------------------------------------------------------------------------------

void ShowViewport(int argc, char* argv[]) {
	// initialize scene data
	baseObjectPosition = Vec3f(0, 0, 0);
	baseObjectRotation = Vec3f(0, 0, 0);
	baseObjectScale = Vec3f(1, 1, 1);

	baseObjectColor = Vec3f(0, 0, 0);

	bgColor = new Vec3f(0, 0, 0);

	for (int i = 0; i < 3; i++)
		mouseStates[i] = GLUT_UP;

	mousePos[0] = 0;
	mousePos[1] = 0;

	lastTickMousePos[0] = mousePos[0];
	lastTickMousePos[1] = mousePos[1];

	screenSize[0] = 800;
	screenSize[1] = 800;

	baseCamera = new Camera();
	baseCamera->SetAspect((float)screenSize[0] / (float)screenSize[1]);

	// basic settings for the viewport
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(screenSize[0], screenSize[1]);
	glutCreateWindow("XW Renderer - CS6610");
	glutDisplayFunc(GlutDisplay);
	glutIdleFunc(GlutIdle);
	//glutReshapeFunc(GlutReshapeWindow);
	glutKeyboardFunc(GlutKeyboard);
	glutSpecialFunc(GlutSpecialKey);
	glutMouseFunc(GlutMouseClick);
	glutMotionFunc(GlutMouseDrag);
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

	if (!targetObject->IsBoundBoxReady()) {
		targetObject->ComputeBoundingBox();
	}
	baseObjectCenter = targetObject->GetBoundMax() + targetObject->GetBoundMin();
	baseObjectCenter /= 2;

	// install shaders to opengl
	InstallShaders();

	glutMainLoop();
}

//-------------------------------------------------------------------------------

void GlutDisplay() {
	// compute matrices here

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// send uniforms here
	Matrix4f objectToWorldMatrix =
		Matrix4f::Translation(baseObjectPosition) *
		Matrix4f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) * 
		Matrix4f::Scale(baseObjectScale) *
		Matrix4f::Translation(-baseObjectCenter);

	Matrix4f WorldToViewMatrix =
		baseCamera->WorldToViewMatrix();

	Matrix4f objectToClampMatrix = WorldToViewMatrix * objectToWorldMatrix;
	if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
		Matrix4f viewToProjectionMatrix = baseCamera->ViewToProjectionMatrix();
		objectToClampMatrix = viewToProjectionMatrix * objectToClampMatrix;
	}
	else if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
		objectToClampMatrix.OrthogonalizeZ();
	}

	glBindVertexArray(baseVertexArrayObjectID);
	glUniformMatrix4fv(objectToClampMatrixUniformLocation, 1, GL_FALSE, &objectToClampMatrix.cell[0]);
	glUniform3fv(dynamicColorUniformLocation, 1, baseObjectColor.Elements());
	glDrawElements(GL_TRIANGLES, baseNumIndices, GL_UNSIGNED_INT, 0);

	glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void GlutIdle() {
	clock_t t;
	t = clock();

	float tFrac = t / 1000.0f;
	baseObjectRotation = Vec3f(-Pi<float>() / 2, tFrac / 10, 0);

	baseObjectColor.x = .5f * sinf(tFrac) + .5f;
	baseObjectColor.y = .5f * cosf(2 * tFrac - 2) + .5f;
	baseObjectColor.z = .5f * sinf(-3 * tFrac + 1) + .5f;

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

void GlutSpecialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_F6:
		printf("Recompile shaders\n");
		CompileShaders();
		break;
	}
}

//-------------------------------------------------------------------------------

void GlutMouseClick(int button, int state, int x, int y) {
	mouseStates[button] = state;
	if (state == GLUT_DOWN) {
		lastTickMousePos[0] = x;
		lastTickMousePos[1] = y;
	}
	mousePos[0] = x;
	mousePos[1] = y;
}

//-------------------------------------------------------------------------------

void GlutMouseDrag(int x, int y) {
	if (IsMouseDown()) {
		Vec2f mouseMove = Vec2f(x - lastTickMousePos[0], y - lastTickMousePos[1]);
		mouseMove.y *= -1;
		if (mouseStates[GLUT_LEFT_BUTTON] == GLUT_DOWN) {
			Vec2f cameraRotate = mouseMove * CAMERA_ROTATION_SPEED;
			baseCamera->RotateCameraByLocal(cameraRotate);
		}
		else if (mouseStates[GLUT_RIGHT_BUTTON] == GLUT_DOWN) {
			float cameraMoveDis = mouseMove.Dot(Vec2f(1, -1)) * CAMERA_MOVE_SPEED;
			baseCamera->MoveCameraAlongView(cameraMoveDis);
		}
		else if (mouseStates[GLUT_MIDDLE_BUTTON] == GLUT_DOWN) {

		}
		lastTickMousePos[0] = mousePos[0];
		lastTickMousePos[1] = mousePos[1];
	}
	mousePos[0] = x;
	mousePos[1] = y;
}

//-------------------------------------------------------------------------------

void GlutReshapeWindow(int width, int height) {
}

//-------------------------------------------------------------------------------

void SendDataToOpenGL(char objName[]) {
	// read teapot data
	char objPath[30] = "Data\\";
	strcat(objPath, objName);
	
	targetObject = new cyTriMesh();
	if (targetObject->LoadFromFileObj(objPath, false)) {

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
		glBufferData(GL_ARRAY_BUFFER, targetObject->NV() * sizeof(Vec3f), objVertices, GL_STATIC_DRAW);

		glGenBuffers(1, &baseIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, baseNumIndices * sizeof(unsigned int), objIndices, GL_STATIC_DRAW);

		delete[] objVertices;
		delete[] objIndices;
		
		// setup vertex arrays
		glGenVertexArrays(1, &baseVertexArrayObjectID);
		glBindVertexArray(baseVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);
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

	objectToClampMatrixUniformLocation = glGetUniformLocation(baseProgram->GetID(), "objectToClampMatrix");
	dynamicColorUniformLocation = glGetUniformLocation(baseProgram->GetID(), "dynamicColor");
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