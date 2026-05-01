#version 330 core

layout(location=0) in vec4 in_Position;
layout(location=1) in vec3 in_Color;
layout(location=2) in vec3 in_Normal;
layout(location=3) in vec2 in_UVs;
layout(location=4) in mat4 in_instancedModelMatrix;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec3 viewPos;

uniform int usingNoise;
uniform int usingBiomes;
uniform float uMaxHeight;
uniform float uNoiseScale;
uniform int uOctaves;
uniform float uFrequency;
uniform float uAmplitude;
uniform float uLacunarity;
uniform float uGain;
uniform int uBiomeOctaves;
uniform float uBiomeFrequency;
uniform float uBiomeAmplitude;
uniform float uBiomeLacunarity;
uniform float uBiomeGain;
uniform float uLowerTreeTreshold;
uniform float uUpperTreeTreshold;
uniform float uShadowBias;
uniform float uShadowThickness;

struct Light
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

uniform Light lightShader;

out float shadowAlpha;
out vec3 frag_Position;
out vec3 in_ViewPos;

vec2 hash(vec2 p) {
    p = vec2(
        dot(p, vec2(127.1, 311.7)),
        dot(p, vec2(269.5, 183.3))
    );
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

vec2 fade(vec2 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float perlinNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = fade(f);

    float a = dot(hash(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0));
    float b = dot(hash(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0));
    float c = dot(hash(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0));
    float d = dot(hash(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float perlinFBM(vec2 p, int octaves, float frequency, float amplitude, float lacunarity, float gain) {
    float value = 0.0;
    float freq = frequency;
    float amp = amplitude;

    for (int i = 0; i < octaves; i++) {
        value += amp * perlinNoise(p * freq);
        freq *= lacunarity;
        amp *= gain;
    }

    return value;
}

float terrainLocalHeight(vec2 worldXY) {
    if (usingNoise != 1) {
        return 0.0;
    }

    vec2 uv = worldXY * uNoiseScale;

    float noise = perlinFBM(
        uv,
        uOctaves,
        uFrequency,
        uAmplitude,
        uLacunarity,
        uGain
    );

    float bnoise = perlinFBM(
        uv,
        uBiomeOctaves,
        uBiomeFrequency,
        uBiomeAmplitude,
        uBiomeLacunarity,
        uBiomeGain
    );

    bnoise = bnoise * 0.75 + 0.0;
    noise = noise * 0.5 + 0.5;

    if (usingBiomes == 1) {
        noise = mix(noise, bnoise, bnoise);
    }

    return -noise * uMaxHeight;
}

float terrainWorldHeight(vec2 worldXY) {
    return modelMatrix[3].z + terrainLocalHeight(worldXY);
}

vec3 shadowRayForVertex(vec3 worldPos) {
    vec3 rayDir;
    if (lightShader.position.w == 0.0) {
        rayDir = normalize(lightShader.position.xyz);
    }
    else {
        rayDir = normalize(lightShader.position.xyz - worldPos);
    }

    if (abs(rayDir.z) < 0.05) {
        rayDir.z = rayDir.z < 0.0 ? -0.05 : 0.05;
        rayDir = normalize(rayDir);
    }

    float terrainZ = terrainWorldHeight(worldPos.xy);
    if ((terrainZ - worldPos.z) / rayDir.z < 0.0) {
        rayDir = -rayDir;
    }

    return rayDir;
}

vec3 projectOnTerrain(vec3 worldPos) {
    vec3 rayDir = shadowRayForVertex(worldPos);
    float t = (terrainWorldHeight(worldPos.xy) - worldPos.z) / rayDir.z;
    t = max(t, 0.0);

    for (int i = 0; i < 1; i++) {
        vec2 sampleXY = worldPos.xy + rayDir.xy * t;
        float targetZ = terrainWorldHeight(sampleXY) + uShadowBias;
        float nextT = (targetZ - worldPos.z) / rayDir.z;
        t = mix(t, nextT, 0.75);
    }

    t = max(t, 0.0);
    vec2 shadowXY = worldPos.xy + rayDir.xy * t;
    float shadowZ = terrainWorldHeight(shadowXY) + uShadowBias;
    return vec3(shadowXY, shadowZ);
}

void main(void)
{
    vec3 instWorldPos = (modelMatrix * in_instancedModelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    float placementHeight = terrainLocalHeight(instWorldPos.xy);
    float placementNoise = -placementHeight / uMaxHeight;

    if (placementNoise < uLowerTreeTreshold || placementNoise > uUpperTreeTreshold) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        shadowAlpha = 0.0;
        frag_Position = vec3(0.0);
        in_ViewPos = viewPos;
        return;
    }

    vec4 displacedPosition = in_instancedModelMatrix * in_Position;
    displacedPosition.z += placementHeight;

    mat3 normalMat = mat3(modelMatrix * in_instancedModelMatrix);
    vec3 worldNormal = normalize(normalMat * in_Normal);
    vec3 worldPos = vec3(modelMatrix * displacedPosition) + worldNormal * uShadowThickness;
    vec3 terrainShadowPos = projectOnTerrain(worldPos);

    float casterHeight = max(worldPos.z - terrainWorldHeight(worldPos.xy), 0.0);
    shadowAlpha = clamp(casterHeight / 1200.0, 0.25, 1.0);
    frag_Position = terrainShadowPos;
    in_ViewPos = viewPos;

    gl_Position = projectionMatrix * viewMatrix * vec4(terrainShadowPos, 1.0);
}
