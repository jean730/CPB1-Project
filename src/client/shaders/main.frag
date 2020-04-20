#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 FragColor;
layout(location = 1) in vec3 FragPos;
layout(location = 2) flat in vec3 FragNorm;

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 lightSourcesPosition[20];
	vec4 lightSourcesColor[20];
	float lightSourcesPower[20];
	float time;
} ubo;

void main() {
	vec3 lightDirection=vec3(0.4,1,0.5);
	vec3 tmpColor = 0.03*FragColor * max(dot(normalize(FragNorm), normalize(lightDirection)), 0);
	for(int i=0;i<20;i++){
		vec3 light = (FragColor * vec3(ubo.lightSourcesColor[i]) * ubo.lightSourcesPower[i]*10);
		light/=max(1,pow((length(vec3(ubo.lightSourcesPosition[i])-FragPos)),2));
		light*=max(dot(normalize(FragNorm), normalize(vec3(ubo.lightSourcesPosition[i])-FragPos)), 0);
		tmpColor+=light;
	}

	outColor = vec4(tmpColor,1);
}
