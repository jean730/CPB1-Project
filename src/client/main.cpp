#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "client/vertex.h"
#include "client/entity.h"
#include "client/uniform.h"
#include "client/terrain.h"
#include <math.h>
#include <chrono>
unsigned short int WIDTH=1280;
unsigned short int HEIGHT=960;
bool WIREFRAME_MODE=false;
void err(int errnum, const char* err){
	std::cerr << errnum << " -- " << err << std::endl;
}
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
int main(){
	glfwSetErrorCallback(err);
	if(!glfwInit()){
		std::cerr << "glfw did not initialize !" << std::endl;
		glfwTerminate();
		return -1;
	}
	if(glfwVulkanSupported() == GLFW_FALSE){
		std::cerr << "Vulkan Not Supported !" << std::endl;
		glfwTerminate();
		return -1;
	}

	GLFWwindow *window;

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "test", NULL,NULL);

	VkInstance vkinstance;
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "test";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
	applicationInfo.pEngineName = "test";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	unsigned int count=0;
	const char** exts = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensionVector;
	for(unsigned int i=0;i<count;i++){
		extensionVector.push_back(exts[i]);
	}
	extensionVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	createInfo.enabledExtensionCount = count;
	createInfo.ppEnabledExtensionNames = extensionVector.data();
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	std::vector<const char*> selectedLayers;
//	selectedLayers.push_back("VK_LAYER_KHRONOS_validation");
	selectedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	createInfo.enabledLayerCount = selectedLayers.size();
	createInfo.ppEnabledLayerNames=selectedLayers.data();
	std::cout << "Available Layers:" << std::endl;
	for(unsigned int i=0;i<availableLayers.size();i++){
		std::cout << "\t- " << availableLayers[i].layerName << std::endl;
	}
	std::cout << "Selected Layers:" << std::endl;
	for(unsigned int i=0;i<selectedLayers.size();i++){
		std::cout << "\t- " << selectedLayers[i] << std::endl;
	}
	std::cout << "Loaded extensions:" << std::endl;
	for(unsigned int i=0;i<extensionVector.size();i++){
		std::cout << "\t- " << extensionVector[i] << std::endl;
	}
	
	vkCreateInstance(&createInfo, nullptr,&vkinstance);
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkinstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkinstance, &deviceCount, devices.data());
	std::cout << "Available devices:" << std::endl;
	std::string deviceName;
	for (long unsigned int i=0;i<devices.size();i++) {
		VkPhysicalDeviceProperties vkp;
		vkGetPhysicalDeviceProperties(devices[i],&vkp);
		std::cout << "\t- " << vkp.deviceName << std::endl;
		if(physicalDevice==NULL or
				vkp.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			physicalDevice=devices[i];
			deviceName=vkp.deviceName;
		}
	}
	std::cout << "Selected device: " << std::endl
	<< "\t- " << deviceName << std::endl;

	uint32_t graphicsFamilyID;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
	for (long unsigned int i=0;i<queueFamilies.size();i++) {
		if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			graphicsFamilyID = i;
			std::cout << "Graphics queue family ID:" << std::endl
			<< "\t- " << i << std::endl;
		}
	}
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphicsFamilyID;
	queueCreateInfo.queueCount = 1;
	float priority = 1.0f;
	queueCreateInfo.pQueuePriorities = &priority;
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.fillModeNonSolid=true;
	VkDeviceCreateInfo devCreateInfo = {};
	devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	devCreateInfo.queueCreateInfoCount = 1;
	devCreateInfo.pEnabledFeatures = &deviceFeatures;

	std::vector<const char*> logicalExtensionVector;
	logicalExtensionVector.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	devCreateInfo.enabledExtensionCount = logicalExtensionVector.size();
	devCreateInfo.ppEnabledExtensionNames = logicalExtensionVector.data();
	VkDevice device = nullptr;
	if (vkCreateDevice(physicalDevice, &devCreateInfo, nullptr, &device) != VK_SUCCESS) {
		std::cerr << "Failed to create logical device." << std::endl ;
		glfwTerminate();
	}
	VkQueue graphicsQueue;
	vkGetDeviceQueue(device, graphicsFamilyID, 0, &graphicsQueue);
	VkSurfaceKHR surface;
	if ( glfwCreateWindowSurface(vkinstance, window, nullptr, &surface) != VK_SUCCESS){
		std::cerr << "Cannot create window surface " << std::endl ;
		glfwTerminate();
	}
	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamilyID, surface, &presentSupport);
	if(presentSupport){
		std::cout << "Queue supports presentation." << std::endl;
	}
	else{
		std::cerr << "This program is not designed to support presentation on a different queue than graphics." << std::endl;
		glfwTerminate();
	}
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
	}
	
	VkSurfaceFormatKHR format = formats[0];
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for(unsigned int i=0;i<presentModeCount;i++){
		if(presentModes[i]==VK_PRESENT_MODE_MAILBOX_KHR){
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			std::cout << "Surface supports triple buffering." << std::endl;
		}
	}
	VkExtent2D extent = {WIDTH,HEIGHT};
	uint32_t imageCount=capabilities.minImageCount+1;
	if(capabilities.minImageCount == capabilities.maxImageCount){
		imageCount-=1;
	}
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = format.format;
	swapchainCreateInfo.imageColorSpace = format.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain;
	if(vkCreateSwapchainKHR(device,&swapchainCreateInfo,nullptr,&swapchain) != VK_SUCCESS){
		std::cerr << "Could not create swap chain." << std::endl;
		glfwTerminate();
	}
	else{
		std::cout << "Swapchain successfully created." << std::endl;
	}
	std::vector<VkImage> swapChainImages;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages.data());
	VkFormat swapChainImageFormat = format.format;
	std::vector<VkImageView> ImageViews;
	ImageViews.resize(imageCount);
	for(unsigned int i=0;i<imageCount;i++){
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapChainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = swapChainImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		if(vkCreateImageView(device,&imageViewCreateInfo,nullptr,&ImageViews[i])!=VK_SUCCESS){
			std::cerr << "Could not create Image View." << i << std::endl;
			glfwTerminate();
		}

	}

	auto fragmentShaderModule = loadShaderModuleFromFile(device,"frag.sprv");
	auto vertexShaderModule = loadShaderModuleFromFile(device,"vert.sprv");

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	vertexShaderStageInfo.module = vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaders[] = {vertexShaderStageInfo, fragmentShaderStageInfo};

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions(4);
	for(unsigned int i=0;i<4;i++){
		vertexInputAttributeDescriptions[i].binding=0;
		vertexInputAttributeDescriptions[i].location=i;
		vertexInputAttributeDescriptions[i].format=VK_FORMAT_R32G32B32_SFLOAT;
	}
	vertexInputAttributeDescriptions[0].offset=offsetof(Vertex,Vertex::Position);
	vertexInputAttributeDescriptions[1].offset=offsetof(Vertex,Vertex::Color);
	vertexInputAttributeDescriptions[2].offset=offsetof(Vertex,Vertex::Normal);
	vertexInputAttributeDescriptions[3].offset=offsetof(Vertex,Vertex::UV);
	vertexInputAttributeDescriptions[3].format=VK_FORMAT_R32G32_SFLOAT;

	

	VkVertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.binding=0;
	vertexInputBindingDescription.stride=sizeof(Vertex);
	vertexInputBindingDescription.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = 4;
	vertexInputInfo.pVertexBindingDescriptions=&vertexInputBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions=vertexInputAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportStateInfo = {};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;

	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	if(!WIREFRAME_MODE){
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
	else{
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	}
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	
	VkPipelineMultisampleStateCreateInfo multisampler = {};
	multisampler.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampler.sampleShadingEnable = VK_FALSE;
	multisampler.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkDescriptorSetLayoutBinding uniformBufferLayoutBinding={};
	uniformBufferLayoutBinding.binding=0;
	uniformBufferLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferLayoutBinding.descriptorCount=1;
	uniformBufferLayoutBinding.stageFlags=VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		1,
		&uniformBufferLayoutBinding
	};
	vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayout);

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional


	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
	    std::cerr << "Could not create pipeline layout." << std::endl;
	    glfwTerminate();
	}
	else{
	    std::cout << "Pipeline layout successfully created." << std::endl;
	}
	


	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // We do not need the stencil buffer for now
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		std::cerr << "Could not create renderpass." << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Renderpass successfully created." << std::endl;
	}


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount=2;
	pipelineCreateInfo.pStages=shaders;
	pipelineCreateInfo.pVertexInputState=&vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState=&inputAssembly;
	pipelineCreateInfo.pViewportState=&viewportStateInfo;
	pipelineCreateInfo.pRasterizationState=&rasterizer;
	pipelineCreateInfo.pMultisampleState=&multisampler;
	pipelineCreateInfo.pColorBlendState=&colorBlending;
	pipelineCreateInfo.layout=pipelineLayout;
	pipelineCreateInfo.renderPass=renderPass;
	pipelineCreateInfo.subpass=0;

	VkPipeline graphicsPipeline; // FINALLY the graphics pipeline
	if (vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		std::cerr << "Could not create the graphics pipeline." << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Graphics Pipeline successfully created." << std::endl;
	}
	
	std::vector<VkFramebuffer> Framebuffers;
	Framebuffers.resize(imageCount);
	for(unsigned int i=0;i<imageCount;i++){

		VkImageView attachments[] = {
			ImageViews[i]
	   	};
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = extent.width;
		framebufferCreateInfo.height = extent.height;
		framebufferCreateInfo.layers = 1;
		
		if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &Framebuffers[i]) != VK_SUCCESS) {
			std::cerr << "Could not create framebuffer "<< i << std::endl;
			glfwTerminate();
		}
		else{
		    std::cout << "Framebuffer " << i << " successfully created." << std::endl;
		}
	}

	std::vector<Entity> Entities;
	Entities.push_back(Entity());
	Entities[0].Position = glm::vec3(0,5,0);
	Entities[0].Angle = glm::vec3(0,0,0);

	//RIGHT
	Entities[0].Vertices.push_back({{0.5,-0.5,-0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,-0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,-0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,-0.5},{0,0,1},{0,0,0},{0,0}});

	//LEFT
	Entities[0].Vertices.push_back({{0.5,0.5,0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,0.5},{0,0,1},{0,0,0},{0,0}});

	//DOWN
	Entities[0].Vertices.push_back({{0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,-0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,-0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,-0.5},{0,0,1},{0,0,0},{0,0}});

	//UP
	Entities[0].Vertices.push_back({{0.5,0.5,-0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,-0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,-0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,0.5},{0,1,0},{0,0,0},{0,0}});
	
	//FRONT
	Entities[0].Vertices.push_back({{0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{0.5,-0.5,-0.5},{0,0,1},{0,0,0},{0,0}});

	//BACK
	Entities[0].Vertices.push_back({{-0.5,0.5,0.5},{0,0,1},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,0.5,-0.5},{0,1,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,0.5},{1,0,0},{0,0,0},{0,0}});
	Entities[0].Vertices.push_back({{-0.5,-0.5,-0.5},{0,0,1},{0,0,0},{0,0}});
	
	Entities.push_back(createTerrain(3,1,0,0,8.0f,0.02,2));
	Entities.push_back(createTerrain(3,1,-1,0,8.0f,0.02,2));
	Entities.push_back(createTerrain(3,1,0,-1,8.0f,0.02,2));
	Entities.push_back(createTerrain(3,1,-1,-1,8.0f,0.02,2));
	for (Entity &entity: Entities){
		entity.createBuffers(std::ref(device),std::ref(physicalDevice));
	}
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	
	VkBufferCreateInfo uniformBufferCreateInfo = {};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.size = sizeof(uniformBufferStruct);
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(device, &uniformBufferCreateInfo, nullptr, &uniformBuffer);
	VkMemoryRequirements memoryRequirements;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	vkGetBufferMemoryRequirements(device, uniformBuffer, &memoryRequirements);
	int memoryType=0;
	for(unsigned int i=0;i<memoryProperties.memoryTypeCount;i++){
		if(memoryRequirements.memoryTypeBits &(1<<i) && (memoryProperties.memoryTypes[i].propertyFlags &
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==
				(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){
			memoryType=i;
		}
	}

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryType;
	if(vkAllocateMemory(device,&memoryAllocateInfo,nullptr,&uniformBufferMemory) == VK_SUCCESS){
		std::cout << "Allocated Memory for uniform buffer: " << memoryRequirements.size << std::endl;
		vkBindBufferMemory(device, uniformBuffer, uniformBufferMemory, 0);
	}


	VkDescriptorPoolSize descPoolSize = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		imageCount
	};

	VkDescriptorPoolCreateInfo descPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		imageCount,
		1,
		&descPoolSize
	};

	VkDescriptorPool descPool;

	if(vkCreateDescriptorPool(device,&descPoolCreateInfo,nullptr,&descPool)  != VK_SUCCESS) {
                std::cerr << "Could not create Descriptor Pool " << std::endl;
                glfwTerminate();
        }
        else{
		std::cout << "Descriptor Pool successfully created." << std::endl;
        }

	std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);

	VkDescriptorSetAllocateInfo descAllocInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descPool,
		imageCount,
		layouts.data()
	};

	std::vector<VkDescriptorSet> descSets(imageCount);
	if(vkAllocateDescriptorSets(device,&descAllocInfo,descSets.data())  != VK_SUCCESS) {
                std::cerr << "Could not create Descriptor Sets " << std::endl;
                glfwTerminate();
        }
        else{
		std::cout << "Descriptor Sets successfully created." << std::endl;
        }
	





	VkCommandPool commandPool;
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex=graphicsFamilyID;
	poolCreateInfo.flags=VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		std::cerr << "Could not create Command Pool " << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Command Pool successfully created." << std::endl;
	}
	std::vector<VkCommandBuffer> commandBuffers;
	commandBuffers.resize(imageCount);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkSemaphore imageAvailableSemaphore;
	vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
	VkSemaphore renderFinishedSemaphore;
	vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);

	uniformBufferStruct uniformBufferObject={};
	uniformBufferObject.time=0;	

	for(unsigned int i=0;i<imageCount;i++){
		VkDescriptorBufferInfo descBufferInfo = {
			uniformBuffer,
			0,
			VK_WHOLE_SIZE
		};
		
		VkWriteDescriptorSet writeDescriptor = {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			NULL,
			descSets[i],
			0,
			0,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			&descBufferInfo,
			nullptr
		};
		vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, nullptr);
	}
	glm::vec3 camPos(0,0,0);
	glm::vec3 direction(0,0,0);
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	while(!glfwWindowShouldClose(window)){
		camPos=glm::vec3(uniformBufferObject.time*1+50,20,uniformBufferObject.time*1+50);
		int timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		start = std::chrono::system_clock::now();
		glfwPollEvents();

		VkCommandBufferAllocateInfo bufferAllocInfo = {};
		bufferAllocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocInfo.commandPool = commandPool;
		bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
		vkAllocateCommandBuffers(device, &bufferAllocInfo, commandBuffers.data());
		uniformBufferObject.time+=timeElapsed*0.001;
		uniformBufferObject.viewMatrix = glm::lookAt(camPos,direction,glm::vec3(0.0f,1.0f,0.0f));
		uniformBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)extent.width/(float)extent.height, 0.1f, 1000.0f);
		uniformBufferObject.projectionMatrix[1][1] *= -1;
		uint32_t i = 1;
		for(unsigned int i=0;i<imageCount;i++){
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional
			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = Framebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = extent;
			VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;
				vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSets[i], 0, nullptr);

			for (Entity &entity: Entities){
				entity.Angle.y=uniformBufferObject.time;
				uniformBufferObject.modelMatrix=glm::mat4(1.0f);
				uniformBufferObject.modelMatrix = glm::translate(uniformBufferObject.modelMatrix, entity.Position);
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.y,glm::vec3(0.0f,1.0f,0.0f));
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.x,glm::vec3(1.0f,0.0f,0.0f));
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.z,glm::vec3(0.0f,0.0f,1.0f));


				void* data;
				vkMapMemory(device, uniformBufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
				memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
				vkUnmapMemory(device, uniformBufferMemory);

				VkBuffer vertexBuffers[] = {entity.vertexBuffer};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
				if(entity.hasIndexBuffer){
					vkCmdBindIndexBuffer(commandBuffers[i], entity.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(entity.Indices.size()), 1, 0, 0, 0);
				}
				else{
					vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(entity.Vertices.size()), 1, 0, 0);
				}
			}

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				std::cerr << "Render Pass Failed" << std::endl;
			}
		}
		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)!=VK_SUCCESS){
			std::cerr << "Queue submit Failed" << std::endl;
		}


		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = {swapchain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		vkQueuePresentKHR(graphicsQueue, &presentInfo);
		vkQueueWaitIdle(graphicsQueue);








		 end = std::chrono::system_clock::now();
	}


	vkDeviceWaitIdle(device);
	for (Entity &entity : Entities){
		entity.destroyBuffers(device);
	}
	
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(device, commandPool, nullptr);
	for(unsigned int i=0;i<imageCount;i++){
		vkDestroyFramebuffer(device,Framebuffers[i],nullptr);
	}

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	for(unsigned int i=0;i<imageCount;i++){
		vkDestroyImageView(device,ImageViews[i],nullptr);
	}
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyBuffer(device, uniformBuffer, nullptr);
	vkFreeMemory(device,uniformBufferMemory,nullptr);
	vkDestroyDescriptorPool(device, descPool, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroySurfaceKHR(vkinstance,surface, nullptr);
	vkDestroyDevice(device,nullptr);
	vkDestroyInstance(vkinstance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

}
