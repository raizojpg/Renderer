#include "ShaderManager.h"

ShaderManager::ShaderManager(){}

void ShaderManager::Init(){
	MyShader.Create("shaders/Shader.vert", "shaders/Shader.frag");
	MyInstancingShader.Create("shaders/InstancingShader.vert", "shaders/InstancingShader.frag");
	MyLightShader.Create("shaders/LightShader.vert", "shaders/LightShader.frag");
	MyTerrainShaderHeightmap.Create("shaders/TerrainShaderHeightmap.vert", "shaders/TerrainShaderHeightmap.frag");
	MyTerrainShaderNoise.Create("shaders/TerrainShaderNoise.vert", "shaders/TerrainShaderNoise.frag");
	MyVegetationShader.Create("shaders/VegetationShader.vert", "shaders/VegetationShader.frag");
}

void ShaderManager::UpdateTerrainNoise(Terrain& MyTerrain, Camera& MyCamera, Light& MyLight){
	Shader& MyShader = MyTerrainShaderNoise;
	
	MyShader.updateLight(MyLight);
	MyShader.setUniformVec3("viewPos", glm::vec4(MyCamera.getObs(), 0) - MyTerrain.getTerrainMat()[3]);
	MyShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
	
	MyShader.setUniformMat4("modelMatrix", MyTerrain.getTerrainMat());
	MyShader.setUniformInt("codCol", 0);
	MyShader.updateMaterial(MyTerrain.getMaterial());

	MyShader.setUniformInt("usingNoise", 1);
	MyShader.setUniformFloat("uMaxHeight", MyTerrain.getMaxHeight());
	MyShader.setUniformFloat("uNoiseScale", vNoiseScale);
	MyShader.setUniformInt("uOctaves", vOctaves);
	MyShader.setUniformFloat("uFrequency", vFrequency);
	MyShader.setUniformFloat("uAmplitude", vAmplitude);
	MyShader.setUniformFloat("uLacunarity", vLacunarity);
	MyShader.setUniformFloat("uGain", vGain);

	MyShader.setUniformFloat("uFogStart", vFogStart);
	MyShader.setUniformFloat("uFogEnd", vFogEnd);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MyTerrain.getHeightmapTex());
	MyShader.setUniformInt("heightmap", 0);
	MyShader.setUniformFloat("heightmapScale", (MyTerrain.getPatchSize() - 1) * (MyTerrain.getPatchSize() - 1) * MyTerrain.getStep() / 2);

}

void ShaderManager::UpdateVegetation(Terrain& MyTerrain, Camera& MyCamera, Light& MyLight) {
	Shader& MyShader = MyVegetationShader;

	MyShader.updateLight(MyLight);
	MyShader.setUniformVec3("viewPos", MyCamera.getObs());
	MyShader.setUniformMat4("viewMatrix", MyCamera.getView());
	MyShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());

	MyShader.setUniformMat4("modelMatrix", MyTerrain.getTerrainMat());
	MyShader.setUniformInt("codCol", 0);
	MyShader.updateMaterial(MyTerrain.getMaterial());

	MyShader.setUniformInt("usingNoise", 1);
	MyShader.setUniformFloat("uMaxHeight", MyTerrain.getMaxHeight());
	MyShader.setUniformFloat("uNoiseScale", vNoiseScale);
	MyShader.setUniformFloat("uNoiseScale", vNoiseScale);
	MyShader.setUniformInt("uOctaves", vOctaves);
	MyShader.setUniformFloat("uFrequency", vFrequency);
	MyShader.setUniformFloat("uAmplitude", vAmplitude);
	MyShader.setUniformFloat("uLacunarity", vLacunarity);
	MyShader.setUniformFloat("uGain", vGain);

	MyShader.setUniformFloat("uFogStart", vFogStart);
	MyShader.setUniformFloat("uFogEnd", vFogEnd);

}


ShaderManager::~ShaderManager(){}
