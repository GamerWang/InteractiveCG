#pragma once
//-------------------------------------------------------------------------------

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
using namespace std;

//-------------------------------------------------------------------------------

#include "cyVector.h";
#include "cyTriMesh.h";
#include "cyGL.h"
#include "cyMatrix.h";
using namespace cy;

//-------------------------------------------------------------------------------

struct xwVertex {
	Vec3f Position;
	Vec3f Normal;
	Vec2f TexCoords;
	Vec3f Tangent;
	Vec3f Bitangent;
};

struct Texture {
	GLuint id;
	string type;
	string path;
};

//-------------------------------------------------------------------------------

class Mesh {
public:
	vector<xwVertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	GLuint VAO;

	/* Functions */
	// constructor
	Mesh(vector<xwVertex> vertices, vector<GLuint> indices, vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}
	// render the mesh
	void Draw(GLSLProgram* program) {
		// bind appropriate textures

		if (textures.size() > 0) {
			glActiveTexture(GL_TEXTURE0 + textures[0].id);
			glBindTexture(GL_TEXTURE_2D, textures[0].id);
			program->SetUniform("diffuseTexture", (int)textures[0].id);

			glActiveTexture(GL_TEXTURE0 + textures[1].id);
			glBindTexture(GL_TEXTURE_2D, textures[1].id);
			program->SetUniform("matcapTexture", (int)textures[1].id);

			glActiveTexture(GL_TEXTURE0 + textures[2].id);
			glBindTexture(GL_TEXTURE_2D, textures[2].id);
			program->SetUniform("matcapMask", (int)textures[2].id);
		}

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}
private:
	GLuint VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(xwVertex), 
			&vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
			&indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
			sizeof(xwVertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
			sizeof(xwVertex), (void*)offsetof(xwVertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 
			sizeof(xwVertex), (void*)offsetof(xwVertex, TexCoords));
		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 
			sizeof(xwVertex), (void*)offsetof(xwVertex, Tangent));
		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 
			sizeof(xwVertex), (void*)offsetof(xwVertex, Bitangent));

		glBindVertexArray(0);
	}
};

//-------------------------------------------------------------------------------