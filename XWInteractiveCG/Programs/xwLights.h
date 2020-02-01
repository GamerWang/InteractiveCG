#pragma once
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "cyVector.h";

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
	Vec3f position;
	Vec3f intensity;
public:
	PointLight() : intensity(1), position(0) {}
	virtual Vec3f GetIntensity() { return intensity; }
	virtual void SetIntensity(Vec3f i) { intensity = i; }
	virtual void SetIntensity(float i) { intensity = Vec3f(i); }
	virtual Vec3f GetPosition() { return position; }
	virtual void SetPosition(Vec3f p) { position = p; }
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