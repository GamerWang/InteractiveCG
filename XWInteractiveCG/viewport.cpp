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
#include "xwMaterial.h";
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
GLRenderTexture2D* baseSceneBuffer;

char RTextureVertShaderPath[30] = "Data\\SV_Plane.glsl";
char RTextureFragShaderPath[30] = "Data\\SF_Plane.glsl";
GLSLProgram* RTextureProgram;

//-------------------------------------------------------------------------------

char uniformNames[500] = {
	"worldToClampMatrix"
	" objectToWorldMatrix"
	" objectNormalToWorldMatrix"
	" glossiness"
	" diffuseColor"
	" specularColor"
	" cameraPosition"
	" pointLight0pos"
	" pointLight0Intensity"
	" diffuseTexture"
};

char planeSceneUniformNames[500] = {
	"objectToClampMatrix"
};

//-------------------------------------------------------------------------------

GLuint baseVertexArrayObjectID;
GLuint baseVertexBufferID;
GLuint baseIndexBufferID;
GLuint baseNumIndices;

GLuint RTextureVertexArrayObjectID;
GLuint RTexturePlaneVertexBufferID;
GLuint RTexturePlaneIndexBufferID;

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

//-------------------------------------------------------------------------------

Material* baseObjectMaterial;
cyGLTexture2D* baseObjectDiffuse;
cyGLTexture2D* baseObjectSpecular;
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
Camera* planeSceneCamera = nullptr;
#define CAMERA_ROTATION_SPEED 0.001f
#define CAMERA_MOVE_SPEED 0.03f

//-------------------------------------------------------------------------------||TEMP variables
GLuint framebuffer;
GLuint textureColorBuffer;
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
void InstallTeapotSceneShaders();
void CompileTeapotSceneShaders();
void InstallPlaneSceneShaders();

//-------------------------------------------------------------------------------

bool IsMouseDown() { return !(mouseStates[0] * mouseStates[1] * mouseStates[2]); }

//-------------------------------------------------------------------------------

void ShowViewport(int argc, char* argv[]) {
	// initialize scene data
	baseObjectPosition = Vec3f(0, 0, 0);
	baseObjectRotation = Vec3f(0, 0, 0);
	baseObjectScale = Vec3f(1.0f, 1.0f, 1.0f);

	baseObjectMaterial = new Material();
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

	planeSceneCamera = new Camera();
	planeSceneCamera->SetAspect((float)screenSize[0] / (float)screenSize[1]);
	planeSceneCamera->SetPosition(Vec3f(0, 0, 3));

	ambientLight.SetIntensity(.15f);
	pointLight0.SetPosition(Vec3f(0, 0, 40));
	pointLight0.SetIntensity(Vec3f(.7f));
	pointLight0.SetRotation(Vec2f(-Pi<float>() / 3, Pi<float>() / 3));

	renderMode = XW_RENDER_TRIANGLES;

	// basic settings for the viewport
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(800, 50);
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

	// initialize render texture buffer
	baseSceneBuffer = new GLRenderTexture2D();
	baseSceneBuffer->Initialize(true, 3, 1024, 1024);
	baseSceneBuffer->SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
	baseSceneBuffer->BuildTextureMipmaps();

	// install shaders to opengl
	InstallTeapotSceneShaders();
	InstallPlaneSceneShaders();

	glutMainLoop();
} 

//-------------------------------------------------------------------------------

