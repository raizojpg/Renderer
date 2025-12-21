#include "ShaderManager.h"

ShaderManager::ShaderManager(){}

void ShaderManager::Init(){
	MyShader.Create("shaders/Shader.vert", "shaders/Shader.frag");
	MyInstancingShader.Create("shaders/InstancingShader.vert", "shaders/InstancingShader.frag");
	MyLightShader.Create("shaders/LightShader.vert", "shaders/LightShader.frag");
	MyTerrainShader.Create("shaders/TerrainShader.vert", "shaders/TerrainShader.frag");
}



ShaderManager::~ShaderManager(){}
