#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <vector>

class Model
{
public:
	Model();
	void Bind();
	void Unbind();
	virtual void CreateVAO() = 0;
	virtual void Draw() = 0;
	virtual ~Model();

protected:
	float const PI = 3.141592f;
	GLuint VaoId, VboId, EboId;
};

