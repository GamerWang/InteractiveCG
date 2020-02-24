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
#include <vector>

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

char targetVertShaderPath[30] = "Data\\VertexShader.glsl";
char targetFragShaderPath[30] = "Data\\FragmentShader.glsl";
GLSLShader* vertexShader;
GLSLShader* fragmentShader;
GLSLProgram* targetObjectProgram;
GLRenderTexture2D* baseSceneBuffer;

char RTextureVertShaderPath[30] = "Data\\SV_RTPlane.glsl";
char RTextureFragShaderPath[30] = "Data\\SF_RTPlane.glsl";
GLSLProgram* RTextureProgram;

char cubemapVertShaderPath[30] = "Data\\SV_Cubemap.glsl";
char cubemapFragShaderPath[30] = "Data\\SF_Cubemap.glsl";
GLSLProgram* cubemapProgram;

char planeVertShaderPath[30] = "Data\\SV_Plane.glsl";
char planeFragShaderPath[30] = "Data\\SF_Plane.glsl";
GLSLProgram* planeProgram;

//-------------------------------------------------------------------------------

char targetUniformNames[500] = {
	"objectToWorldMatrix"
	" objectNormalToWorldMatrix"
	" glossiness"
	" diffuseColor"
	" specularColor"
	" cameraPosition"
	" clipPlane"
	" pointLight0pos"
	" pointLight0Intensity"
	" brdfMode"
	" diffuseTexture"
	" specularTexture"
	" skybox"
};

char planeUniformNames[500] = {
	"objectToWorldMatrix"
};


char cubemapUniformNames[500] = {
	"objectToWorldMatrix"
};

char RTPlaneSceneUniformNames[500] = {
	"objectToClampMatrix"
};

//-------------------------------------------------------------------------------

GLuint baseVertexArrayObjectID;
GLuint baseVertexBufferID;
GLuint baseIndexBufferID;
GLuint baseNumIndices;

GLuint envCubeVertexArrayObjectID;
GLuint envVertexBufferID;
GLuint envIndexBufferID;

GLuint TextureVertexArrayObjectID;
GLuint TexturePlaneVertexBufferID;
GLuint TexturePlaneIndexBufferID;

GLuint uniformBufferObjectMatrices;

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

Vec3f planePosition;
Vec3f planeRotation;
Vec3f planeScale;

//-------------------------------------------------------------------------------

cyTriMesh* envCubeObject = nullptr;
GLTextureCubeMap* envCubeMapTexture;

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
void InstallRTPlaneSceneShaders();

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

	planePosition = Vec3f(0, -8, 0);
	planeScale = Vec3f(40);
	planeRotation = Vec3f(-Pi<float>() / 2, 0, 0);

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
	baseCamera->SetPosition(Vec3f(0, 0, 100));

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
	baseSceneBuffer->BuildTextureMipmaps();
	baseSceneBuffer->SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
	baseSceneBuffer->SetTextureMaxAnisotropy();

	// install shaders to opengl
	InstallTeapotSceneShaders();
	//InstallRTPlaneSceneShaders();

	// generate and bind uniform buffer objects
	{
		{
			// get relevant block indices
			GLuint uniformBlockIndexBaseVert = glGetUniformBlockIndex(targetObjectProgram->GetID(), "Matrices");
			GLuint uniformBlockIndexCubemapVert = glGetUniformBlockIndex(cubemapProgram->GetID(), "Matrices");
			GLuint uniformBlockIndexPlaneVert = glGetUniformBlockIndex(planeProgram->GetID(), "Matrices");

			// link each shader's uniform block to this uniform binding point
			glUniformBlockBinding(targetObjectProgram->GetID(), uniformBlockIndexBaseVert, 0);
			glUniformBlockBinding(cubemapProgram->GetID(), uniformBlockIndexCubemapVert, 0);
			glUniformBlockBinding(planeProgram->GetID(), uniformBlockIndexPlaneVert, 0);

			// now create the buffer
			glGenBuffers(1, &uniformBufferObjectMatrices);
			glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObjectMatrices);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix4f), NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			// define the range of the buffer that links to a uniform binding point
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, uniformBufferObjectMatrices, 0, sizeof(Matrix4f));
		}
	}

	glutMainLoop();
} 

