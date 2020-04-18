#ifndef SHADER_H
#define SHADER_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <vector>
VkShaderModule loadShaderModuleFromFile(VkDevice device,std::string filename);

class Engine;
class ShaderPair{
public:
	ShaderPair(Engine *engineInstance,std::string vertex,std::string fragment);
	VkShaderModule fragmentShader;
	VkShaderModule vertexShader;
	VkPipelineShaderStageCreateInfo shaderCreateInfo[2];
};
#endif
