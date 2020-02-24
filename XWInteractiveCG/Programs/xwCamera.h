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

#define camera_bias 0.001f

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
	Vec3f originPosition;
	Vec2f cameraOriginRotation;
public:
	Camera() :
		position(0, 0, 50),
		viewDirection(0, 0, -50),
		UP(0, 1, 0),
		fov(Pi<float>() / 3),
		aspect(1), 
		znear(.1f), 
		zfar(1000), 
		type(XW_CAMERA_PERSPECTIVE), 
		originPosition(0), 
		cameraOriginRotation(0) {}

	Camera(const Camera &c) {
		position = c.position;
		viewDirection = c.viewDirection;
		UP = c.UP;
		fov = c.fov;
		aspect = c.aspect;
		znear = c.znear;
		zfar = c.zfar;
		type = c.type;
		originPosition = c.originPosition;
		cameraOriginRotation = c.cameraOriginRotation;
	}

	Camera(const Camera* c) {
		position = c->position;
		viewDirection = c->viewDirection;
		UP = c->UP;
		fov = c->fov;
		aspect = c->aspect;
		znear = c->znear;
		zfar = c->zfar;
		type = c->type;
		originPosition = c->originPosition;
		cameraOriginRotation = c->cameraOriginRotation;
	}
	
	void Reset();

	void SetAspect(float f) { aspect = f; }
	void SetCameraType(CameraType t) { type = t; }
	CameraType GetCameraType() { return type; }
	void SwitchCameraType() { type = 
		type == XW_CAMERA_PERSPECTIVE ? XW_CAMERA_ORTHOGONAL : XW_CAMERA_PERSPECTIVE; }
	
	Vec3f GetPosition() { return position; }
	void SetPosition(Vec3f p) { position = p; }

	Vec3f GetViewDir() { return viewDirection; }
	void SetViewDir(Vec3f v) { viewDirection = v; }

	void RotateCameraByLocal(Vec2f rotation);
	void RotateCameraByOrigin(Vec2f rotation);
	void MoveCameraAlongView(float moveDistance);
	void ScaleDistanceAlongView(float moveDistance);

	void ReflectOnYPlane(float planeY);

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

inline void Camera::RotateCameraByOrigin(Vec2f rotation) {
	rotation *= Vec2f(-1, 1);
	float rotationAngle = rotation.Length();

	if (rotationAngle > 0) {
		Vec2f nextCameraOriginRotation = cameraOriginRotation + rotation;
		Matrix3f baseRotationMatrix = 
			Matrix3f::RotationXYZ(cameraOriginRotation.y, cameraOriginRotation.x, 0);
		Matrix3f nextRotationMatrix = 
			Matrix3f::RotationXYZ(nextCameraOriginRotation.y, nextCameraOriginRotation.x, 0);

		baseRotationMatrix.Invert();
		Matrix3f finalRotationMatrix = nextRotationMatrix * baseRotationMatrix;

		Vec3f relativePosition = position - originPosition;
		relativePosition = finalRotationMatrix * relativePosition;
		position = relativePosition + originPosition;
		viewDirection = finalRotationMatrix * viewDirection;
		UP = finalRotationMatrix * UP;

		cameraOriginRotation = nextCameraOriginRotation;
	}

}

//-------------------------------------------------------------------------------

inline void Camera::MoveCameraAlongView(float moveDistance) {
	position = position + viewDirection.GetNormalized() * moveDistance;
}

//-------------------------------------------------------------------------------

inline void Camera::ScaleDistanceAlongView(float moveDistance) {
	Vec3f aimingPoint = position + viewDirection;
	if (moveDistance - viewDirection.Length() > camera_bias) {
		moveDistance = viewDirection.Length() - camera_bias;
	}
	viewDirection = viewDirection - viewDirection.GetNormalized() * moveDistance;
	position = aimingPoint - viewDirection;
}

//-------------------------------------------------------------------------------

inline void Camera::ReflectOnYPlane(float planeY) {
	position -= Vec3f(0, 2 * (position.y - planeY), 0);
	viewDirection *= Vec3f(1, -1, 1);
	UP *= Vec3f(1, -1, 1);
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------