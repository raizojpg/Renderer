#pragma once
#include "Model.h"
#include "VegetationManager.h"

class Terrain : public Model{
public:
	Terrain(int w, int l, int s = 500);
	void CreateVAO() override;
	void Draw(Shader* MyShader = nullptr) override;
	void DrawVegetation(Shader* MyShader = nullptr);

	int getWidth();
	int getLength();
	int getStep();
	int getPatchSize();
	int getMaxHeight();
	GLuint getHeightmapTex();
	GLuint getGrassTexture();
	GLuint getRockTexture();
	GLuint getSnowTexture();
	GLuint getBarkTexture();
	GLuint getSkyTexture();
	glm::mat4 getTerrainMat();

	void loadHightmap();
	void loadTerrainTextures();
	void updateMap(Camera& MyCamera);
	void updateLodMap(glm::vec3 obs);
	void updateCullMap(Camera& MyCamera);
		
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
	GLuint grassTexture;
	GLuint rockTexture;
	GLuint snowTexture;
	GLuint barkTexture;
	GLuint skyTexture;

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
	std::vector<std::vector<bool>> cullMap;

	VegetationManager vegetation;

	GLuint loadTerrainTexture(const char* path, const glm::vec3& fallbackColor);
};

