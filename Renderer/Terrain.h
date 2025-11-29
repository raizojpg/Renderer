#pragma once
#include "Model.h"

class Terrain : public Model{
public:
	Terrain(int w, int l, int s = 500);
	void CreateVAO() override;
	void Draw() override;

	int getWidth();
	int getLength();
	glm::mat4 getTerrainMat();

	~Terrain();

private:
	const int WIDTH, LENGTH, NR_VF;
	int step;
	float maxHeight;
	glm::mat4 terrainMat;
	unsigned char* heightData;
	int imgWidth, imgHeight;
};

