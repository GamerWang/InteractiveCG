#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

# include <GL/glew.h>

//-------------------------------------------------------------------------------

#include "cyMatrix.h"

//-------------------------------------------------------------------------------

struct Vertex {
	Vec3f Position;
	Vec3f Normal;
	Vec2f Texcoords;

	Vertex() {}
	Vertex(Vec3f p, Vec3f n, Vec2f t) {
		Position = p;
		Normal = n;
		Texcoords = t;
	}
};

//-------------------------------------------------------------------------------

Matrix3f NavigationRotationMatrix(Vec2f start, Vec2f end) {
	Matrix3f startRotationMatrix = Matrix3f::RotationXYZ(start.y, start.x, 0);
	Matrix3f endRotationMatrix = Matrix3f::RotationXYZ(end.y, end.x, 0);
	startRotationMatrix.Invert();
	Matrix3f finalRotationMatrix = endRotationMatrix * startRotationMatrix;
	return finalRotationMatrix;
}

//-------------------------------------------------------------------------------

Matrix4f LookAtMatrix(const Vec3f eyePosition, const Vec3f aimPosition, const Vec3f UP) {
	Matrix4f result;
	
	Vec3f forward = aimPosition - eyePosition;
	forward.Normalize();
	Vec3f side = forward.Cross(UP);
	Vec3f up = side.Cross(forward);
	
	// --------------------------
	result[0] = side.x;
	result[4] = side.y;
	result[8] = side.z;
	result[12] = 0.0f;
	// --------------------------
	result[1] = up.x;
	result[5] = up.y;
	result[9] = up.z;
	result[13] = 0.0f;
	// --------------------------
	result[2] = -forward.x;
	result[6] = -forward.y;
	result[10] = -forward.z;
	result[14] = 0.0f;
	// --------------------------
	result[3] = result[7] = result[11] = 0.0;
	result[15] = 1.0f;

	result = Matrix4f::Translation(-eyePosition) * result;

	return result;
}

//-------------------------------------------------------------------------------

void TextureIDConverter(int id, GLint& outID) {
	switch (id)
	{
	case 0:
		outID = GL_TEXTURE0;
		break;
	case 1:
		outID = GL_TEXTURE1;
		break;
	case 2:
		outID = GL_TEXTURE2;
		break;
	case 3:
		outID = GL_TEXTURE3;
		break;
	case 4:
		outID = GL_TEXTURE4;
		break;
	case 5:
		outID = GL_TEXTURE5;
		break;
	case 6:
		outID = GL_TEXTURE6;
		break;
	case 7:
		outID = GL_TEXTURE7;
		break;
	case 8:
		outID = GL_TEXTURE8;
		break;
	case 9:
		outID = GL_TEXTURE9;
		break;
	case 10:
		outID = GL_TEXTURE10;
		break;
	case 11:
		outID = GL_TEXTURE11;
		break;
	case 12:
		outID = GL_TEXTURE12;
		break;
	case 13:
		outID = GL_TEXTURE13;
		break;
	case 14:
		outID = GL_TEXTURE14;
		break;
	case 15:
		outID = GL_TEXTURE15;
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------------------
//	geometry classes
//-------------------------------------------------------------------------------

class Plane {
protected:
	Vertex vertices[4];
	unsigned int indices[6];
public:
	Plane() {
		vertices[0] = Vertex(Vec3f(-1, 1, 0), Vec3f(0, 0, 0), Vec2f(0, 1));
		vertices[1] = Vertex(Vec3f(-1, -1, 0), Vec3f(0, 0, 0), Vec2f(0, 0));
		vertices[2] = Vertex(Vec3f(1, -1, 0), Vec3f(0, 0, 0), Vec2f(1, 0));
		vertices[3] = Vertex(Vec3f(1, 1, 0), Vec3f(0, 0, 0), Vec2f(1, 1));
		indices[0] = 0; indices[1] = 1; indices[2] = 2;
		indices[3] = 0; indices[4] = 2; indices[5] = 3;
	};

	Vertex* GetVertices() {
		return vertices;
	}
	unsigned int* GetIndices() {
		return indices;
	}
};

//-------------------------------------------------------------------------------