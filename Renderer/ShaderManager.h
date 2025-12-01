#pragma once

#include "Shader.h"

class ShaderManager{
public:
	ShaderManager();
	void Init();
	~ShaderManager();
public:
	Shader MyShader;
	Shader MyInstancingShader;
	Shader MyLightShader;
};
