#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightPos = vec3(0.0, 0.0, -10.0);
    vec3 L = normalize(lightPos - vec3(0.0, 0.0, 0.0));
    vec3 nDotL = max(vec3(0.0, 0.0, 0.0), dot(fragNormal, L));
    outColor = vec4(fragColor * nDotL, 1.0);
    //outColor = vec4(nDotL, 1.0);
    //outColor = vec4(fragNormal, 1.0);
}