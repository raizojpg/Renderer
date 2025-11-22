#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Shader
{
public:
	Shader();
	void Create(const char* vs, const char* fs);
	void Bind();

	void setUniformMat4(const char* loc, glm::mat4 value);
	void setUniformInt(const char* loc, int value);
	~Shader();

private:
	GLuint ProgramId;
};

