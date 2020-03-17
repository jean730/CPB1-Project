#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "client/vertex.h"
#ifndef ENTITY_H
#define ENTITY_H
class Entity{
public:
	Entity();
	void destroyBuffers(VkDevice &device);
	void createBuffers(VkDevice &device,VkPhysicalDevice &pDevice);
	glm::vec3 Position = glm::vec3(0);
	glm::vec3 Angle = glm::vec3(0);
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	bool hasIndexBuffer=false;



};
#endif