//-------------------------------------------------------------------------------

void GlutDisplay() {
	// rendering the teapot reflection to target buffer
	{
		baseSceneBuffer->Bind();

		glEnable(GL_CLIP_DISTANCE0);

		baseSceneBuffer->Unbind();
	}

	glClearColor(0, 0, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// send worldToClamp to uniform buffer object
	{
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

		glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObjectMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4f), &worldToClampMatrix.cell[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// render environment cube
	{
		glDepthMask(GL_FALSE);

		Matrix4f objectToWorldMatrix =
			Matrix4f::Translation(baseCamera->GetPosition());

		Matrix4f WorldToViewMatrix =
			baseCamera->WorldToViewMatrix();

		glBindVertexArray(envCubeVertexArrayObjectID);
		cubemapProgram->Bind();

		cubemapProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);

		switch (renderMode)
		{
		case XW_RENDER_LINELOOP:
			glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_INT, 0);
			break;
		case XW_RENDER_TRIANGLES:
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		default:
			break;
		}
	}

	// render target object
	// compute matrices here
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_CLIP_DISTANCE0);

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
		targetObjectProgram->Bind();

		targetObjectProgram->SetUniformMatrix4("worldToClampMatrix", &worldToClampMatrix.cell[0]);
		targetObjectProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
		targetObjectProgram->SetUniformMatrix3("objectNormalToWorldMatrix", &objectNormalToWorldMatrix.cell[0]);

		targetObjectProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
		targetObjectProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
		targetObjectProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

		targetObjectProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());
		targetObjectProgram->SetUniform3("ambientLight", 1, ambientLight.GetIntensity().Elements());
		targetObjectProgram->SetUniform3("pointLight0pos", 1, pointLight0.GetPosition().Elements());
		targetObjectProgram->SetUniform3("pointLight0Intensity", 1, pointLight0.GetIntensity().Elements());

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
	}

	// render plane object
	{
		Matrix4f objectToWorldMatrix = 
			Matrix4f::Translation(planePosition) *
			Matrix4f::Scale(planeScale) *
			Matrix4f::RotationXYZ(planeRotation.x, planeRotation.y, planeRotation.z);

		glBindVertexArray(TextureVertexArrayObjectID);
		planeProgram->Bind();

		planeProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);

		switch (renderMode)
		{
		case XW_RENDER_LINELOOP:
			glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
			break;
		case XW_RENDER_TRIANGLES:
		default:
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			break;
		}
	}

	//baseSceneBuffer->Unbind();

	//Matrix4f PlaneSceneWorldToViewMatrix =
	//	planeSceneCamera->WorldToViewMatrix();
	//if (planeSceneCamera->GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
	//	Matrix4f viewToProjectionMatrix = planeSceneCamera->ViewToProjectionMatrix();
	//	PlaneSceneWorldToViewMatrix = viewToProjectionMatrix * PlaneSceneWorldToViewMatrix;
	//}
	//else if (planeSceneCamera->GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
	//	PlaneSceneWorldToViewMatrix.OrthogonalizeZ();
	//}

	//glClearColor(.5f, .5f, .5f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//RTextureProgram->Bind();
	//glBindVertexArray(TextureVertexArrayObjectID);
	//baseSceneBuffer->BindTexture(3);

	//RTextureProgram->SetUniformMatrix4("objectToClampMatrix", &PlaneSceneWorldToViewMatrix.cell[0]);

	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
	case GLUT_KEY_F1:
		targetObjectProgram->SetUniform("brdfMode", 0);
		renderMode = XW_RENDER_TRIANGLES;
		break;
	case GLUT_KEY_F2:
		targetObjectProgram->SetUniform("brdfMode", 1);
		renderMode = XW_RENDER_TRIANGLES;
		break;
	case GLUT_KEY_F3:
		targetObjectProgram->SetUniform("brdfMode", 0);
		renderMode = XW_RENDER_LINELOOP;
		break;
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
	char objPath[50] = "Data\\";
	strcat(objPath, objName);
	
	// read teapot data
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

		// process target material data
		{
			if (targetObject->NM() > 0) {
				cyTriMesh::Mtl* material = &targetObject->M(0);
				baseObjectMaterial->Initialize(material);
			}
			else {
				baseObjectMaterial->Initialize();
			}
			// passing data
			{
				Material::Texture diffuseTexture = baseObjectMaterial->GetDiffuseTextureData();
				baseObjectDiffuse = new cyGLTexture2D();
				baseObjectDiffuse->Initialize();
				baseObjectDiffuse->SetFilteringMode(GL_LINEAR, GL_LINEAR);
				baseObjectDiffuse->SetWrappingMode(GL_REPEAT, GL_REPEAT);
				baseObjectDiffuse->SetMaxAnisotropy();
				baseObjectDiffuse->SetImage(
					GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 
					diffuseTexture.textureData.data(), diffuseTexture.width, diffuseTexture.height);
				baseObjectDiffuse->BuildMipmaps();

				Material::Texture specularTexture = baseObjectMaterial->GetSpecularTextureData();
				baseObjectSpecular = new cyGLTexture2D();
				baseObjectSpecular->Initialize();
				baseObjectSpecular->SetFilteringMode(GL_LINEAR, GL_LINEAR);
				baseObjectSpecular->SetWrappingMode(GL_REPEAT, GL_REPEAT);
				baseObjectSpecular->SetMaxAnisotropy();
				baseObjectSpecular->SetImage(
					GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
					specularTexture.textureData.data(), specularTexture.width, specularTexture.height);
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

	// read cube data
	char cubePath[50] = "Data\\cube.obj";
	envCubeObject = new cyTriMesh();
	if (envCubeObject->LoadFromFileObj(cubePath, true)) {
		GLuint vertexNumber = envCubeObject->NV();
		GLuint indicesNumber = envCubeObject->NF() * 3;
		Vec3f* cubeVertices = new Vec3f[vertexNumber];
		GLuint* cubeIndices = new GLuint[indicesNumber];

		for (GLuint i = 0; i < vertexNumber; i++) {
			cubeVertices[i] = Vec3f(envCubeObject->V(i));
		}

		for (GLuint i = 0; i < envCubeObject->NF(); i++) {
			TriMesh::TriFace currentFace = envCubeObject->F(i);

			cubeIndices[i * 3] = currentFace.v[0];
			cubeIndices[i * 3 + 1] = currentFace.v[1];
			cubeIndices[i * 3 + 2] = currentFace.v[2];
			//printf("Current index: %d, %d, %d\n", currentFace.v[0], currentFace.v[1], currentFace.v[2]);
		}

		glGenBuffers(1, &envVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, envVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, vertexNumber * sizeof(Vec3f), cubeVertices, GL_STATIC_DRAW);

		glGenBuffers(1, &envIndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, envIndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNumber * sizeof(GLuint), cubeIndices, GL_STATIC_DRAW);

		delete[] cubeVertices;
		delete[] cubeIndices;

		glGenVertexArrays(1, &envCubeVertexArrayObjectID);

		glBindVertexArray(envCubeVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, envVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, envIndexBufferID);
		glBindVertexArray(0);
	}
	else {
		return;
	}

	// process cubemap textures
	{
		std::vector<Material::Texture> faces{
			Material::Texture("environment\\cubemap_posx.png"),
			Material::Texture("environment\\cubemap_negx.png"),
			Material::Texture("environment\\cubemap_posy.png"),
			Material::Texture("environment\\cubemap_negy.png"),
			Material::Texture("environment\\cubemap_posz.png"),
			Material::Texture("environment\\cubemap_negz.png")
		};

		envCubeMapTexture = new GLTextureCubeMap();
		envCubeMapTexture->Initialize();

		for (int i = 0; i < 6; i++) {
			envCubeMapTexture->SetImage(
				(GLTextureCubeMap::Side)i,
				GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
				faces[i].textureData.data(), faces[i].width, faces[i].height
			);
		}

		envCubeMapTexture->SetFilteringMode(GL_LINEAR, GL_LINEAR);
		envCubeMapTexture->SetMaxAnisotropy();
		envCubeMapTexture->SetSeamless(true);
		envCubeMapTexture->BuildMipmaps();
	}

	// Plane data
	{
		Plane plane;
		glGenBuffers(1, &TexturePlaneVertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, TexturePlaneVertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), plane.GetVertices(), GL_STATIC_DRAW);

		glGenBuffers(1, &TexturePlaneIndexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, TexturePlaneIndexBufferID);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(unsigned int), plane.GetIndices(), GL_STATIC_DRAW);
	
		glGenVertexArrays(1, &TextureVertexArrayObjectID);
		glBindVertexArray(TextureVertexArrayObjectID);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, TexturePlaneVertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char*)(sizeof(float) * 6));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TexturePlaneIndexBufferID);

		glBindVertexArray(0);	
	}

	baseObjectDiffuse->Bind(0);
	baseObjectSpecular->Bind(1);
	envCubeMapTexture->Bind(2);
}
//-------------------------------------------------------------------------------

