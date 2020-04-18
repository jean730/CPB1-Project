#ifndef ENGINE_H
#define ENGINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <functional>
#include "client/vertex.h"
#include "client/entity.h"
#include "client/uniform.h"
#include "client/terrain.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define PI 3.14159265
#include <math.h>
#include <chrono>
#include <thread>

class ShaderPair;
class Engine {
public:
	Engine(std::string name,uint32_t width,uint32_t height);
	~Engine();
	void initVulkan();

	GLFWwindow *window;
	void draw();
	void update(int timeElapsed);
	void mainLoop();

	std::vector<Entity> Entities;
	void initTerrain(int size);
	
	//Engine parameters
	int WIDTH;
	int HEIGHT;
	std::string ENGINE_NAME;
	bool ENABLE_WIREFRAME = false;
	bool ENABLE_VALIDATION_LAYERS = false;

	//Engine Variables
	glm::vec3 camPos = glm::vec3(0,0,0);
	glm::vec2 eyeAngles = glm::vec2(0,0);
	bool FORWARD=false;
	bool BACK=false;
	bool RIGHT=false;
	bool LEFT=false;
	bool UP=false;
	bool DOWN=false;
	bool SPRINT=false;

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


	//Pipeline creation
	ShaderPair *shaderPair;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	VkVertexInputBindingDescription vertexInputBindingDescription;
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	VkPipelineMultisampleStateCreateInfo multisampler = {};
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkAttachmentDescription depthAttachment = {};
	VkAttachmentReference depthAttachmentRef = {};
	VkAttachmentDescription colorAttachment = {};
	VkAttachmentReference colorAttachmentRef = {};
	VkSubpassDependency subpassDependency = {};
	VkSubpassDescription subpass = {};
	VkRenderPass renderPass;

	//Framebuffers creation
	std::vector<VkFramebuffer> Framebuffers;
	VkImageView depthImageView;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;

	//Uniform Buffer
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	uniformBufferStruct uniformBufferObject={};

	//Descriptors
	VkDescriptorPool descPool;
	std::vector<VkDescriptorSet> descSets;

	//Command
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	//Semaphores
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	//Fences
	VkFence Fence;

	



	uint32_t imageCount;

};
#endif
