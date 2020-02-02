#include "client/entity.h"
#include <iostream>
Entity::Entity(){

}
void Entity::destroyVertexBuffer(VkDevice &device){
	vkDestroyBuffer(device, this->vertexBuffer, nullptr);
	vkFreeMemory(device, this->vertexBufferMemory, nullptr);
}
void Entity::createVertexBuffer(VkDevice &device,VkPhysicalDevice &pDevice){
	VkBufferCreateInfo vertexBufferCreateInfo = {};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = sizeof(Vertex) * this->Vertices.size();
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(device, &vertexBufferCreateInfo, nullptr, &this->vertexBuffer);
	VkMemoryRequirements memoryRequirements;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(pDevice, &memoryProperties);
	vkGetBufferMemoryRequirements(device, this->vertexBuffer, &memoryRequirements);
	int memoryType=0;
	for(int i=0;i<memoryProperties.memoryTypeCount;i++){
		if(memoryRequirements.memoryTypeBits &(1<<i) && memoryProperties.memoryTypes[i].propertyFlags &
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)==
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT){
			memoryType=i;
//			std::cout << "Memory Type: " << i << std::endl;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryType;
	if(vkAllocateMemory(device,&memoryAllocateInfo,nullptr,&this->vertexBufferMemory) == VK_SUCCESS){
//		std::cout << "Allocated Memory for entity: " << memoryRequirements.size << std::endl;
		vkBindBufferMemory(device, this->vertexBuffer, this->vertexBufferMemory, 0);
	}
	void* vertexData;
	vkMapMemory(device, this->vertexBufferMemory, 0, vertexBufferCreateInfo.size, 0, &vertexData);
	memcpy(vertexData, this->Vertices.data(), (size_t) vertexBufferCreateInfo.size);
	vkUnmapMemory(device, vertexBufferMemory);


}