void GlutDisplay() {
	// rendering the teapot scene to target buffer
	baseSceneBuffer->Bind();

	// compute matrices here
	glClearColor(.5f, .1f, .1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// send uniforms here
	Matrix4f objectToWorldMatrix =
		Matrix4f::Translation(baseObjectPosition) *
		Matrix4f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) *
		Matrix4f::Scale(baseObjectScale) *
		Matrix4f::RotationXYZ(-Pi<float>() / 2, 0, 0) *
		Matrix4f::Translation(-baseObjectCenter);

	Matrix3f objectNormalToWorldMatrix = 
		Matrix3f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) *
		Matrix3f::Scale(baseObjectScale).GetInverse() *
		Matrix3f::RotationXYZ(-Pi<float>() / 2, 0, 0);

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
	baseProgram->Bind();

	baseProgram->SetUniformMatrix4("worldToClampMatrix", &worldToClampMatrix.cell[0]);
	baseProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
	baseProgram->SetUniformMatrix3("objectNormalToWorldMatrix", &objectNormalToWorldMatrix.cell[0]);

	baseProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
	baseProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
	baseProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

	baseProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());
	baseProgram->SetUniform3("ambientLight", 1, ambientLight.GetIntensity().Elements());
	baseProgram->SetUniform3("pointLight0pos", 1, pointLight0.GetPosition().Elements());
	baseProgram->SetUniform3("pointLight0Intensity", 1, pointLight0.GetIntensity().Elements());

	baseObjectDiffuse->Bind(0);
	baseObjectSpecular->Bind(1);

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
	baseSceneBuffer->Unbind();

	Matrix4f PlaneSceneWorldToViewMatrix =
		planeSceneCamera->WorldToViewMatrix();
	if (planeSceneCamera->GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
		Matrix4f viewToProjectionMatrix = planeSceneCamera->ViewToProjectionMatrix();
		PlaneSceneWorldToViewMatrix = viewToProjectionMatrix * PlaneSceneWorldToViewMatrix;
	}
	else if (planeSceneCamera->GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
		PlaneSceneWorldToViewMatrix.OrthogonalizeZ();
	}

	glClearColor(.5f, .5f, .5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RTextureProgram->Bind();
	glBindVertexArray(RTextureVertexArrayObjectID);
	baseSceneBuffer->BindTexture(2);

	RTextureProgram->SetUniformMatrix4("objectToClampMatrix", &PlaneSceneWorldToViewMatrix.cell[0]);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
		CompileTeapotSceneShaders();
		break;
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
	case GLUT_KEY_ALT_L:
	case GLUT_KEY_ALT_R:
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
	case GLUT_KEY_ALT_L:
	case GLUT_KEY_ALT_R:
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
			else if (specialKeyStates[GLUT_KEY_ALT_L] * specialKeyStates[GLUT_KEY_ALT_R] == GLUT_DOWN) {
				Vec2f cameraRotate = mouseMove * CAMERA_ROTATION_SPEED;
				planeSceneCamera->RotateCameraByOrigin(cameraRotate);
			}
			else {
				Vec2f cameraRotate = mouseMove * CAMERA_ROTATION_SPEED;
				//baseCamera->RotateCameraByLocal(cameraRotate);
				baseCamera->RotateCameraByOrigin(cameraRotate);
			}
		}
		else if (mouseStates[GLUT_RIGHT_BUTTON] == GLUT_DOWN) {
			if (specialKeyStates[GLUT_KEY_ALT_L] * specialKeyStates[GLUT_KEY_ALT_R] == GLUT_DOWN) {
				float cameraMoveDis = mouseMove.Dot(Vec2f(1, -1)) * CAMERA_MOVE_SPEED;
				planeSceneCamera->ScaleDistanceAlongView(cameraMoveDis);
			}
			else {
				float cameraMoveDis = mouseMove.Dot(Vec2f(1, -1)) * CAMERA_MOVE_SPEED;
				//baseCamera->MoveCameraAlongView(cameraMoveDis);
				baseCamera->ScaleDistanceAlongView(cameraMoveDis);
			}
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
	// Source teapot scene data
	// read teapot data
	char objPath[50] = "Data\\";
	strcat(objPath, objName);
	
	targetObject = new cyTriMesh();
	if (targetObject->LoadFromFileObj(objPath, true)) {
		// process vertex data
		{
			baseNumIndices = targetObject->NF() * 3;
			Vertex* objVertices = new Vertex[baseNumIndices];
			unsigned int* objIndices = new unsigned int[baseNumIndices];

			for (int i = 0; i < targetObject->NF(); i++) {
				Vertex vertex = Vertex();

				vertex.Position  = targetObject->GetVec(i, Vec3f(1, 0, 0));
				vertex.Normal = targetObject->GetNormal(i, Vec3f(1, 0, 0));
				vertex.Texcoords = targetObject->GetTexCoord(i, Vec3f(1, 0, 0)).XY() * Vec2f(1, -1); // flip v direciton to get correct result
				objVertices[i * 3] = vertex;

				vertex = Vertex();
				vertex.Position = targetObject->GetVec(i, Vec3f(0, 1, 0));
				vertex.Normal = targetObject->GetNormal(i, Vec3f(0, 1, 0));
				vertex.Texcoords = targetObject->GetTexCoord(i, Vec3f(0, 1, 0)).XY() * Vec2f(1, -1);
				objVertices[i * 3 + 1] = vertex;

				vertex = Vertex();
				vertex.Position = targetObject->GetVec(i, Vec3f(0, 0, 1));
				vertex.Normal = targetObject->GetNormal(i, Vec3f(0, 0, 1));
				vertex.Texcoords = targetObject->GetTexCoord(i, Vec3f(0, 0, 1)).XY() * Vec2f(1, -1);
				objVertices[i * 3 + 2] = vertex;

				objIndices[i * 3] = i * 3;
				objIndices[i * 3 + 1] = i * 3 + 1;
				objIndices[i * 3 + 2] = i * 3 + 2;
			}

			glGenBuffers(1, &baseVertexBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
			glBufferData(GL_ARRAY_BUFFER, baseNumIndices * sizeof(Vertex), objVertices, GL_STATIC_DRAW);

			glGenBuffers(1, &baseIndexBufferID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, baseNumIndices * sizeof(unsigned int), objIndices, GL_STATIC_DRAW);

			delete[] objVertices;
			delete[] objIndices;
		}

		// process material data
		if (targetObject->NM() > 0) {
			cyTriMesh::Mtl* material = &targetObject->M(0);
			baseObjectMaterial->Initialize(material);
			// passing data
			{
				Material::Texture diffuseTexture = baseObjectMaterial->GetDiffuseTextureData();
				baseObjectDiffuse = new cyGLTexture2D();
				baseObjectDiffuse->Initialize();
				baseObjectDiffuse->SetWrappingMode(GL_REPEAT, GL_REPEAT);
				baseObjectDiffuse->SetImage(
					GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 
					diffuseTexture.textureData.data(), diffuseTexture.width, diffuseTexture.height, 0);
				baseObjectDiffuse->BuildMipmaps();

				Material::Texture specularTexture = baseObjectMaterial->GetSpecularTextureData();
				baseObjectSpecular = new cyGLTexture2D();
				baseObjectSpecular->Initialize();
				baseObjectSpecular->SetWrappingMode(GL_REPEAT, GL_REPEAT);
				baseObjectSpecular->SetImage(
					GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
					specularTexture.textureData.data(), specularTexture.width, specularTexture.height, 0);
				baseObjectSpecular->BuildMipmaps();
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		
		// setup vertex arrays
		glGenVertexArrays(1, &baseVertexArrayObjectID);

		glBindVertexArray(baseVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, baseVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)(sizeof(float) * 3));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)(sizeof(float) * 6));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baseIndexBufferID);
		glBindVertexArray(0); // unbind vertex array when done
	}
	else {
		return;
	}

	// Plane Scene data
	Plane plane;
	glGenBuffers(1, &RTexturePlaneVertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, RTexturePlaneVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), plane.GetVertices(), GL_STATIC_DRAW);

	glGenBuffers(1, &RTexturePlaneIndexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, RTexturePlaneIndexBufferID);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(unsigned int), plane.GetIndices(), GL_STATIC_DRAW);
	
	glGenVertexArrays(1, &RTextureVertexArrayObjectID);
	glBindVertexArray(RTextureVertexArrayObjectID);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, RTexturePlaneVertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)(sizeof(float) * 6));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RTexturePlaneIndexBufferID);

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------

