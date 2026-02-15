#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "sources/ModelManager.h"
#include "sources/ShaderManager.h"
#include "sources/InputManager.h"

#include "PerlinNoise.hpp"

ModelManager* models = nullptr;
ShaderManager* shaders = nullptr;
LightManager lights;

Camera MyCamera;
InputManager inputs(MyCamera);

GLint winWidth = 1000, winHeight = 600;

void ProcessNormalKeys(unsigned char key, int x, int y) {
	ImGui_ImplGLUT_KeyboardFunc(key, x, y);
	if (!ImGui::GetIO().WantCaptureKeyboard) {
		inputs.ProcessNormalKeys(key, x, y);
	}
}

void ProcessSpecialKeys(int key, int x, int y) {
	ImGui_ImplGLUT_SpecialFunc(key, x, y);
	if (!ImGui::GetIO().WantCaptureKeyboard) {
		inputs.ProcessSpecialKeys(key, x, y);
	}
}

void MouseButton(int button, int state, int x, int y) {
	ImGui_ImplGLUT_MouseFunc(button, state, x, y);
	if (!ImGui::GetIO().WantCaptureMouse) {
		inputs.MouseButton(button, state, x, y);
	}
}

void MouseMotion(int x, int y) {
	ImGui_ImplGLUT_MotionFunc(x, y);
	if (!ImGui::GetIO().WantCaptureMouse) {
		inputs.MouseMotion(x, y);
	}
}

void ReshapeFunction(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
	winWidth = newWidth;
	winHeight = newHeight;
	MyCamera.widthR() = winWidth / 10, MyCamera.heightR() = winHeight / 10;
	
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(winWidth), static_cast<float>(winHeight));
}

void Cleanup(void){
	
	if (shaders != nullptr) {
		delete shaders;
		shaders = nullptr;
	}

	if (models != nullptr)
	{
		delete models;
		models = nullptr;
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();
}

void Initialize(void)
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	
	if (shaders == nullptr)
	{
		shaders = new ShaderManager();
		shaders->Init();
	}
	
	if (models != nullptr)
	{
		delete models;
	}
	models = new ModelManager();
	models->Initialize();
}

void ShowMyImGuiWindow()
{
	static bool needsInitialization = false;
	bool freezeSimulation = false;

	const char* terrainSizes[] = { "128", "256", "512", "1024", "2048", "4096" };
	static int terrainSizeIdx = 4;

	const char* patchSizes[] = { "4", "8", "16", "32", "64", "128", "256" };
	static int patchSizeIdx = 5;

	ImGui::Begin("Options");

	// General
	ImGui::SliderInt("Speed", &vSpeed, 1, 2000);
	needsInitialization |= ImGui::SliderInt("Seed", &vSeed, 1, 5000);
	freezeSimulation |= ImGui::IsItemActive();

	// Skybox & Fog
	ImGui::Text("Skybox & Fog");
	ImGui::SliderInt("Skybox Scale", &vSkyboxScale, 1, 400);
	ImGui::SliderInt("Fog Start", &vFogStart, 0, 100000);
	ImGui::SliderInt("Fog End", &vFogEnd, 0, 200000);

	ImGui::Separator();

	// Terrain
	ImGui::Text("Terrain");
	needsInitialization |= ImGui::Combo("Terrain Size", &terrainSizeIdx, terrainSizes, IM_ARRAYSIZE(terrainSizes));
	vTerrainSize = atoi(terrainSizes[terrainSizeIdx]);
	freezeSimulation |= ImGui::IsItemActive();

	needsInitialization |= ImGui::SliderInt("Terrain Step", &vTerrainStep, 5, 500);
	freezeSimulation |= ImGui::IsItemActive();

	needsInitialization |= ImGui::SliderInt("Max Height", &vMaxHeight, -5000, 5000);
	freezeSimulation |= ImGui::IsItemActive();

	ImGui::Separator();

	// LOD & Patches
	ImGui::Text("LOD & Patches");
	needsInitialization |= ImGui::Combo("Patch Size", &patchSizeIdx, patchSizes, IM_ARRAYSIZE(patchSizes));
	vPatchSize = atoi(patchSizes[patchSizeIdx]);
	freezeSimulation |= ImGui::IsItemActive();

	needsInitialization |= ImGui::SliderInt("Max LOD", &vMaxLod, 1, 10);
	freezeSimulation |= ImGui::IsItemActive();

	ImGui::SliderInt("LOD Distribution", &vLodDistribution, 1, 5);

	ImGui::Separator();

	// Vegetation
	ImGui::Text("Vegetation");
	ImGui::SliderInt("Tree LOD Distribution", &vTreeLodDistribution, 1, 5);
	ImGui::SliderInt("Tree Instance Count", &vTreeInstanceCount, 0, 64);
	ImGui::SliderFloat("Lower Tree Threshold", &vLowerTreeTreshold, 0.0f, 1.0f);
	ImGui::SliderFloat("Upper Tree Threshold", &vUpperTreeTreshold, 0.0f, 1.0f);

	ImGui::Separator();

	// Noise
	ImGui::Text("Noise");
	ImGui::SliderFloat("Noise Scale", &vNoiseScale, 0.00001f, 0.001f, "%.05f");
	ImGui::SliderInt("Octaves", &vOctaves, 1, 10);
	ImGui::SliderFloat("Frequency", &vFrequency, 0.01f, 10.0f);
	ImGui::SliderFloat("Amplitude", &vAmplitude, 0.01f, 10.0f);
	ImGui::SliderFloat("Lacunarity", &vLacunarity, 0.01f, 10.0f);
	ImGui::SliderFloat("Gain", &vGain, 0.0f, 1.0f);

	// Biomes
	ImGui::Text("Biomes");
	ImGui::Checkbox("Use Biomes", &vUseBiomes);
	ImGui::SliderInt("Biome Octaves", &vBiomeOctaves, 1, 10);
	ImGui::SliderFloat("Biome Frequency", &vBiomeFrequency, 0.001f, 1.0f);
	ImGui::SliderFloat("Biome Amplitude", &vBiomeAmplitude, 1.0f, 20.0f);
	ImGui::SliderFloat("Biome Lacunarity", &vBiomeLacunarity, 0.01f, 10.0f);
	ImGui::SliderFloat("Biome Gain", &vBiomeGain, 0.0f, 1.0f);

	// Reinitialization trigger
	if (needsInitialization && !freezeSimulation)
	{
		Initialize();
		needsInitialization = false;
	}

	ImGui::End();
}

