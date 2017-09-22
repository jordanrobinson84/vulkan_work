#version 450
#extension GL_ARB_separate_shader_objects : enable

struct uniformLayoutStruct{
    mat4 MVP[8];
    mat4 Normal[8];
};

layout(set = 0, binding = 1) uniform matrixBlock {
    uniformLayoutStruct uniformStruct;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    //float x = ((gl_InstanceIndex & 1) == 0) ? -0.375f : 0.375f;
    //float y = ((gl_InstanceIndex & 2) == 0) ? -0.375f : 0.375f;
    //float z = ((gl_InstanceIndex & 4) == 0) ? -0.375f : 0.375f;
    //vec3 displacement = vec3(x, y, z);

    gl_Position = ubo.uniformStruct.MVP[gl_InstanceIndex] * vec4(pos, 1.0);
    fragTexCoord = texCoord;
    fragNormal = normalize((ubo.uniformStruct.Normal[gl_InstanceIndex] * vec4(normal, 0.0)).xyz);
}