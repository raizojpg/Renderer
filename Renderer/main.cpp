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
#include "sources/Profiler.h"
#include "sources/ShaderManager.h"
#include "sources/InputManager.h"

#include "PerlinNoise.hpp"

ModelManager* models = nullptr;
ShaderManager* shaders = nullptr;
LightManager lights;

Camera OverviewCamera;
Camera MyCamera;
InputManager inputs(MyCamera);

GLint winWidth = 1000, winHeight = 600;

Profiler gProfiler(16384, 12);

void ApplyBackfaceCullingState()
{
	if (vEnableBackfaceCulling) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
}

void UpdateSunLight(float deltaSeconds)
{
	static float sunAngle = atan2f(-0.25f, 0.35f);

	if (vAnimateSun) {
		sunAngle += deltaSeconds * vSunRotationSpeed * 3.14 / 180.0f;
	}

	const float horizontalStrength = 1.0f;
	const float verticalStrength = -vSunElevation;
	lights.myLight.position = glm::vec4(
		cosf(sunAngle) * horizontalStrength,
		sinf(sunAngle) * horizontalStrength,
		verticalStrength,
		0.0f
	);
}

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
	gProfiler.exportCsv("profiler_metrics.csv");
	
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
	ImGui::Text("Misc");
	ImGui::Checkbox("Enable Profiler", &vEnableProfiler);
	if(gProfiler.isEnabled() != vEnableProfiler){
		gProfiler.setEnabled(vEnableProfiler);
	}

	ImGui::Checkbox("Enable Frustum Culling", &vEnableFrustumCulling);
	ImGui::Checkbox("Enable Backface Culling", &vEnableBackfaceCulling);
	ImGui::SliderInt("Speed", &vSpeed, 1, 2000);
	needsInitialization |= ImGui::SliderInt("Seed", &vSeed, 1, 5000);
	freezeSimulation |= ImGui::IsItemActive();

	ImGui::Separator();

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
	ImGui::Checkbox("Enable Vegetation Instancing", &vEnableVegetationInstancing);
	ImGui::Checkbox("Enable Vegetation Shadows", &vEnableVegetationShadows);
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

	ImGui::Separator();

	// Biomes
	ImGui::Text("Biomes");
	ImGui::Checkbox("Use Biomes", &vUseBiomes);
	ImGui::SliderInt("Biome Octaves", &vBiomeOctaves, 1, 10);
	ImGui::SliderFloat("Biome Frequency", &vBiomeFrequency, 0.001f, 1.0f);
	ImGui::SliderFloat("Biome Amplitude", &vBiomeAmplitude, 1.0f, 20.0f);
	ImGui::SliderFloat("Biome Lacunarity", &vBiomeLacunarity, 0.01f, 10.0f);
	ImGui::SliderFloat("Biome Gain", &vBiomeGain, 0.0f, 1.0f);

	ImGui::Separator();

	// Shading
	ImGui::Text("Shading");
	const char* shadingModels[] = { "Gouraud", "Phong", "Blinn-Phong" };
	ImGui::Combo("Lighting Model", &vShadingModel, shadingModels, IM_ARRAYSIZE(shadingModels));
	ImGui::Checkbox("Animate Sun", &vAnimateSun);
	ImGui::SliderFloat("Sun Speed", &vSunRotationSpeed, 0.0f, 45.0f, "%.1f deg/s");
	ImGui::SliderFloat("Sun Elevation", &vSunElevation, 0.1f, 2.0f, "%.2f");

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

	gProfiler.beginFrame();
	Profiler::ScopedBinding profilerBinding(gProfiler);

	accumTime += delta.count();
	accumFrames++;

	if (accumTime >= 0.25f) {
		displayFPS = accumFrames / accumTime;
		accumTime = 0.0f;
		accumFrames = 0;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CCW); 
	ApplyBackfaceCullingState();
	glEnable(GL_DEPTH_TEST);
		
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();

	MyCamera.Update();
	UpdateSunLight(delta.count());
	models->MyTerrain->updateMap(MyCamera);

	{
		PROFILE_GPU(gProfiler, "terrain");
		Shader& MyShader = shaders->MyTerrainShaderNoise;
		MyShader.Bind();

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
		
		shaders->MyInstancingShader.Bind();
		shaders->MyInstancingShader.setUniformMat4("viewMatrix", MyCamera.getView());
		shaders->MyInstancingShader.setUniformMat4("projectionMatrix", MyCamera.getProjection());
		shaders->MyInstancingShader.setUniformVec3("viewPos", MyCamera.getObs());
		shaders->MyInstancingShader.setUniformInt("codCol", 0);
		models->MyCube->Draw();
	}

	if (vEnableVegetationShadows) {
		PROFILE_GPU(gProfiler, "vegetation_shadows");
		shaders->MyVegetationShadowShader.Bind();
		shaders->UpdateVegetationShadow(*models->MyTerrain, MyCamera, lights.myLight);

		GLint previousDepthFunc;
		glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glBlendEquation(GL_MIN);
		glBlendFunc(GL_ONE, GL_ONE);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-8.0f, -16.0f);
		glDisable(GL_CULL_FACE);

		models->MyTerrain->DrawVegetation(&shaders->MyVegetationShadowShader);

		ApplyBackfaceCullingState();
		glDisable(GL_POLYGON_OFFSET_FILL);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);
		glDepthFunc(previousDepthFunc);
		glDepthMask(GL_TRUE);
	}

	{
		PROFILE_GPU(gProfiler, "vegetation");
		shaders->MyVegetationShader.Bind();
		shaders->UpdateVegetation(*models->MyTerrain, MyCamera, lights.myLight);
		models->MyTerrain->DrawVegetation(&shaders->MyVegetationShader);
	}

	if (vEnableOverview) {
		PROFILE_GPU(gProfiler, "world_overview");
		glDisable(GL_DEPTH_TEST);
		glViewport(winWidth - winWidth / 4 - 10, winHeight - winHeight / 4 - 10, winWidth / 4, winHeight / 4);

		OverviewCamera.setObs(glm::vec3(MyCamera.getObs().x, MyCamera.getObs().y, -100000.0f));
		OverviewCamera.setRef(MyCamera.getObs());
		OverviewCamera.setVert(glm::vec3(1.0f, 0.0f, 0.0f));
		OverviewCamera.setView();
		OverviewCamera.setProjection(MyCamera.getProjection());
		
		shaders->MyTerrainShaderNoise.Bind();
		shaders->UpdateTerrainNoise(*models->MyTerrain, OverviewCamera, lights.myLight);
		shaders->MyTerrainShaderNoise.setUniformInt("uUseFog", 0);
		models->MyTerrain->Draw();
		
		glViewport(0, 0, winWidth, winHeight);
		glEnable(GL_DEPTH_TEST);
	}

	ShowMyImGuiWindow();
	{
		PROFILE_GPU(gProfiler, "ui");
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	RenderHud(displayFPS);
	gProfiler.endFrame();

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
	gProfiler.setEnabled(vEnableProfiler);

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
