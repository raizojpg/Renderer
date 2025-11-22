#pragma once
#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Camera
{
public:
	Camera();

	void Update();

	void setObs(float x, float y, float z);
	glm::vec3 getObs();

	void setRef(float x, float y, float z);
	glm::vec3 getRef();

	void setVert(float x, float y, float z);
	glm::vec3 getVert();

	void setView();
	glm::mat4 getView();

	void setProjection();
	glm::mat4 getProjection();


	float& widthR();
	float& heightR();
	float& distR();
	float& alphaR();
	float& betaR();

private:
	glm::mat4 view, projection;
	//perspective
	float width, height, znear, fov;
	//view
	glm::vec3 Obs, Ref, Vert;
	float Refx, Refy, Refz;
	float Obsx, Obsy, Obsz;
	float Vx, Vy, Vz;
	//movement
	float alpha, beta, dist; 
};