void InstallTeapotSceneShaders() {
	vertexShader = new GLSLShader();
	fragmentShader = new GLSLShader();

	targetObjectProgram = new GLSLProgram();
	cubemapProgram = new GLSLProgram();
	planeProgram = new GLSLProgram();

	CompileTeapotSceneShaders();

	targetObjectProgram->RegisterUniforms(targetUniformNames);
	cubemapProgram->RegisterUniforms(cubemapUniformNames);
	planeProgram->RegisterUniforms(planeUniformNames);
}

//-------------------------------------------------------------------------------

void CompileTeapotSceneShaders() {
	// base scene shaders
	// env cubemap shader
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}

		if (!vertexShader->CompileFile(cubemapVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(cubemapFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}

		cubemapProgram->CreateProgram();
		cubemapProgram->AttachShader(vertexShader->GetID());
		cubemapProgram->AttachShader(fragmentShader->GetID());
		cubemapProgram->Link();

		cubemapProgram->SetUniform("skybox", 2);
	}

	// target object shader
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}
		if (!vertexShader->CompileFile(targetVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(targetFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}

		targetObjectProgram->CreateProgram();
		targetObjectProgram->AttachShader(vertexShader->GetID());
		targetObjectProgram->AttachShader(fragmentShader->GetID());
		targetObjectProgram->Link();

		targetObjectProgram->SetUniform("brdfMode", 1);
		targetObjectProgram->SetUniform("diffuseTexture", 0);
		targetObjectProgram->SetUniform("specularTexture", 1);
		targetObjectProgram->SetUniform("skybox", 2);
	}

	// reflective plane shader
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}
		if (!vertexShader->CompileFile(planeVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(planeFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}

		planeProgram->CreateProgram();
		planeProgram->AttachShader(vertexShader->GetID());
		planeProgram->AttachShader(fragmentShader->GetID());
		planeProgram->Link();
		planeProgram->SetUniform("skybox", 2);
	}

	vertexShader->Delete();
	fragmentShader->Delete();
}

//-------------------------------------------------------------------------------

void InstallRTPlaneSceneShaders() {
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

	RTextureProgram->SetUniform("renderTexture", 3);

	vertexShader->Delete();
	fragmentShader->Delete();

	RTextureProgram->RegisterUniforms(RTPlaneSceneUniformNames);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------