#ifndef SHADER_H
#define SHADER_H
#include "client/engine.h"
#include <fstream>
VkShaderModule loadShaderModuleFromFile(VkDevice device,std::string filename);
#endif
