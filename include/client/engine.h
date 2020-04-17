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
	VkExtent2D extent;
	std::string engineName;

	VkInstance vkinstance;
	VkApplicationInfo applicationInfo = {};
	
	//Devices
	VkPhysicalDevice physicalDevice = NULL;
	VkDevice logicalDevice = NULL;

	//Queue
	uint32_t graphicsFamily;
	VkQueue graphicsQueue;

	VkSurfaceKHR surface;
	VkSurfaceFormatKHR format;

	VkSwapchainKHR swapchain;

	uint32_t imageCount;

};
#endif
