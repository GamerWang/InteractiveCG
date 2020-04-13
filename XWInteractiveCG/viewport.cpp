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
#include <filesystem>

//-------------------------------------------------------------------------------

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
#include "xwModel.h";

//-------------------------------------------------------------------------------

enum RenderMode {
	XW_RENDER_TRIANGLES,
	XW_RENDER_LINELOOP
};

//-------------------------------------------------------------------------------

//#define enable_env_cube
//#define plane_reflective
#define plane_diffuse 0

#define tScale_Uniform 0
//#define tScale_NUniform 0

//-------------------------------------------------------------------------------

GLRenderTexture2D* reflectionFrameBuffer;

//-------------------------------------------------------------------------------

GLSLShader* vertexShader;
GLSLShader* fragmentShader;
GLSLShader* geometryShader;

char targetVertShaderPath[30] = "Data\\SV_TargetObj.glsl";
char targetFragShaderPath[30] = "Data\\SF_TargetObj.glsl";
GLSLProgram* targetObjectProgram;

char targetOutlineVertShaderPath[30] = "Data\\TargetObjOutline_VS.glsl";
char targetOutlineGeomShaderPath[30] = "Data\\TargetObjOutline_GS.glsl";
char targetOutlineFragShaderPath[30] = "Data\\TargetObjOutline_FS.glsl";
GLSLProgram* targetOutlineProgram;

char pokeballVertShaderPath[30] = "Data\\PokeBall_VS.glsl";
char pokeballFragShaderPath[30] = "Data\\PokeBall_FS.glsl";
GLSLProgram* pokeBallProgram;

char RTextureVertShaderPath[30] = "Data\\SV_RTPlane.glsl";
char RTextureFragShaderPath[30] = "Data\\SF_RTPlane.glsl";
GLSLProgram* RTextureProgram;

char cubemapVertShaderPath[30] = "Data\\SV_Cubemap.glsl";
char cubemapFragShaderPath[30] = "Data\\SF_Cubemap.glsl";
GLSLProgram* cubemapProgram;

char pointLightDepthVertShaderPath[40] = "Data\\PointShadowDepth_VS.glsl";
char pointLightDepthFragShaderPath[40] = "Data\\PointShadowDepth_FS.glsl";
char pointLightDepthGeomShaderPath[40] = "Data\\PointShadowDepth_GS.glsl";
GLSLProgram* pointLightDepthProgram;

#ifdef plane_reflective
char planeVertShaderPath[30] = "Data\\SV_RefPlane.glsl";
char planeFragShaderPath[30] = "Data\\SF_RefPlane.glsl";
#endif // plane_reflective
#ifdef plane_diffuse
char planeVertShaderPath[30] = "Data\\SV_DiffPlane.glsl";
char planeFragShaderPath[30] = "Data\\SF_DiffPlane.glsl";
#endif // plane_diffuse
GLSLProgram* planeProgram;

//-------------------------------------------------------------------------------

Model* mainModel;
Model* pBallTopModel;
Model* pBallLockModel;
Model* pBallBottomModel;

//-------------------------------------------------------------------------------

char targetUniformNames[500] = {
	"objectToWorldMatrix"
	" objectNormalToWorldMatrix"
	" glossiness"
	" cameraPosition"
	" clipPlane"
	" brdfMode"
	" diffuseTexture"
	" specularTexture"
	" matcapTexture"
	" matcapMask"
	" skybox"
	" depthMap"
	" far_plane"
};

char pointShadowUniformNames[500] = {
	"model"
	" lightPos"
	" far_plane"
	" shadowMatrices[0]"
	" shadowMatrices[1]"
	" shadowMatrices[2]"
	" shadowMatrices[3]"
	" shadowMatrices[4]"
	" shadowMatrices[5]"
};

