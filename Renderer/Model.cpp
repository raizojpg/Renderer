#include "Model.h"

Model::Model() {}

void Model::Bind() {
	glBindVertexArray(VaoId);
}

void Model::Unbind() {
	glBindVertexArray(0);
}

Model::~Model() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glBindVertexArray(0);
}