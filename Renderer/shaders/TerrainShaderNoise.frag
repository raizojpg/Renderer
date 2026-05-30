#version 330 core

in vec3 ex_Color;
in vec3 frag_Position;
in vec3 frag_Normal;
in vec3 in_ViewPos;
in vec3 gouraud_Color;
in vec2 terrain_TexUV;
in float terrain_HeightFactor;
in float terrain_BiomeFactor;
in vec3 sky_Direction;

struct Material
{
    vec3 emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininessValue;
};

struct Light
{
    vec4 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

uniform Material materialShader;
uniform Light lightShader;
uniform int uShadingModel;

out vec3 out_Color;

vec3 result;
uniform int uUseFog;
uniform float uFogStart;
uniform float uFogEnd;
uniform int uUseTerrainTextures;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;
uniform sampler2D skyTexture;
uniform float uGrassRockThreshold;
uniform float uRockSnowThreshold;
uniform float uTextureBlendRange;
uniform int uUseSkyTexture;
vec3 fogColor = vec3(0.7, 0.7, 0.7);
const float PI = 3.14159265359;

vec3 calculateLighting(vec3 positionVertex3D, vec3 normal, bool useBlinnSpecular) {
    vec3 s_normal = normalize(frag_Normal);
    s_normal = normalize(normal);

    vec3 positionSource3D = vec3(lightShader.position);
    float distSV = distance(positionSource3D, positionVertex3D);

    vec3 lightDir;
    vec3 viewDir;
    vec3 reflectDir;
    float diffCoeff;
    float specCoeff;
    float attenuation_factor;

    if (lightShader.position.w == 0.0)
        lightDir = normalize(positionSource3D);
    else
        lightDir = normalize(positionSource3D - positionVertex3D);

    vec3 emission = materialShader.emission;
    vec3 ambient_model = vec3(0.2,0.2,0.2) * materialShader.ambient;
    vec3 ambient_term = lightShader.ambient * materialShader.ambient;

    diffCoeff = max(dot(s_normal, lightDir), 0.0);
    vec3 diffuse_term = diffCoeff * lightShader.diffuse * materialShader.diffuse;

    viewDir = normalize(in_ViewPos - positionVertex3D);

    if (useBlinnSpecular) {
        vec3 halfDir = normalize(lightDir + viewDir);
        specCoeff = pow(max(dot(s_normal, halfDir), 0.0), materialShader.shininessValue);
    }
    else {
        reflectDir = normalize(reflect(-lightDir, s_normal));
        specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), materialShader.shininessValue);
    }

    vec3 specular_term = specCoeff * lightShader.specular * materialShader.specular;

    if (lightShader.position.w != 0.0)
        attenuation_factor = 1.0 / (lightShader.attenuation[0] + lightShader.attenuation[1]*distSV + lightShader.attenuation[2]*distSV*distSV);
    else
        attenuation_factor = 1.0;

    return emission + ambient_model + attenuation_factor*(ambient_term + diffuse_term + specular_term);
}

vec3 calculateNeutralTerrainLighting(vec3 positionVertex3D, vec3 normal) {
    vec3 s_normal = normalize(normal);
    vec3 positionSource3D = vec3(lightShader.position);

    vec3 lightDir;
    if (lightShader.position.w == 0.0)
        lightDir = normalize(positionSource3D);
    else
        lightDir = normalize(positionSource3D - positionVertex3D);

    float diffCoeff = max(dot(s_normal, lightDir), 0.0);
    float ambientStrength = 0.90;
    float diffuseStrength = 0.90;

    return vec3(ambientStrength + diffuseStrength * diffCoeff);
}

void main(void)
{
    if (uUseSkyTexture == 1) {
        vec3 dir = normalize(sky_Direction);
        vec2 skyUV = vec2(atan(dir.y, dir.x) / (2.0 * PI) + 0.5, clamp(dir.z * 0.5 + 0.5, 0.0, 1.0));
        out_Color = texture(skyTexture, skyUV).rgb;
        return;
    }

    vec3 lightingColor;
    if (uShadingModel == 0)
        lightingColor = gouraud_Color;
    else
        lightingColor = calculateLighting(frag_Position, frag_Normal, uShadingModel == 2);

    if (uUseTerrainTextures == 1) {
        vec3 grassColor = texture(grassTexture, terrain_TexUV).rgb;
        vec3 rockColor = texture(rockTexture, terrain_TexUV * 0.75).rgb;
        vec3 snowColor = texture(snowTexture, terrain_TexUV * 1.25).rgb;

        float blendRange = max(uTextureBlendRange, 0.001);
        float rockFromGrass = smoothstep(uGrassRockThreshold - blendRange, uGrassRockThreshold + blendRange, terrain_HeightFactor);
        float snowFromRock = smoothstep(uRockSnowThreshold - blendRange, uRockSnowThreshold + blendRange, terrain_HeightFactor);
        float biomeRock = smoothstep(0.42, 0.72, terrain_BiomeFactor) * (1.0 - snowFromRock);

        vec3 terrainColor = mix(grassColor, rockColor, max(rockFromGrass, biomeRock));
        terrainColor = mix(terrainColor, snowColor, snowFromRock);

        vec3 neutralLight = calculateNeutralTerrainLighting(frag_Position, frag_Normal);
        vec3 litTerrain = terrainColor * clamp(neutralLight, 0.88, 1.60);
        result = clamp(litTerrain, 0.0, 1.0);
    }
    else {
        result = clamp(lightingColor + ex_Color, 0.0, 1.0);
    }

    float distance = length(in_ViewPos - frag_Position);
    float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);

    if (uUseFog == 0){
         out_Color = mix(fogColor, result, 1);
    }
    else{
         out_Color = mix(fogColor, result, fogFactor);
    }
  
}
