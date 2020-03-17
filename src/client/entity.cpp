#include "client/entity.h"
#include <iostream>
Entity::Entity(){

}
void Entity::destroyBuffers(VkDevice &device){
	if(this->hasIndexBuffer){
		vkDestroyBuffer(device, this->indexBuffer, nullptr);
		vkFreeMemory(device, this->indexBufferMemory, nullptr);
	}
	vkDestroyBuffer(device, this->vertexBuffer, nullptr);
	vkFreeMemory(device, this->vertexBufferMemory, nullptr);
}
void Entity::createBuffers(VkDevice &device,VkPhysicalDevice &pDevice){
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
		if(memoryRequirements.memoryTypeBits &(1<<i) && (memoryProperties.memoryTypes[i].propertyFlags &
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){
			memoryType=i;
//			std::cout << "Memory Type: " << i << std::endl;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryType;
	if(vkAllocateMemory(device,&memoryAllocateInfo,nullptr,&this->vertexBufferMemory) == VK_SUCCESS){
		std::cout << "Allocated Vertex Memory for entity: " << memoryRequirements.size << std::endl;
		vkBindBufferMemory(device, this->vertexBuffer, this->vertexBufferMemory, 0);
	}
	void* vertexData;
	vkMapMemory(device, this->vertexBufferMemory, 0, vertexBufferCreateInfo.size, 0, &vertexData);
	memcpy(vertexData, this->Vertices.data(), (size_t) vertexBufferCreateInfo.size);
	vkUnmapMemory(device, vertexBufferMemory);


	if(this->hasIndexBuffer){
		VkBufferCreateInfo indexBufferCreateInfo = {};
		indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferCreateInfo.size = sizeof(uint32_t) * this->Indices.size();
		indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(device, &indexBufferCreateInfo, nullptr, &this->indexBuffer);
		VkMemoryRequirements indexMemoryRequirements;
		VkPhysicalDeviceMemoryProperties indexMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(pDevice, &indexMemoryProperties);
		vkGetBufferMemoryRequirements(device, this->indexBuffer, &indexMemoryRequirements);
		int indexMemoryType=0;
		for(int i=0;i<indexMemoryProperties.memoryTypeCount;i++){
			if(indexMemoryRequirements.memoryTypeBits &(1<<i) && (indexMemoryProperties.memoryTypes[i].propertyFlags &
					(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==
					(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){
				indexMemoryType=i;
	//			std::cout << "Memory Type: " << i << std::endl;
			}
		}

		VkMemoryAllocateInfo indexMemoryAllocateInfo = {};
		indexMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		indexMemoryAllocateInfo.allocationSize = indexMemoryRequirements.size;
		indexMemoryAllocateInfo.memoryTypeIndex = indexMemoryType;
		if(vkAllocateMemory(device,&indexMemoryAllocateInfo,nullptr,&this->indexBufferMemory) == VK_SUCCESS){
			std::cout << "Allocated Index Memory for entity: " << indexMemoryRequirements.size << std::endl;
			vkBindBufferMemory(device, this->indexBuffer, this->indexBufferMemory, 0);
		}
		void* indexData;
		vkMapMemory(device, this->indexBufferMemory, 0, indexBufferCreateInfo.size, 0, &indexData);
		memcpy(indexData, this->Indices.data(), (size_t) indexBufferCreateInfo.size);
		vkUnmapMemory(device, indexBufferMemory);
	}
}
