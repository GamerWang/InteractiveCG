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

#define polar_angle_bias 0.001f

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
	Vec3f originPoint;
public:
	Camera() :
		position(0, 0, 50),
		viewDirection(0, 0, -50),
		UP(0, 1, 0),
		fov(Pi<float>() / 3),
		aspect(1), 
		znear(.1f), 
		zfar(1000), 
		type(XW_CAMERA_PERSPECTIVE) {}
	
	void Reset();

	void SetAspect(float f) { aspect = f; }
	void SetCameraType(CameraType t) { type = t; }
	CameraType GetCameraType() { return type; }
	void SwitchCameraType() { type = 
		type == XW_CAMERA_PERSPECTIVE ? XW_CAMERA_ORTHOGONAL : XW_CAMERA_PERSPECTIVE; }
	
	void RotateCameraByLocal(Vec2f rotation);
	void RotateCameraByTarget(Vec2f rotation);
	void MoveCameraAlongView(float moveDistance);

	Matrix4f WorldToViewMatrix();
	Matrix4f ViewToProjectionMatrix();
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

inline void Camera::Reset() {
	position = Vec3f(0, 0, 50);
	viewDirection = Vec3f(0, 0, -50);
	UP = Vec3f(0, 1, 0);
	fov = Pi<float>() / 3;
	aspect = 1;
	znear = .1f;
	zfar = 1000;
	type = XW_CAMERA_PERSPECTIVE;
}

//-------------------------------------------------------------------------------

inline void Camera::RotateCameraByLocal(Vec2f rotation) {
	float rotationAngle = rotation.Length();
	Vec3f yDir = UP.GetNormalized();
	Vec3f zDir = (-viewDirection).GetNormalized();
	Vec3f xDir = yDir.Cross(zDir);
	yDir = zDir.Cross(xDir);
	Vec3f rotationAxis = (yDir * rotation.y + xDir * rotation.x).Cross(-zDir);
	rotationAxis.Normalize();
	if (rotationAngle > 0) {
		Matrix3f cameraRotationMatrix = Matrix3f::Rotation(rotationAxis, rotationAngle);
		viewDirection = cameraRotationMatrix * viewDirection;
	}
}

//-------------------------------------------------------------------------------

inline void Camera::RotateCameraByTarget(Vec2f rotation) {
	rotation *= -1;
	float rotationAngle = rotation.Length();
	Vec3f yDir = UP.GetNormalized();
	Vec3f zDir = (-viewDirection).GetNormalized();
	Vec3f xDir = yDir.Cross(zDir);
	yDir = zDir.Cross(xDir);
	Vec3f rotationAxis = (yDir * rotation.y + xDir * rotation.x).Cross(-zDir);
	rotationAxis.Normalize();
	Vec3f aimingPoint = position + viewDirection;
	Vec3f relativePosition = -viewDirection;
	if (rotationAngle > 0) {
		Matrix3f cameraRotationMatrix = Matrix3f::Rotation(rotationAxis, rotationAngle);
		viewDirection = cameraRotationMatrix * viewDirection;
		position = aimingPoint - viewDirection;
	}
}

//-------------------------------------------------------------------------------

inline void Camera::MoveCameraAlongView(float moveDistance) {
	position = position + viewDirection.GetNormalized() * moveDistance;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------