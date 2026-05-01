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
uniform int uShadingModel;

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

out vec3 ex_Color;
out vec3 frag_Position;
out vec3 frag_Normal;
out vec3 in_ViewPos;
out vec3 gouraud_Color;

/* ---------- Noise ---------- */

vec2 hash(vec2 p) {
    p = vec2(
        dot(p, vec2(127.1, 311.7)),
        dot(p, vec2(269.5, 183.3))
    );
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

// Fade curve (Perlin s improved smoothstep)
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

    return mix(
        mix(a, b, u.x),
        mix(c, d, u.x),
        u.y
    );
}

float perlinFBM(
    vec2 p,
    int octaves,
    float frequency,
    float amplitude,
    float lacunarity,
    float gain
) {
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

/* ---------- Noise ---------- */

vec3 calculateLighting(vec3 positionVertex3D, vec3 normal, bool useBlinnSpecular) {
    vec3 s_normal = normalize(normal);
    vec3 positionSource3D = vec3(lightShader.position);
    float distSV = distance(positionSource3D, positionVertex3D);

    vec3 lightDir;
    if (lightShader.position.w == 0.0)
        lightDir = normalize(positionSource3D);
    else
        lightDir = normalize(positionSource3D - positionVertex3D);

    vec3 emission = materialShader.emission;
    vec3 ambient_model = vec3(0.2, 0.2, 0.2) * materialShader.ambient;
    vec3 ambient_term = lightShader.ambient * materialShader.ambient;

    float diffCoeff = max(dot(s_normal, lightDir), 0.0);
    vec3 diffuse_term = diffCoeff * lightShader.diffuse * materialShader.diffuse;

    vec3 viewDir = normalize(viewPos - positionVertex3D);
    float specCoeff;
    if (useBlinnSpecular) {
        vec3 halfDir = normalize(lightDir + viewDir);
        specCoeff = pow(max(dot(s_normal, halfDir), 0.0), materialShader.shininessValue);
    }
    else {
        vec3 reflectDir = normalize(reflect(-lightDir, s_normal));
        specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), materialShader.shininessValue);
    }

    vec3 specular_term = specCoeff * lightShader.specular * materialShader.specular;

    float attenuation_factor;
    if (lightShader.position.w != 0.0)
        attenuation_factor = 1.0 / (lightShader.attenuation[0] + lightShader.attenuation[1] * distSV + lightShader.attenuation[2] * distSV * distSV);
    else
        attenuation_factor = 1.0;

    return emission + ambient_model + attenuation_factor * (ambient_term + diffuse_term + specular_term);
}



void main(void)
{

     vec4 position;
     vec3 frag;

    if (usingNoise == 1) {
        vec3 instWorldPos = (modelMatrix * in_instancedModelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vec2 uv = instWorldPos.xy * uNoiseScale;

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
        
        if(usingBiomes == 1){
			noise = mix(noise, bnoise, bnoise);
		}

        // Threshold check
        if (noise < uLowerTreeTreshold || noise > uUpperTreeTreshold) {
            gl_Position = vec4(2.0, 2.0, 2.0, 1.0); // move it outside the clip space
            return;
        }

        vec4 displacedPosition = in_instancedModelMatrix * in_Position;
        displacedPosition.z -= noise * uMaxHeight;
        //displacedPosition.z += 0;
        
        position = projectionMatrix * viewMatrix * modelMatrix * displacedPosition;
        frag = vec3( modelMatrix * displacedPosition);
  

    } else {
        position = projectionMatrix * viewMatrix * modelMatrix * in_instancedModelMatrix * in_Position;
        frag = vec3( modelMatrix * in_instancedModelMatrix * in_Position);
    }

    gl_Position = position;
    
    frag_Position = frag; 
    mat3 normalMat = transpose(inverse(mat3(modelMatrix * in_instancedModelMatrix)));
    frag_Normal = normalize(normalMat * in_Normal);
    
    in_ViewPos = viewPos;
    ex_Color = in_Color;
    gouraud_Color = calculateLighting(frag_Position, frag_Normal, false);
  
}