void RenderHud(float fps)
{
	char fpsText[64];
	sprintf_s(fpsText, "FPS: %.f", fps);

	GLint prevProgram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

	glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TRANSFORM_BIT);

	glUseProgram(0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_SCISSOR_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, winWidth, 0, winHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(1.0f, 1.0f, 1.0f);

	glWindowPos2i(10, winHeight - 20);

	for (const char* c = fpsText; *c; ++c)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

	glPopMatrix(); 
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glPopAttrib();
	glUseProgram(prevProgram);
}

void RenderFunction(void)
{
	static auto lastTime = std::chrono::high_resolution_clock::now();
	static float displayFPS = 0.0f;

	static float accumTime = 0.0f;
	static int accumFrames = 0;

	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> delta = now - lastTime;
	lastTime = now;

	accumTime += delta.count();
	accumFrames++;

	if (accumTime >= 0.25f) {
		displayFPS = accumFrames / accumTime;
		accumTime = 0.0f;
		accumFrames = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CCW); 
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
		
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();

	MyCamera.Update();

	Shader& MyShader = shaders->MyTerrainShaderNoise;
	MyShader.Bind();

	models->MyTerrain->updateLodMap(MyCamera.getObs());
	shaders->UpdateTerrainNoise(*models->MyTerrain, MyCamera, lights.myLight);
	models->MyTerrain->Draw();

	MyShader.setUniformInt("usingNoise", 0);

	glm::mat4 skyboxMat = glm::translate(glm::mat4(1.0f), MyCamera.getObs()) * glm::scale(glm::mat4(1.0f), glm::vec3(vSkyboxScale, vSkyboxScale, vSkyboxScale));
	MyShader.setUniformMat4("modelMatrix", skyboxMat);
	MyShader.setUniformInt("codCol", 2);
	MyShader.updateMaterial(models->MySkybox->getMaterial());
	models->MySkybox->Draw();

	MyShader.setUniformVec3("viewPos", MyCamera.getObs());
	models->Update(MyShader);


	shaders->MyVegetationShader.Bind();
	shaders->UpdateVegetation(*models->MyTerrain, MyCamera, lights.myLight);
	models->MyTerrain->DrawVegetation(&shaders->MyVegetationShader);

	///*MyShader.setUniformMat4("modelMatrix", matStack.top());
	//for (Model* model : models) {
	//	model->Draw();
	//}*/

	shaders->MyInstancingShader.Bind();
	shaders->MyInstancingShader.setUniformMat4("viewMatrix", MyCamera.getView());
	shaders->MyInstancingShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
	shaders->MyInstancingShader.setUniformVec3("viewPos", MyCamera.getObs());
	shaders->MyInstancingShader.setUniformInt("codCol", 0);
	models->MyCube->Draw();

	//shaders->MyInstancingShader.setUniformInt("codCol", 1);
	//MyCube.DrawEdges();

	ShowMyImGuiWindow();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	RenderHud(displayFPS);
	glutSwapBuffers();
	glFlush();
}


int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Render");

	glewInit();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGLUT_Init();
	ImGui_ImplOpenGL3_Init();

	Initialize();
	glutReshapeFunc(ReshapeFunction);
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(RenderFunction);
	glutKeyboardFunc(ProcessNormalKeys);
	glutSpecialFunc(ProcessSpecialKeys);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutCloseFunc(Cleanup);

	glutMainLoop();

	return 0;
}