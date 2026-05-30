#pragma once

extern int vSpeed;
extern int vSeed;
extern bool vEnableProfiler;
extern bool vEnableFrustumCulling;
extern bool vEnableBackfaceCulling;
extern bool vEnableOverview;

extern int vSkyboxScale;
extern bool vUseFog;
extern int vFogStart;
extern int vFogEnd;

extern int vTerrainSize;
extern int vTerrainStep;
extern int vMaxHeight;

extern int vPatchSize;
extern int vMaxLod;
extern int vLodDistribution;

extern bool vEnableVegetationInstancing;
extern bool vEnableVegetationShadows;
extern int vTreeLodDistribution;
extern int vTreeInstanceCount;
extern float vLowerTreeTreshold;
extern float vUpperTreeTreshold;

extern float vNoiseScale;
extern int vOctaves;
extern float vFrequency;
extern float vAmplitude;
extern float vLacunarity;
extern float vGain;

extern bool vUseBiomes;
extern int vBiomeOctaves;
extern float vBiomeFrequency;
extern float vBiomeAmplitude;
extern float vBiomeLacunarity;
extern float vBiomeGain;

extern bool vUseTerrainTextures;
extern float vTerrainTextureScale;
extern float vTerrainGrassRockThreshold;
extern float vTerrainRockSnowThreshold;
extern float vTerrainTextureBlendRange;
extern float vTreeTextureScale;

extern int vShadingModel;
extern bool vAnimateSun;
extern float vSunRotationSpeed;
extern float vSunElevation;
