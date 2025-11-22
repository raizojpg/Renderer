#include "Shader.h"
#include "loadShaders.h"

Shader::Shader(){}

void Shader::Create(const char* vs, const char* fs){
	ProgramId = LoadShaders(vs, fs);
	glUseProgram(ProgramId);
}

void Shader::Bind() {
	glUseProgram(ProgramId);
}

void Shader::setUniformMat4(const char* loc, glm::mat4 value) {
	GLuint location = glGetUniformLocation(ProgramId, loc);
	glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniformInt(const char* loc, int value) {
	GLuint location = glGetUniformLocation(ProgramId, loc);
	glUniform1i(location, value);
}

Shader::~Shader()
{
	glDeleteProgram(ProgramId);
}
