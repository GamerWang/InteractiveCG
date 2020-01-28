#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include <stdio.h>

//-------------------------------------------------------------------------------

#include "cyMatrix.h";

//-------------------------------------------------------------------------------

enum CameraType {
	XW_CAMERA_PERSPECTIVE,
	XW_CAMERA_ORTHOGONAL
};

//-------------------------------------------------------------------------------
class Camera {
	Vec3f position;
	Vec3f viewDirection;
	Vec3f UP;
	float fov;
	float aspect;
	float znear;
	float zfar;
	CameraType type;
public:
	Camera() :
		position(0, 0, 70),
		viewDirection(0, 0, -70),
		UP(0, 1, 0),
		fov(Pi<float>() / 3),
		aspect(.75f), 
		znear(.1f), 
		zfar(1000), 
		type(CameraType::XW_CAMERA_PERSPECTIVE) {}
	Matrix4f WorldToViewMatrix();
	Matrix4f ViewToProjectionMatrix();
	CameraType GetCameraType() { return type; }
	void RotateCameraByLocal(Vec2f rotation);
	void RotateCameraByTarget(Vec2f rotation);
	void MoveCameraAlongView(float moveDistance);
};

//-------------------------------------------------------------------------------

inline Matrix4f Camera::WorldToViewMatrix() {
	return Matrix4f::View(position, position + viewDirection, UP);
}

//-------------------------------------------------------------------------------

inline Matrix4f Camera::ViewToProjectionMatrix() {
	return Matrix4f::Perspective(fov, aspect, znear, zfar);
}

//-------------------------------------------------------------------------------

inline void Camera::RotateCameraByLocal(Vec2f rotation) {
	float rotationAngle = rotation.Length();
	Vec3f yDir = UP.GetNormalized();
	Vec3f zDir = (-viewDirection).GetNormalized();
	Vec3f xDir = yDir.Cross(zDir);
	Vec3f rotationAxis = (yDir * rotation.y + xDir * rotation.x).Cross(-zDir);
	rotationAxis.Normalize();
	if (rotationAngle > 0) {
		Matrix3f cameraRotationMatrix = Matrix3f::Rotation(rotationAxis, rotationAngle);
		//UP = cameraRotationMatrix * UP;
		viewDirection = cameraRotationMatrix * viewDirection;
	}
}

//-------------------------------------------------------------------------------

inline void Camera::RotateCameraByTarget(Vec2f rotation) {

}

//-------------------------------------------------------------------------------

inline void Camera::MoveCameraAlongView(float moveDistance) {
	position = position + viewDirection.GetNormalized() * moveDistance;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------