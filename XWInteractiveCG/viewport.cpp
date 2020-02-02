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
#include "xwLights.h";
#include "xwHelper.h";

//-------------------------------------------------------------------------------

enum RenderMode {
	XW_RENDER_TRIANGLES,
	XW_RENDER_LINELOOP
};

//-------------------------------------------------------------------------------

char vertexShaderPath[30] = "Data\\VertexShader.glsl";
char fragmentShaderPath[30] = "Data\\FragmentShader.glsl";
GLSLShader* vertexShader;
GLSLShader* fragmentShader;
GLSLProgram* baseProgram;

//-------------------------------------------------------------------------------

char uniformNames[500] = {
	"worldToClampMatrix "
	"objectToWorldMatrix "
	"glossiness "
	"diffuseColor "
	"specularColor "
	"cameraPosition "
	"pointLight0pos "
	"pointLight0Intensity "
};

//-------------------------------------------------------------------------------

GLuint baseVertexArrayObjectID;
GLuint baseVertexBufferID;
GLuint baseVertexNormalBufferID;
GLuint baseIndexBufferID;
GLuint baseNumIndices;

//-------------------------------------------------------------------------------

RenderMode renderMode;

//-------------------------------------------------------------------------------

int mouseStates[3];
int keyStates[256];
int specialKeyStates[128];
int mousePos[2];
int screenSize[2];
int lastTickMousePos[2];

//-------------------------------------------------------------------------------

Vec3f* bgColor = nullptr;

//-------------------------------------------------------------------------------

cyTriMesh* targetObject = nullptr;

Vec3f baseObjectPosition;
Vec3f baseObjectRotation;
Vec3f baseObjectScale;
Vec3f baseObjectCenter;

Vec3f baseObjectColor;
Vec3f baseObjectDiffuseColor;
Vec3f baseObjectSpecularColor;
float baseObjectGlossiness;

//-------------------------------------------------------------------------------

Light ambientLight = Light();
PointLight pointLight0 = PointLight();
DirectionalLight dirLight0 = DirectionalLight();

#define POINTLIGHT0_ROTATION_SPEED 0.004f

//-------------------------------------------------------------------------------

Camera* baseCamera = nullptr;
#define CAMERA_ROTATION_SPEED 0.001f
#define CAMERA_MOVE_SPEED 0.03f

//-------------------------------------------------------------------------------

void GlutDisplay();
void GlutIdle();
void GlutKeyboard(unsigned char key, int x, int y);
void GlutSpecialKey(int key, int x, int y);
void GlutSpecialKeyUp(int key, int x, int y);
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

	baseObjectGlossiness = 20;
	baseObjectDiffuseColor = Vec3f(.3f, .6f, .9f);
	baseObjectSpecularColor = Vec3f(.9f);

	baseObjectColor = Vec3f(0, 0, 0);

	bgColor = new Vec3f(0, 0, 0);

	for (int i = 0; i < 3; i++)
		mouseStates[i] = GLUT_UP;

	mousePos[0] = 0;
	mousePos[1] = 0;

	for (int i = 0; i < 128; i++)
		specialKeyStates[i] = GLUT_UP;

	lastTickMousePos[0] = mousePos[0];
	lastTickMousePos[1] = mousePos[1];

	screenSize[0] = 800;
	screenSize[1] = 800;

	baseCamera = new Camera();
	baseCamera->SetAspect((float)screenSize[0] / (float)screenSize[1]);

	ambientLight.SetIntensity(.15f);
	pointLight0.SetPosition(Vec3f(0, 0, 40));
	pointLight0.SetIntensity(Vec3f(1, .5f, .5f));
	pointLight0.SetRotation(Vec2f(-Pi<float>() / 3, Pi<float>() / 3));

	renderMode = XW_RENDER_TRIANGLES;

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
	glutSpecialUpFunc(GlutSpecialKeyUp);
	glutMouseFunc(GlutMouseClick);
	glutMotionFunc(GlutMouseDrag);
	glClearColor(bgColor->x, bgColor->y, bgColor->z, 0);
	
	glewInit();
	glEnable(GL_DEPTH_TEST);

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
		Matrix4f::RotationXYZ(-Pi<float>() / 2, 0, 0) *
		Matrix4f::Translation(-baseObjectCenter);

	Matrix4f WorldToViewMatrix =
		baseCamera->WorldToViewMatrix();

	Matrix4f worldToClampMatrix = WorldToViewMatrix;
	if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
		Matrix4f viewToProjectionMatrix = baseCamera->ViewToProjectionMatrix();
		worldToClampMatrix = viewToProjectionMatrix * worldToClampMatrix;
	}
	else if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
		worldToClampMatrix.OrthogonalizeZ();
	}

	glBindVertexArray(baseVertexArrayObjectID);

	baseProgram->SetUniformMatrix4("worldToClampMatrix", &worldToClampMatrix.cell[0]);
	baseProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);

	baseProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
	baseProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
	baseProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

	baseProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());
	baseProgram->SetUniform3("ambientLight", 1, ambientLight.GetIntensity().Elements());
	baseProgram->SetUniform3("pointLight0pos", 1, pointLight0.GetPosition().Elements());
	baseProgram->SetUniform3("pointLight0Intensity", 1, pointLight0.GetIntensity().Elements());

	switch (renderMode)
	{
	case XW_RENDER_LINELOOP:
		glDrawElements(GL_LINE_LOOP, baseNumIndices, GL_UNSIGNED_INT, 0);
		break;
	case XW_RENDER_TRIANGLES:
	default:
		glDrawElements(GL_TRIANGLES, baseNumIndices, GL_UNSIGNED_INT, 0);
		break;
	}

	glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void GlutIdle() {
	clock_t t;
	t = clock();

	float tFrac = t / 1000.0f;

	baseObjectColor.x = .5f * sinf(tFrac) + .5f;
	baseObjectColor.y = .5f * cosf(2 * tFrac - 2) + .5f;
	baseObjectColor.z = .5f * sinf(-3 * tFrac + 1) + .5f;

	glutPostRedisplay();
}