void InstallTeapotSceneShaders() {
	vertexShader = new GLSLShader();
	fragmentShader = new GLSLShader();

	baseProgram = new GLSLProgram();
	CompileTeapotSceneShaders();
	baseProgram->RegisterUniforms(uniformNames);
}

//-------------------------------------------------------------------------------

void CompileTeapotSceneShaders() {
	// base scene shaders
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

	baseProgram->SetUniform("diffuseTexture", 0);
	baseProgram->SetUniform("specularTexture", 1);

	vertexShader->Delete();
	fragmentShader->Delete();
}

//-------------------------------------------------------------------------------

void InstallPlaneSceneShaders() {
	vertexShader = new GLSLShader();
	fragmentShader = new GLSLShader();

	RTextureProgram = new GLSLProgram();
	
	if (!vertexShader->CompileFile(RTextureVertShaderPath, GL_VERTEX_SHADER)) {
		return;
	}
	if (!fragmentShader->CompileFile(RTextureFragShaderPath, GL_FRAGMENT_SHADER)) {
		return;
	}

	RTextureProgram->CreateProgram();
	RTextureProgram->AttachShader(vertexShader->GetID());
	RTextureProgram->AttachShader(fragmentShader->GetID());
	RTextureProgram->Link();

	RTextureProgram->SetUniform("renderTexture", 2);

	vertexShader->Delete();
	fragmentShader->Delete();

	RTextureProgram->RegisterUniforms(planeSceneUniformNames);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------