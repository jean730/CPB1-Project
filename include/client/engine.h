#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "client/entity.h"
#include "client/terrain.h"
class Engine {
public:
	Engine(std::string name,uint32_t width,uint32_t height);
	void initVulkan();

	GLFWwindow *window;

	std::vector<Entity> Entities;
	void initTerrain(int size);
	
	//Engine parameters
	int WIDTH;
	int HEIGHT;
	std::string ENGINE_NAME = "default";
	bool PREFER_TRIPLE_BUFFERING = true;

	VkExtent2D extent;

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

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> imageViews;

	uint32_t imageCount;

};
#endif