//-------------------------------------------------------------------------------

void GlutKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		renderMode = XW_RENDER_TRIANGLES;
		break;
	case '2':
		renderMode = XW_RENDER_LINELOOP;
		break;
	case 'r':
	case 'R':
		baseCamera->Reset();
		break;
	case 'p':
	case 'P':
		baseCamera->SwitchCameraType();
		break;
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
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		specialKeyStates[key] = GLUT_DOWN;
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------

void GlutSpecialKeyUp(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		specialKeyStates[key] = GLUT_UP;
		break;
	default:
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
			if (specialKeyStates[GLUT_KEY_CTRL_L] * specialKeyStates[GLUT_KEY_CTRL_R] == GLUT_DOWN) {
				Vec2f pointLight0Rotate = mouseMove * POINTLIGHT0_ROTATION_SPEED;
				pointLight0.Rotate(pointLight0Rotate);
			}
			else {
				Vec2f cameraRotate = mouseMove * CAMERA_ROTATION_SPEED;
				//baseCamera->RotateCameraByLocal(cameraRotate);
				baseCamera->RotateCameraByOrigin(cameraRotate);
			}
		}
		else if (mouseStates[GLUT_RIGHT_BUTTON] == GLUT_DOWN) {
			float cameraMoveDis = mouseMove.Dot(Vec2f(1, -1)) * CAMERA_MOVE_SPEED;
			//baseCamera->MoveCameraAlongView(cameraMoveDis);
			baseCamera->ScaleDistanceAlongView(cameraMoveDis);
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
		Vec3f* objVertices = new Vec3f[targetObject->NVN()];
		Vec3f* objNormals = new Vec3f[targetObject->NVN()];

		for (int i = 0; i < targetObject->NVN(); i++) {
			objNormals[i] = targetObject->VN(i);
		}

		//GLushort* indices = new GLushort[targetObject->NF() * 3];
		unsigned int* objIndices = new unsigned int[targetObject->NF() * 3];
		for (int i = 0; i < targetObject->NF(); i++) {
			TriMesh::TriFace currentNormalFace = targetObject->FN(i);
			unsigned int v0 = currentNormalFace.v[0];
			unsigned int v1 = currentNormalFace.v[1];
			unsigned int v2 = currentNormalFace.v[2];
			objIndices[i * 3] = v0;
			objIndices[i * 3 + 1] = v1;
			objIndices[i * 3 + 2] = v2;
			objVertices[v0] = targetObject->GetVec(i, Vec3f(1, 0, 0));
			objVertices[v1] = targetObject->GetVec(i, Vec3f(0, 1, 0));
			objVertices[v2] = targetObject->GetVec(i, Vec3f(0, 0, 1));
		}
		baseNumIndices = targetObject->NF() * 3;

		// send vertex data & index data to buffers
		glGenBuffers(1, &baseVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, targetObject->NVN() * sizeof(Vec3f), objVertices, GL_STATIC_DRAW);

		glGenBuffers(1, &baseVertexNormalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexNormalBufferID);
		glBufferData(GL_ARRAY_BUFFER, targetObject->NVN() * sizeof(Vec3f), objNormals, GL_STATIC_DRAW);

		glGenBuffers(1, &baseIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, baseNumIndices * sizeof(unsigned int), objIndices, GL_STATIC_DRAW);

		delete[] objVertices;
		delete[] objIndices;
		
		// setup vertex arrays
		glGenVertexArrays(1, &baseVertexArrayObjectID);
		glBindVertexArray(baseVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexNormalBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);
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

	baseProgram->RegisterUniforms(uniformNames);
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