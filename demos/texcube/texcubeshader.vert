#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform matrixBlock {
    mat4 mvp;
    mat4 normalMat;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.mvp * vec4(pos, 1.0);
    fragTexCoord = texCoord;
    fragNormal = normalize((ubo.normalMat * vec4(normal, 0.0)).xyz);
}