char planeUniformNames[500] = {
	"objectToWorldMatrix"
	//" cameraPosition"
	" worldNormal"
#ifdef plane_diffuse
	" diffuseTexture"
	" depthMap"
	" far_plane"
#endif // plane_diffuse
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

GLuint UBOMatrices;
GLuint UBOLights;

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

//-------------------------------------------------------------------------------

GLuint depthMapFBO;
GLuint depthCubemap;
const GLuint SHADOW_WIDTH = 1024;
const GLuint SHADOW_HEIGHT = 1024;
GLfloat depthNear = 1.0f;
GLfloat depthFar = 200.0f;

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
void CompileShaders();

//-------------------------------------------------------------------------------

bool IsMouseDown() { return !(mouseStates[0] * mouseStates[1] * mouseStates[2]); }

//-------------------------------------------------------------------------------

void ShowViewport(int argc, char* argv[]) {
	// initialize scene data
	{
		baseCamera = new Camera();
	}

	// Fuka-settings
	/*{
		baseObjectPosition = Vec3f(0, -43, 0);
		baseObjectRotation = Vec3f(Pi<float>() / 2, 0, 0);
		baseObjectScale = Vec3f(.9f, .9f, .9f);

		planePosition = Vec3f(0, -43, 0);
		baseCamera->SetPosition(Vec3f(0, 0, 30));
	}*/

	// Magikarp settings
	{
#ifdef tScale_Uniform
		baseObjectPosition = Vec3f(0, -23, 0);
		baseObjectRotation = Vec3f(Pi<float>() / 2, -Pi<float>() / 3, 0);
		baseObjectScale = Vec3f(.3f, .3f, .3f);
#endif

#ifdef tScale_NUniform
		baseObjectPosition = Vec3f(0, -23, 0);
		baseObjectRotation = Vec3f(0, -Pi<float>() / 3, 0);
		baseObjectScale = Vec3f(.6f, .6f, .6f);
#endif

		planePosition = Vec3f(0, -45, 0);
		baseCamera->SetPosition(Vec3f(0, 0, 100));
	}

	baseObjectMaterial = new Material();
	baseObjectGlossiness = 20;
	baseObjectDiffuseColor = Vec3f(.3f, .6f, .9f);
	baseObjectSpecularColor = Vec3f(.9f);

	baseObjectColor = Vec3f(0, 0, 0);

	// center cut position
	//planePosition = Vec3f(0, -8, 0);
	// below teapot position
	//planePosition = Vec3f(0, -43, 0);
	planeScale = Vec3f(280);
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

	//baseCamera = new Camera();
	baseCamera->SetAspect((float)screenSize[0] / (float)screenSize[1]);
	//baseCamera->SetPosition(Vec3f(0, 0, 30));

	planeSceneCamera = new Camera();
	planeSceneCamera->SetAspect((float)screenSize[0] / (float)screenSize[1]);
	planeSceneCamera->SetPosition(Vec3f(0, 0, 3));

	ambientLight.SetIntensity(.15f);
	pointLight0.SetPosition(Vec3f(0, 0, 80));
	pointLight0.SetIntensity(Vec3f(.7f));
	pointLight0.SetRotation(Vec2f(Pi<float>() * (-50.f/180.f), Pi<float>() * (30.f / 180.f)));

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
	glEnable(GL_CULL_FACE);

	// prepare data for opengl
	if (argc <= 1) {
		printf("Obj filename missing!\n");
		return;
	}
	else {
		SendDataToOpenGL(argv[1]);
	}

	//if (!targetObject->IsBoundBoxReady()) {
	//	targetObject->ComputeBoundingBox();
	//}
	//baseObjectCenter = targetObject->GetBoundMax() + targetObject->GetBoundMin();
	//baseObjectCenter /= 2;

	// initialize reflection render texture buffer
	{
		reflectionFrameBuffer = new GLRenderTexture2D();
		reflectionFrameBuffer->Initialize(true, 3, 800, 800);
		reflectionFrameBuffer->BuildTextureMipmaps();
		reflectionFrameBuffer->SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
		reflectionFrameBuffer->SetTextureMaxAnisotropy();
	}

	// initialize depth map frame buffer
	{
		glGenFramebuffers(1, &depthMapFBO);
		glGenTextures(1, &depthCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		for (GLuint i = 0; i < 6; i++) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
				SHADOW_WIDTH, SHADOW_HEIGHT, 0, 
				GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// attach cubemap as depth map FBO's color buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("Framebuffer not complete!\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// read model
	mainModel = new Model("Data/Magikarp_UScale.fbx");
	pBallTopModel = new Model("Data/Pokeball_top.fbx");
	pBallLockModel = new Model("Data/Pokeball_Lock.fbx");
	pBallBottomModel = new Model("Data/Pokeball_Bottom.fbx");

	// install shaders to opengl
	InstallTeapotSceneShaders();
	//InstallRTPlaneSceneShaders();

	// generate and bind uniform buffer objects
	{
		// Matrices buffer object
		{
			// get relevant block indices
			// link each shader's uniform block to this uniform binding point
			GLuint uniformBlockIndexBaseVert = glGetUniformBlockIndex(targetObjectProgram->GetID(), "Matrices");
			glUniformBlockBinding(targetObjectProgram->GetID(), uniformBlockIndexBaseVert, 0);

#ifdef enable_env_cube
			GLuint uniformBlockIndexCubemapVert = glGetUniformBlockIndex(cubemapProgram->GetID(), "Matrices");
			glUniformBlockBinding(cubemapProgram->GetID(), uniformBlockIndexCubemapVert, 0);
#endif // enable_env_cube
			
			GLuint uniformBlockIndexPlaneVert = glGetUniformBlockIndex(planeProgram->GetID(), "Matrices");
			glUniformBlockBinding(planeProgram->GetID(), uniformBlockIndexPlaneVert, 0);

			// now create the buffer
			glGenBuffers(1, &UBOMatrices);
			glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix4f) * 3, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			// define the range of the buffer that links to a uniform binding point
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBOMatrices, 0, sizeof(Matrix4f) * 3);
		}
		// lights buffer object
		{
			GLuint uniformBlockIndexBaseFrag = glGetUniformBlockIndex(targetObjectProgram->GetID(), "Lights");
			glUniformBlockBinding(targetObjectProgram->GetID(), uniformBlockIndexBaseFrag, 1);

#ifdef plane_diffuse
			GLuint uniformBlockIndexPlaneFrag = glGetUniformBlockIndex(planeProgram->GetID(), "Lights");
			glUniformBlockBinding(planeProgram->GetID(), uniformBlockIndexPlaneFrag, 1);
#endif // plane_diffuse

			GLuint uniformBlockIndexPBFrag = glGetUniformBlockIndex(pokeBallProgram->GetID(), "Lights");
			glUniformBlockBinding(pokeBallProgram->GetID(), uniformBlockIndexBaseFrag, 1);

			glGenBuffers(1, &UBOLights);
			glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glBindBufferRange(GL_UNIFORM_BUFFER, 1, UBOLights, 0, sizeof(Vec3f) * 3);
		}
	}

	glutMainLoop();
} 

//-------------------------------------------------------------------------------

void GlutDisplay() {
	// rendering the teapot reflection to target buffer
	{
		glViewport(0, 0, screenSize[0], screenSize[1]);
		reflectionFrameBuffer->Bind();

		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Camera reflectionCam = Camera(baseCamera);
		// setting the reflection camera
		{
			reflectionCam.ReflectOnYPlane(planePosition.y);

			Matrix4f worldToViewMatrix =
				reflectionCam.WorldToViewMatrix();

			Matrix4f worldToClampMatrix = worldToViewMatrix;
			Matrix4f viewToProjectionMatrix = reflectionCam.ViewToProjectionMatrix();
			if (reflectionCam.GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
				worldToClampMatrix = viewToProjectionMatrix * worldToClampMatrix;
			}
			else if (reflectionCam.GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
				worldToClampMatrix.OrthogonalizeZ();
			}
			worldToClampMatrix = Matrix4f::Scale(-1, -1, 1, 1) * worldToClampMatrix;

			glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4f), &worldToClampMatrix.cell[0]);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		// send point light info to uniform buffer object
		{
			glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);

			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Vec3f), ambientLight.GetIntensity().Elements());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4, sizeof(Vec3f), pointLight0.GetIntensity().Elements());
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 8, sizeof(Vec3f), pointLight0.GetPosition().Elements());

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

