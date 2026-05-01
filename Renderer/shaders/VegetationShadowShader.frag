#version 330 core

in float shadowAlpha;
in vec3 frag_Position;
in vec3 in_ViewPos;

uniform float uFogStart;
uniform float uFogEnd;

out vec4 out_Color;

void main(void)
{
    vec3 shadowColor = vec3(0.60, 0.50, 0.75);
    vec3 fogColor = vec3(0.7, 0.7, 0.7);

    float distance = length(in_ViewPos - frag_Position);
    float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);

    out_Color = vec4(mix(fogColor, shadowColor, fogFactor), 1.0);
}
