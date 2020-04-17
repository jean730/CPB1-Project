#include "client/shaderloader.h"
VkShaderModule loadShaderModuleFromFile(VkDevice device,std::string filename){
        std::ifstream file(filename,std::ios::ate | std::ios::binary);
        unsigned int filesize = file.tellg();
        file.seekg(0);
        std::vector<char> bytecode(filesize);
        file.read(bytecode.data(), filesize);
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = bytecode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());
        VkShaderModule shaderModule;
        if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule)==VK_SUCCESS){
                std::cout << "Loaded shader " << filename << std::endl;
                return shaderModule;
        }
        else{
                std::cerr << "Failed to load shader " << filename << std::endl;
                return NULL;
        }
}
