#include "ShaderManager.h"

ShaderManager::ShaderManager(){}

void ShaderManager::Init(){
	MyShader.Create("Shader.vert", "Shader.frag");
	MyInstancingShader.Create("InstancingShader.vert", "InstancingShader.frag");
	MyLightShader.Create("LightShader.vert", "LightShader.frag");
}



ShaderManager::~ShaderManager(){}
