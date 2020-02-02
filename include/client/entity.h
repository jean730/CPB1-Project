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
	void destroyVertexBuffer(VkDevice &device);
	void createVertexBuffer(VkDevice &device,VkPhysicalDevice &pDevice);
	glm::vec3 Position;
	glm::vec3 Angle;
	std::vector<Vertex> Vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;



};
#endif
