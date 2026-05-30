#version 330 core

in float shadowAlpha;
in vec3 frag_Position;
in vec3 in_ViewPos;

uniform float uFogStart;
uniform float uFogEnd;
uniform int uUseTerrainTextures;

out vec4 out_Color;

void main(void)
{
    vec3 shadowColor = (uUseTerrainTextures == 1) ? vec3(0.18, 0.17, 0.15) : vec3(0.60, 0.50, 0.75);
    vec3 fogColor = vec3(0.7, 0.7, 0.7);

    float distance = length(in_ViewPos - frag_Position);
    float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);

    float alpha = (uUseTerrainTextures == 1) ? shadowAlpha * 0.55 : shadowAlpha;
    out_Color = vec4(mix(fogColor, shadowColor, fogFactor), alpha);
}
