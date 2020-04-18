#ifndef SHADER_H
#define SHADER_H
#include "client/engine.h"
#include <fstream>
VkShaderModule loadShaderModuleFromFile(VkDevice device,std::string filename);
class ShaderPair{
public:
	ShaderPair(Engine *engineInstance,std::string vertex,std::string fragment);
	VkShaderModule fragmentShader;
	VkShaderModule vertexShader;
	VkPipelineShaderStageCreateInfo shaderCreateInfo[2];
};
#endif
