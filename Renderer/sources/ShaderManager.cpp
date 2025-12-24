#include "ShaderManager.h"

ShaderManager::ShaderManager(){}

void ShaderManager::Init(){
	MyShader.Create("shaders/Shader.vert", "shaders/Shader.frag");
	MyInstancingShader.Create("shaders/InstancingShader.vert", "shaders/InstancingShader.frag");
	MyLightShader.Create("shaders/LightShader.vert", "shaders/LightShader.frag");
	MyTerrainShaderHeightmap.Create("shaders/TerrainShaderHeightmap.vert", "shaders/TerrainShaderHeightmap.frag");
	MyTerrainShaderNoise.Create("shaders/TerrainShaderNoise.vert", "shaders/TerrainShaderNoise.frag");
}



ShaderManager::~ShaderManager(){}
