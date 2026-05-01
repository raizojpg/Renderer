#pragma once
#include "../Model.h"
#include "../LSystem.h"
#include "../MeshBuilder.h"

class Tree : public Model{
public:
	Tree();
	void CreateVAO() override;
	void CreateVAO(TreeGenParams& p, int seed);
	void Draw(Shader* MyShader = nullptr) override;
	void DrawEdges();
	~Tree();

private:
	void DrawInstanced();
	void DrawNonInstanced();

	GLuint VbPos, VbCol, VbMat, VbNorm, VbUV, IndexCount;
	std::vector<glm::vec3> InstanceColors;
	std::vector<glm::mat4> InstanceMatrices;
};