#ifdef enable_env_cube
		// render environment cube to reflection buffer
		//{
		//	glDepthMask(GL_FALSE);

		//	Matrix4f objectToWorldMatrix =
		//		Matrix4f::Translation(reflectionCam.GetPosition());

		//	glBindVertexArray(envCubeVertexArrayObjectID);
		//	cubemapProgram->Bind();

		//	cubemapProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);

		//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//}
#endif // enable_environment_cube

		// render target object
		{
			glDepthMask(GL_TRUE);
			glEnable(GL_CLIP_DISTANCE0);

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

			Vec4f clipPlane = Vec4f(0, 1, 0, -planePosition.y);
			Vec4f cameraPos = Vec4f(baseCamera->GetPosition());
			if (cameraPos.Dot(clipPlane) < 0) {
				clipPlane *= -1;
			}

			glBindVertexArray(baseVertexArrayObjectID);
			targetObjectProgram->Bind();

			targetObjectProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
			targetObjectProgram->SetUniformMatrix3("objectNormalToWorldMatrix", &objectNormalToWorldMatrix.cell[0]);

			targetObjectProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
			targetObjectProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
			targetObjectProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

			targetObjectProgram->SetUniform3("cameraPosition", 1, reflectionCam.GetPosition().Elements());

			targetObjectProgram->SetUniform4("clipPlane", 1, clipPlane.Elements());

			glDrawElements(GL_TRIANGLES, baseNumIndices, GL_UNSIGNED_INT, 0);
		}

		reflectionFrameBuffer->Unbind();
	}

	// rendering depth to frame buffer
	{
		GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;

		Matrix4f shadowProj = Matrix4f::Perspective(Pi<float>() / 2 , aspect, depthNear, depthFar);
		std::vector<Matrix4f> shadowTransforms;
		Vec3f lightp = pointLight0.GetPosition();
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f( 1,  0,  0), Vec3f(0, -1,  0)));
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f(-1,  0,  0), Vec3f(0, -1,  0)));
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f( 0,  1,  0), Vec3f(0,  0,  1)));
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f( 0, -1,  0), Vec3f(0,  0, -1)));
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f( 0,  0,  1), Vec3f(0, -1,  0)));
		shadowTransforms.push_back(shadowProj* Matrix4f::View(lightp, lightp + Vec3f( 0,  0, -1), Vec3f(0, -1,  0)));

		// render to buffer
		{
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			pointLightDepthProgram->Bind();

			for (GLuint i = 0; i < 6; i++) {
				std::string targetMatrixName = "shadowMatrices[" + std::to_string(i) + "]";
				pointLightDepthProgram->SetUniformMatrix4(
					targetMatrixName.c_str(),
					&shadowTransforms[i].cell[0]);
			}
			pointLightDepthProgram->SetUniform1("far_plane", 1, &depthFar);
			pointLightDepthProgram->SetUniform3("lightPos", 1, pointLight0.GetPosition().Elements());

			// render scene
			{
				// render target object
				{
					Matrix4f objectToWorldMatrix =
						Matrix4f::Translation(baseObjectPosition) *
						Matrix4f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) *
						Matrix4f::Scale(baseObjectScale) *
						Matrix4f::RotationXYZ(-Pi<float>() / 2, 0, 0) *
						Matrix4f::Translation(-baseObjectCenter);

					pointLightDepthProgram->SetUniformMatrix4("model", &objectToWorldMatrix.cell[0]);

					mainModel->Draw(pointLightDepthProgram);
				}

				// render plane
				{
					Matrix4f objectToWorldMatrix =
						Matrix4f::Translation(planePosition) *
						Matrix4f::Scale(planeScale) *
						Matrix4f::RotationXYZ(planeRotation.x, planeRotation.y, planeRotation.z);

					pointLightDepthProgram->SetUniformMatrix4("model", &objectToWorldMatrix.cell[0]);

					glBindVertexArray(TextureVertexArrayObjectID);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				}
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	// render scene as normal
	glViewport(0, 0, screenSize[0], screenSize[1]);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//glClearColor(0, 0, 0, 1.0f);
	//glClearColor(.071f, .0625f, .165f, 1.0f);
	glClearColor(0.1796875, 0.8125, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// send worldToClamp to uniform buffer object
	{
		Matrix4f worldToViewMatrix =
			baseCamera->WorldToViewMatrix();

		Matrix4f worldToClampMatrix = worldToViewMatrix;
		if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_PERSPECTIVE) {
			Matrix4f viewToProjectionMatrix = baseCamera->ViewToProjectionMatrix();
			worldToClampMatrix = viewToProjectionMatrix * worldToClampMatrix;
		}
		else if (baseCamera->GetCameraType() == CameraType::XW_CAMERA_ORTHOGONAL) {
			worldToClampMatrix.OrthogonalizeZ();
		}

		Matrix4f viewToProjectionMatrix = baseCamera->ViewToProjectionMatrix();

		glBindBuffer(GL_UNIFORM_BUFFER, UBOMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4f), &worldToClampMatrix.cell[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4f), sizeof(Matrix4f), &worldToViewMatrix.cell[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4f) * 2, sizeof(Matrix4f), &viewToProjectionMatrix.cell[0]);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// send outline shader matrices
		targetOutlineProgram->SetUniformMatrix4("worldToView", &worldToViewMatrix.cell[0]);
		targetOutlineProgram->SetUniformMatrix4("viewToClip", &viewToProjectionMatrix.cell[0]);

		pokeBallProgram->SetUniformMatrix4("worldToView", &worldToViewMatrix.cell[0]);
		pokeBallProgram->SetUniformMatrix4("viewToClip", &viewToProjectionMatrix.cell[0]);
	}

	// send point light info to uniform buffer object
	{
		glBindBuffer(GL_UNIFORM_BUFFER, UBOLights);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Vec3f), ambientLight.GetIntensity().Elements());
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4, sizeof(Vec3f), pointLight0.GetIntensity().Elements());
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 8, sizeof(Vec3f), pointLight0.GetPosition().Elements());

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

