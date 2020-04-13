#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location=0) out vec3 FragColor;
layout(location=1) out vec4 FragPos;
layout(location=2) flat out vec3 FragNorm;

layout(location=0) in vec3 Position;
layout(location=1) in vec3 Color;
layout(location=2) in vec3 Normal;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

void main() {
    FragPos = ubo.proj * ubo.view * ubo.model * vec4(Position, 1.0);
    gl_Position = FragPos;
    FragColor = Color;
    FragNorm = Normal;
}
