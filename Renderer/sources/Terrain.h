#pragma once
#include "Model.h"

class Terrain : public Model{
public:
	Terrain(int w, int l, int s = 500);
	void CreateVAO() override;
	void Draw() override;

	int getWidth();
	int getLength();
	int getMaxHeight();
	int getHeightmapTex();
	glm::mat4 getTerrainMat();

	void loadHightmap();
	void updateLodMap(glm::vec3 obs);
	void updateShader(Shader& MyShader) override;
		
	~Terrain();

private:
	const int WIDTH, LENGTH, NR_VF;
	int step;
	float maxHeight;
	glm::mat4 terrainMat;
	unsigned char* heightData;
	int imgWidth, imgHeight;
	int patchSize, maxLod;
	GLuint heightmapTex;

	struct BufferInfo {
		int start;
		int count;
	};

	struct LodInfo {
		BufferInfo border[2][4];
		BufferInfo center;
	};

	std::vector<LodInfo> lods;
	std::vector<std::vector<short>> lodMap;
};

