#pragma once
#include "../Model.h"

class Skybox : public Model {
public:
	Skybox();
	void CreateVAO() override;
	void Draw() override;
	~Skybox();

private:
	GLuint VbPos, VbCol, VbMat;
};