#ifdef enable_env_cube
	// render environment cube
	{
		glDepthMask(GL_FALSE);

		Matrix4f objectToWorldMatrix =
			Matrix4f::Translation(baseCamera->GetPosition());

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
#endif // enable_environment_cube

	// render plane object
	{
		Matrix4f objectToWorldMatrix =
			Matrix4f::Translation(planePosition) *
			Matrix4f::Scale(planeScale) *
			Matrix4f::RotationXYZ(planeRotation.x, planeRotation.y, planeRotation.z);

		Vec3f worldNormal = Vec3f(0, 1, 0);
		Vec4f clipPlane = Vec4f(0, 1, 0, -planePosition.y);
		Vec4f cameraPos = Vec4f(baseCamera->GetPosition());
		if (cameraPos.Dot(clipPlane) < 0) {
			worldNormal *= -1;
		}

		planeProgram->Bind();
		reflectionFrameBuffer->BindTexture(3);

		planeProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
		planeProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());
		planeProgram->SetUniform3("worldNormal", 1, worldNormal.Elements());
		planeProgram->SetUniform1("far_plane", 1, &depthFar);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, baseObjectDiffuse->GetID());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		glBindVertexArray(TextureVertexArrayObjectID);
		switch (renderMode)
		{
		case XW_RENDER_LINELOOP:
			//glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
			break;
		case XW_RENDER_TRIANGLES:
		default:
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			break;
		}
	}

	// render target object
	// compute matrices here
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_CLIP_DISTANCE0);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

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

		targetObjectProgram->Bind();

		targetObjectProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
		targetObjectProgram->SetUniformMatrix3("objectNormalToWorldMatrix", &objectNormalToWorldMatrix.cell[0]);
		targetObjectProgram->SetUniform1("far_plane", 1, &depthFar);

		targetObjectProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
		targetObjectProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
		targetObjectProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

		targetObjectProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		mainModel->Draw(targetObjectProgram);
		
		targetOutlineProgram->SetUniformMatrix4("objectToWorld", &objectToWorldMatrix.cell[0]);
		targetOutlineProgram->SetUniformMatrix3("objNmlToWorld", &objectNormalToWorldMatrix.cell[0]);

		pokeBallProgram->SetUniformMatrix4("objectToWorld", &objectToWorldMatrix.cell[0]);
		pokeBallProgram->SetUniformMatrix3("objNmlToWorld", &objectNormalToWorldMatrix.cell[0]);
		pokeBallProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());
		
		// Draw outlines
		targetOutlineProgram->Bind();

		glCullFace(GL_FRONT);

		Vec3f tOutlineClr = Vec3f(.8f, .4f, .3f);
		//Vec3f tOutlineClr = Vec3f(1.f, .9f, .9f);
		Vec3f bOutlineClr = Vec3f(.0f, .0f, .0f);

		targetOutlineProgram->SetUniform3("color", 1, tOutlineClr.Elements());
		mainModel->Draw(targetOutlineProgram);

		targetOutlineProgram->SetUniform3("color", 1, bOutlineClr.Elements());
		pBallTopModel->Draw(targetOutlineProgram);
		pBallBottomModel->Draw(targetOutlineProgram);

		// Draw pokeball
		pokeBallProgram->Bind();

		Vec3f pbTopClr = Vec3f(.86, .0, .0);
		Vec3f pbLckClr = Vec3f(.0, .0, .0);
		Vec3f pbBotClr = Vec3f(.8, .8, .8);

		Vec3f yinClr = Vec3f(.3f, .3f, .5f);

		pokeBallProgram->SetUniform3("color", 1, pbTopClr.Elements());
		pokeBallProgram->SetUniform3("yinClr", 1, Vec3f(1).Elements());
		pBallTopModel->Draw(pokeBallProgram);

		glDisable(GL_CULL_FACE);

		pokeBallProgram->SetUniform3("color", 1, pbLckClr.Elements());
		pokeBallProgram->SetUniform3("yinClr", 1, Vec3f(0).Elements());
		pBallLockModel->Draw(pokeBallProgram);
		
		pokeBallProgram->SetUniform3("color", 1, pbBotClr.Elements());
		pokeBallProgram->SetUniform3("yinClr", 1, yinClr.Elements());
		pBallBottomModel->Draw(pokeBallProgram);
	}

	// render light object temp
	{
		// send uniforms here
		Matrix4f objectToWorldMatrix =
			Matrix4f::Translation(pointLight0.GetPosition()) *
			Matrix4f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) *
			Matrix4f::Scale(0.2f) *
			Matrix4f::RotationXYZ(-Pi<float>() / 2, 0, 0) *
			Matrix4f::Translation(-baseObjectCenter);

		Matrix3f objectNormalToWorldMatrix =
			Matrix3f::RotationXYZ(baseObjectRotation.x, baseObjectRotation.y, baseObjectRotation.z) *
			Matrix3f::Scale(baseObjectScale).GetInverse() *
			Matrix3f::RotationXYZ(-Pi<float>() / 2, 0, 0);

		targetObjectProgram->Bind();

		targetObjectProgram->SetUniformMatrix4("objectToWorldMatrix", &objectToWorldMatrix.cell[0]);
		targetObjectProgram->SetUniformMatrix3("objectNormalToWorldMatrix", &objectNormalToWorldMatrix.cell[0]);

		targetObjectProgram->SetUniform1("glossiness", 1, &baseObjectGlossiness);
		targetObjectProgram->SetUniform3("diffuseColor", 1, baseObjectDiffuseColor.Elements());
		targetObjectProgram->SetUniform3("specularColor", 1, baseObjectSpecularColor.Elements());

		targetObjectProgram->SetUniform3("cameraPosition", 1, baseCamera->GetPosition().Elements());

		glBindVertexArray(baseVertexArrayObjectID);
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

	glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void GlutIdle() {
	//clock_t t;
	//t = clock();

	//float tFrac = t / 1000.0f;

	//baseObjectColor.x = .5f * sinf(tFrac) + .5f;
	//baseObjectColor.y = .5f * cosf(2 * tFrac - 2) + .5f;
	//baseObjectColor.z = .5f * sinf(-3 * tFrac + 1) + .5f;

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
		CompileShaders();
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
	
	// read target data using cyTriMesh
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

		// read target data using AssImp


		// process target material data
		/*{
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
		}*/

		// hard coded texture load
		{
			Material::Texture diffuseTexture = Material::Texture("toonWater.png");
			baseObjectDiffuse = new cyGLTexture2D();
			baseObjectDiffuse->Initialize();
			baseObjectDiffuse->SetFilteringMode(GL_LINEAR, GL_LINEAR);
			baseObjectDiffuse->SetWrappingMode(GL_REPEAT, GL_REPEAT);
			baseObjectDiffuse->SetMaxAnisotropy();
			baseObjectDiffuse->SetImage(
				GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
				diffuseTexture.textureData.data(), diffuseTexture.width, diffuseTexture.height);
			baseObjectDiffuse->BuildMipmaps();


			Material::Texture specularTexture = Material::Texture("brick-specular.png");
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

#ifdef enable_env_cube
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
#endif // enable_env_cube

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
#ifdef enable_env_cube
	envCubeMapTexture->Bind(2);
#endif // enable_env_cube
}
//-------------------------------------------------------------------------------

void InstallTeapotSceneShaders() {
	vertexShader = new GLSLShader();
	fragmentShader = new GLSLShader();
	geometryShader = new GLSLShader();

	// programs
	targetObjectProgram = new GLSLProgram();
	targetOutlineProgram = new GLSLProgram();
	pokeBallProgram = new GLSLProgram();
	planeProgram = new GLSLProgram();
	pointLightDepthProgram = new GLSLProgram();

#ifdef enable_env_cube
	cubemapProgram = new GLSLProgram();
#endif // enable_env_cube

	CompileShaders();

	targetObjectProgram->RegisterUniforms(targetUniformNames);

#ifdef enable_env_cube
	cubemapProgram->RegisterUniforms(cubemapUniformNames);
#endif // enable_env_cube

	planeProgram->RegisterUniforms(planeUniformNames);
}

//-------------------------------------------------------------------------------

void CompileShaders() {
	// base scene shaders
#ifdef enable_env_cube
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
#endif // enable_env_cube

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

		//targetObjectProgram->SetUniform("brdfMode", 0);
		//targetObjectProgram->SetUniform("specularTexture", 1);
		targetObjectProgram->SetUniform("depthMap", 3);
		//targetObjectProgram->SetUniform("skybox", 2);
	}

	// target outline shaders
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (geometryShader == NULL) {
			geometryShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}
		if (!vertexShader->CompileFile(targetOutlineVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(targetOutlineFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}
		if (!geometryShader->CompileFile(targetOutlineGeomShaderPath, GL_GEOMETRY_SHADER)) {
			return;
		}

		targetOutlineProgram->CreateProgram();
		targetOutlineProgram->AttachShader(vertexShader->GetID());
		targetOutlineProgram->AttachShader(fragmentShader->GetID());
		targetOutlineProgram->AttachShader(geometryShader->GetID());
		targetOutlineProgram->Link();
	}

	// pokeball shaders
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}
		if (!vertexShader->CompileFile(pokeballVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(pokeballFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}


		pokeBallProgram->CreateProgram();
		pokeBallProgram->AttachShader(vertexShader->GetID());
		pokeBallProgram->AttachShader(fragmentShader->GetID());
		pokeBallProgram->Link();
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
		planeProgram->SetUniform("diffuseTexture", 0);
		planeProgram->SetUniform("depthMap", 1);
		planeProgram->SetUniform("skybox", 2);
		planeProgram->SetUniform("renderTexture", 3);
	}

	// depth cube shader
	{
		if (vertexShader == NULL) {
			vertexShader = new GLSLShader();
		}
		if (fragmentShader == NULL) {
			fragmentShader = new GLSLShader();
		}
		if (geometryShader == NULL) {
			geometryShader = new GLSLShader();
		}
		if (!vertexShader->CompileFile(pointLightDepthVertShaderPath, GL_VERTEX_SHADER)) {
			return;
		}
		if (!fragmentShader->CompileFile(pointLightDepthFragShaderPath, GL_FRAGMENT_SHADER)) {
			return;
		}
		if (!geometryShader->CompileFile(pointLightDepthGeomShaderPath, GL_GEOMETRY_SHADER)) {
			return;
		}

		pointLightDepthProgram->CreateProgram();
		pointLightDepthProgram->AttachShader(vertexShader->GetID());
		pointLightDepthProgram->AttachShader(geometryShader->GetID());
		pointLightDepthProgram->AttachShader(fragmentShader->GetID());
		pointLightDepthProgram->Link();
		pointLightDepthProgram->SetUniform1("far_plane", 1, &depthFar);
	}

	vertexShader->Delete();
	fragmentShader->Delete();
	geometryShader->Delete();
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------