#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
class Engine {
public:
	Engine(std::string name,int width,int height);
	void initVulkan();

	GLFWwindow *window;

	int WIDTH;
	int HEIGHT;
	std::string engineName;

	VkInstance vkinstance;
	VkApplicationInfo applicationInfo = {};
	
	//Devices
	VkPhysicalDevice physicalDevice = NULL;
	VkDevice logicalDevice = NULL;

	//Queue families IDs
	uint32_t graphicsFamily;

	//Queues
	VkQueue graphicsQueue;

};
#endif
