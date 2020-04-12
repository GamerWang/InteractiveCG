#pragma once
//-------------------------------------------------------------------------------

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
using namespace std;

//-------------------------------------------------------------------------------

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//-------------------------------------------------------------------------------

#include "cyVector.h";
#include "cyTriMesh.h";
#include "cyGL.h"
#include "cyMatrix.h";
using namespace cy;

//-------------------------------------------------------------------------------

#include "xwMesh.h";
#include "xwMaterial.h";

//-------------------------------------------------------------------------------

//#define MODEL_USE_FUKA 0

#define MODEL_USE_MAGI 0

//-------------------------------------------------------------------------------

class Model {
public:
	/* Model Data */
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	/* Functions */
	// constructor, expects a filepath to a 3D model
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
	}

	void Draw(GLSLProgram* program) {
		for (GLuint i = 0; i < meshes.size(); i++) {
			meshes[i].Draw(program);
		}
	}
private:
	/* Functions */
	// loads a model with supported ASSIMP extensions
	void loadModel(string const &path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, 
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_CalcTangentSpace
		);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filePath
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion.
	void processNode(aiNode *node, const aiScene *scene) {
		// process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// recursively process children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene) {
		// data to fill
		vector<xwVertex> vertices;
		vector<GLuint>	 indices;

		// walk through each vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++) {
			xwVertex vertex;
			// positions
			vertex.Position = Vec3f(
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z);
			// normals
			vertex.Normal = Vec3f(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z);
			// texture coordinates
			if (mesh->mTextureCoords[0]) {
				vertex.TexCoords = Vec2f(
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y);
				//cout << "current texcoord: "
				//	<< vertex.TexCoords.x << ","
				//	<< vertex.TexCoords.y << endl;
			}
			else
				vertex.TexCoords = Vec2f(.0f, .0f);
			// tangent
			vertex.Tangent = Vec3f(
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z);
			// bitangent
			vertex.Bitangent = Vec3f(
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z);
			vertices.push_back(vertex);
		}
		// walk through faces
		for (GLuint i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}
		// process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> maps = 
			loadMaterialTextures(material, aiTextureType_DIFFUSE, "Fuka_Tex");

		return Mesh(vertices, indices, maps);
	}

	// checks all material textures
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			string textureName = string(str.C_Str());

#ifdef MODEL_USE_MAGI
			string body = "Body";
			string eye = "Eye";

			size_t fFound = textureName.find(body);
			size_t eFound = textureName.find(eye);

			// Magikarp texture read
			{
				string matCapDir = "MagiTex\\Magi_Body_Matcap.png";

				if (textureName.find(body) != string::npos) {
					Texture diffuseMap, matCap, matCapMask;
					string diffuseDir = "MagiTex\\Magi_Body_Diffuse.png";
					diffuseMap.id = TextureFromFile(diffuseDir);
					diffuseMap.path = diffuseDir;
					diffuseMap.type = typeName;
					textures.push_back(diffuseMap);

					matCap.id = TextureFromFile(matCapDir);
					matCap.path = matCapDir;
					matCap.type = typeName;
					textures.push_back(matCap);

					string matMaskDir = "MagiTex\\Magi_Body_Matcap_Mask.png";
					matCapMask.id = TextureFromFile(matMaskDir);
					matCapMask.path = matMaskDir;
					matCapMask.type = typeName;
					textures.push_back(matCapMask);

				}else if (textureName.find(eye) != string::npos) {
					Texture diffuseMap, matCap, matCapMask;
					string diffuseDir = "MagiTex\\Magi_Eye_Diffuse.png";
					diffuseMap.id = TextureFromFile(diffuseDir);
					diffuseMap.path = diffuseDir;
					diffuseMap.type = typeName;
					textures.push_back(diffuseMap);

					matCap.id = TextureFromFile(matCapDir);
					matCap.path = matCapDir;
					matCap.type = typeName;
					textures.push_back(matCap);

					string matMaskDir = "MagiTex\\Magi_Eye_Matcap_Mask.png";
					matCapMask.id = TextureFromFile(matMaskDir);
					matCapMask.path = matMaskDir;
					matCapMask.type = typeName;
					textures.push_back(matCapMask);
				}
			}
#endif

#ifdef MODEL_USE_FUKA
			string face = "Face";
			string body = "Body";
			string hair = "Hair";

			size_t fFound = textureName.find(face);
			size_t bFound = textureName.find(body);
			size_t hFound = textureName.find(hair);

			cout << str.C_Str() << endl;

			// Fuka texture read
			{
				if (textureName.find(face) != string::npos) {
					Texture diffuseMap, lightMap;
					string diffuseDir =  "FukaTex\\Fuka_Face_BaseColor.png";
					diffuseMap.id = TextureFromFile(diffuseDir);
					diffuseMap.path = diffuseDir;
					diffuseMap.type = typeName;
					textures.push_back(diffuseMap);
				}
				else if (textureName.find(body) != string::npos) {
					Texture diffuseMap, lightMap;
					string diffuseDir = "FukaTex\\Fuka_Body_BaseColor.png";
					diffuseMap.id = TextureFromFile(diffuseDir);
					diffuseMap.path = diffuseDir;
					diffuseMap.type = typeName;
					textures.push_back(diffuseMap);
				}
				else if (textureName.find(hair) != string::npos) {
					Texture diffuseMap, lightMap;
					string diffuseDir = "FukaTex\\Fuka_Hair_BaseColor.png";
					diffuseMap.id = TextureFromFile(diffuseDir);
					diffuseMap.path = diffuseDir;
					diffuseMap.type = typeName;
					textures.push_back(diffuseMap);
				}
			}
#endif 
		}
		return textures;
	}

	GLuint TextureFromFile(const string &directory) {

		GLuint textureID;
		glGenTextures(1, &textureID);

		int width, height;
		Material::Texture currentTex = Material::Texture(directory.c_str());

		width = currentTex.width;
		height = currentTex.height;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, currentTex.textureData.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		return textureID;
	}
};

//-------------------------------------------------------------------------------
