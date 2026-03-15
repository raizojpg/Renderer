#include "Variables.h"

int vSpeed = 200;
int vSeed = 1338;
bool vEnableFrustumCulling = 1;
bool vEnableOverview = 0;

int vSkyboxScale = 200;
bool vUseFog = 1;
int vFogStart = 50000;
int vFogEnd = 100000;

int vTerrainSize = 2048;
int vTerrainStep = 100;
int vMaxHeight = 2550;

int vPatchSize = 128;
int vMaxLod = 10;
int vLodDistribution = 1;

int vTreeLodDistribution = 2;
int vTreeInstanceCount = 20;
float vLowerTreeTreshold = 0.35;
float vUpperTreeTreshold = 0.5;

float vNoiseScale = 0.00010f;
int vOctaves = 8;
float vFrequency = 0.5;
float vAmplitude = 2.5;
float vLacunarity = 2.0;
float vGain = 0.5;

bool vUseBiomes = 1;
int vBiomeOctaves = 4;
float vBiomeFrequency = 0.05;
float vBiomeAmplitude = 10;
float vBiomeLacunarity = 4.0;
float vBiomeGain = 0.25;