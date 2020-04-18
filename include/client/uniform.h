#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#ifndef UNIFORM_H
#define UNIFORM_H
struct uniformBufferStruct{
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	alignas(4) float time = 0;
};
#endif
