#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 FragColor;
layout(location = 1) in vec4 FragPos;
layout(location = 2) flat in vec3 FragNorm;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

void main() {
    vec3 lightDirection=vec3(0.4,1,0.5);
    vec3 tmpColor = FragColor * max(dot(normalize(FragNorm), normalize(lightDirection)), 0);
    outColor = vec4(tmpColor,1);
}
