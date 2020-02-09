#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

# include <GL/glew.h>

//-------------------------------------------------------------------------------

#include "cyMatrix.h"

//-------------------------------------------------------------------------------

Matrix3f NavigationRotationMatrix(Vec2f start, Vec2f end) {
	Matrix3f startRotationMatrix = Matrix3f::RotationXYZ(start.y, start.x, 0);
	Matrix3f endRotationMatrix = Matrix3f::RotationXYZ(end.y, end.x, 0);
	startRotationMatrix.Invert();
	Matrix3f finalRotationMatrix = endRotationMatrix * startRotationMatrix;
	return finalRotationMatrix;
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