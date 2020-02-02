#pragma once
//-------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------