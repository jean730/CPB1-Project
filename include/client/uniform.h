#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#ifndef UNIFORM_H
#define UNIFORM_H
struct alignas(16) float_16 {
  float value;
};
struct uniformBufferStruct{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec4 lightSourcesPosition[20];
	glm::vec4 lightSourcesColor[20];
	float_16 lightSourcesPower[20];
	alignas(4)  float time = 0;
};
#endif
