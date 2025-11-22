#include "Camera.h"
#include <math.h>

Camera::Camera() {

	width = 800, height = 600, znear = 1, fov = 30;

	Refx = 0.0f, Refy = 0.0f, Refz = 0.0f;
	Obsx = 0.0f, Obsy = 0.0f, Obsz = -300.0f;
	Vx = 0.0f, Vy = 0.0f, Vz = 1.0f;

	alpha = 0.0f, beta = 0.0f, dist = 1500.0f;

}

void Camera::Update() {
	this->setRef(Refx, Refy, Refz);
	this->setObs(
		Refx + dist * cos(alpha) * cos(beta),
		Refy + dist * cos(alpha) * sin(beta),
		Refz + dist * sin(alpha)
	);
	this->setVert(Vx, Vy, Vz);
	this->setView();
	this->setProjection();
}


void Camera::setObs(float x, float y, float z){
	Obs = glm::vec3(x, y, z);
}
glm::vec3 Camera::getObs(){ return Obs; }

void Camera::setRef(float x, float y, float z){
	Ref = glm::vec3(x, y, z);
}
glm::vec3 Camera::getRef(){ return Ref; }

void Camera::setVert(float x, float y, float z){
	Vert = glm::vec3(x, y, z);
}
glm::vec3 Camera::getVert() { return Vert; }


void Camera::setView() {
	view = glm::lookAt(Obs, Ref, Vert);
}
glm::mat4 Camera::getView() { return view; }


void Camera::setProjection() {
	projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
}
glm::mat4 Camera::getProjection() { return projection; }

float& Camera::widthR() { return width; }
float& Camera::heightR() { return height; }
float& Camera::distR() { return dist; }
float& Camera::alphaR() { return alpha; }
float& Camera::betaR() { return beta; }
