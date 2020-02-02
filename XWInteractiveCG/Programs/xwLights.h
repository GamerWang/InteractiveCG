#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "cyVector.h";

//-------------------------------------------------------------------------------

#include "xwHelper.h";

//-------------------------------------------------------------------------------

class Light
{
protected:
	Vec3f intensity;
public:
	Light() : intensity(1) {}
	virtual Vec3f GetIntensity() { return intensity; }
	virtual void SetIntensity(Vec3f i) { intensity = i; }
	virtual void SetIntensity(float i) { intensity = Vec3f(i); }
};

//-------------------------------------------------------------------------------

class PointLight : Light {
protected:
	Vec3f intensity;
	Vec3f position;
	Vec3f origin;
	Vec2f rotation;
public:
	PointLight() : intensity(1), position(0), origin(0), rotation(0) {}
	virtual Vec3f GetIntensity() { return intensity; }
	virtual void SetIntensity(Vec3f i) { intensity = i; }
	virtual void SetIntensity(float i) { intensity = Vec3f(i); }
	virtual Vec3f GetPosition() { return position; }
	virtual void SetPosition(Vec3f p) { position = p; }
	virtual void SetRotation(Vec2f r);
	virtual void Rotate(Vec2f rotation);
};

//-------------------------------------------------------------------------------

class DirectionalLight : Light {
protected:
	Vec3f direction;
	Vec3f intensity;
public:
	DirectionalLight() : intensity(1), direction(1) {}
	virtual Vec3f GetIntensity() { return intensity; }
	virtual void SetIntensity(Vec3f i) { intensity = i; }
	virtual void SetIntensity(float i) { intensity = Vec3f(i); }
	virtual Vec3f GetDirection() { return direction; }
	virtual void SetDirection(Vec3f d) { direction = d; }
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

inline void PointLight::SetRotation(Vec2f r) {
	Rotate(r - rotation);
}

//-------------------------------------------------------------------------------

inline void PointLight::Rotate(Vec2f r) {
	r *= Vec2f(1, -1);
	float rotationAngle = r.Length();
	if (rotationAngle > 0) {
		Matrix3f rotationMatrix = NavigationRotationMatrix(rotation, rotation + r);

		Vec3f relativePosition = position - origin;
		relativePosition = rotationMatrix * relativePosition;
		position = relativePosition + origin;
		rotation = rotation + r;
	}
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------