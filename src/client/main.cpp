#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define PI 3.14159265
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "client/vertex.h"
#include "client/entity.h"
#include "client/uniform.h"
#include "client/terrain.h"
#include "client/engine.h"
#include "client/shaderloader.h"
#include <math.h>
#include <chrono>
#include <thread>
bool WIREFRAME_MODE=false;
int main(){
	Engine *engineInstance = new Engine("Test Engine",1280,720);
	engineInstance->PREFER_TRIPLE_BUFFERING=false;
	engineInstance->initVulkan();
	engineInstance->initTerrain(6);

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	ShaderPair shaderPair(engineInstance,"assets/shaders/vert.sprv","assets/shaders/frag.sprv");


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
	viewport.width = (float)engineInstance->extent.width;
	viewport.height = (float)engineInstance->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = engineInstance->extent;

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
	vkCreateDescriptorSetLayout(engineInstance->logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional


	if (vkCreatePipelineLayout(engineInstance->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
	    std::cerr << "Could not create pipeline layout." << std::endl;
	    glfwTerminate();
	}
	else{
	    std::cout << "Pipeline layout successfully created." << std::endl;
	}

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT; 
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	


	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = engineInstance->format.format;
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
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments =attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(engineInstance->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		std::cerr << "Could not create renderpass." << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Renderpass successfully created." << std::endl;
	}


	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount=2;
	pipelineCreateInfo.pStages=shaderPair.shaderCreateInfo;
	pipelineCreateInfo.pVertexInputState=&vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState=&inputAssembly;
	pipelineCreateInfo.pViewportState=&viewportStateInfo;
	pipelineCreateInfo.pRasterizationState=&rasterizer;
	pipelineCreateInfo.pMultisampleState=&multisampler;
	pipelineCreateInfo.pColorBlendState=&colorBlending;
	pipelineCreateInfo.pDepthStencilState=&depthStencil;
	pipelineCreateInfo.layout=pipelineLayout;
	pipelineCreateInfo.renderPass=renderPass;
	pipelineCreateInfo.subpass=0;

	VkPipeline graphicsPipeline; // FINALLY the graphics pipeline
	if (vkCreateGraphicsPipelines(engineInstance->logicalDevice,VK_NULL_HANDLE,1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		std::cerr << "Could not create the graphics pipeline." << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Graphics Pipeline successfully created." << std::endl;
	}
	
	std::vector<VkFramebuffer> Framebuffers;
	VkImageView depthImageView;
	VkImageCreateInfo depthImageInfo = {};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.extent.width = engineInstance->extent.width;
	depthImageInfo.extent.height = engineInstance->extent.height;
	depthImageInfo.extent.depth = 1;
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	if (vkCreateImage(engineInstance->logicalDevice, &depthImageInfo, nullptr, &depthImage) == VK_SUCCESS) {
		std::cout << "Depth buffer successfully created." << std::endl;
	}

	VkMemoryRequirements depthMemoryRequirements;
	vkGetImageMemoryRequirements(engineInstance->logicalDevice, depthImage, &depthMemoryRequirements);
	VkPhysicalDeviceMemoryProperties depthMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(engineInstance->physicalDevice, &depthMemoryProperties);

	VkMemoryAllocateInfo depthBufferAllocInfo = {};
	depthBufferAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depthBufferAllocInfo.allocationSize = depthMemoryRequirements.size;
	int tmp=0;
	for(unsigned int i=0;i<depthMemoryProperties.memoryTypeCount;i++){
		if(depthMemoryRequirements.memoryTypeBits &(1<<i) && (depthMemoryProperties.memoryTypes[i].propertyFlags &
					(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))==
					(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)){
				tmp=i;
		}
	}
	depthBufferAllocInfo.memoryTypeIndex = tmp; 

	if (vkAllocateMemory(engineInstance->logicalDevice, &depthBufferAllocInfo, nullptr, &depthImageMemory) != VK_SUCCESS) {
	    throw std::runtime_error("failed to allocate depthImage memory!");
	}

	vkBindImageMemory(engineInstance->logicalDevice, depthImage, depthImageMemory, 0);

	VkImageViewCreateInfo depthImageViewCreateInfo = {};
        depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthImageViewCreateInfo.image = depthImage;
        depthImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthImageViewCreateInfo.format = VK_FORMAT_D32_SFLOAT;
        depthImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        depthImageViewCreateInfo.subresourceRange.levelCount = 1;
        depthImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        depthImageViewCreateInfo.subresourceRange.layerCount = 1;
	if(vkCreateImageView(engineInstance->logicalDevice,&depthImageViewCreateInfo,nullptr,&depthImageView)==VK_SUCCESS){
		std::cout << "Depth Image View successfully created" << std::endl;
	}
	Framebuffers.resize(engineInstance->imageCount);
	for(unsigned int i=0;i<engineInstance->imageCount;i++){

		VkImageView attachments[] = {
			engineInstance->imageViews[i],
			depthImageView
	   	};
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = engineInstance->extent.width;
		framebufferCreateInfo.height = engineInstance->extent.height;
		framebufferCreateInfo.layers = 1;
		
		if (vkCreateFramebuffer(engineInstance->logicalDevice, &framebufferCreateInfo, nullptr, &Framebuffers[i]) != VK_SUCCESS) {
			std::cerr << "Could not create framebuffer "<< i << std::endl;
			glfwTerminate();
		}
		else{
		    std::cout << "Framebuffer " << i << " successfully created." << std::endl;
		}
	}

	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	
	VkBufferCreateInfo uniformBufferCreateInfo = {};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.size = sizeof(uniformBufferStruct);
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer(engineInstance->logicalDevice, &uniformBufferCreateInfo, nullptr, &uniformBuffer);
	VkMemoryRequirements memoryRequirements;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(engineInstance->physicalDevice, &memoryProperties);
	vkGetBufferMemoryRequirements(engineInstance->logicalDevice, uniformBuffer, &memoryRequirements);
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
	if(vkAllocateMemory(engineInstance->logicalDevice,&memoryAllocateInfo,nullptr,&uniformBufferMemory) == VK_SUCCESS){
		std::cout << "Allocated Memory for uniform buffer: " << memoryRequirements.size << std::endl;
		vkBindBufferMemory(engineInstance->logicalDevice, uniformBuffer, uniformBufferMemory, 0);
	}


	VkDescriptorPoolSize descPoolSize = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		engineInstance->imageCount
	};

	VkDescriptorPoolCreateInfo descPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		engineInstance->imageCount,
		1,
		&descPoolSize
	};

	VkDescriptorPool descPool;

	if(vkCreateDescriptorPool(engineInstance->logicalDevice,&descPoolCreateInfo,nullptr,&descPool)  != VK_SUCCESS) {
                std::cerr << "Could not create Descriptor Pool " << std::endl;
                glfwTerminate();
        }
        else{
		std::cout << "Descriptor Pool successfully created." << std::endl;
        }

	std::vector<VkDescriptorSetLayout> layouts(engineInstance->imageCount, descriptorSetLayout);

	VkDescriptorSetAllocateInfo descAllocInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descPool,
		engineInstance->imageCount,
		layouts.data()
	};

	std::vector<VkDescriptorSet> descSets(engineInstance->imageCount);
	if(vkAllocateDescriptorSets(engineInstance->logicalDevice,&descAllocInfo,descSets.data())  != VK_SUCCESS) {
                std::cerr << "Could not create Descriptor Sets " << std::endl;
                glfwTerminate();
        }
        else{
		std::cout << "Descriptor Sets successfully created." << std::endl;
        }
	





	VkCommandPool commandPool;
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex=engineInstance->graphicsFamily;
	poolCreateInfo.flags=VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(engineInstance->logicalDevice, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		std::cerr << "Could not create Command Pool " << std::endl;
		glfwTerminate();
	}
	else{
	    std::cout << "Command Pool successfully created." << std::endl;
	}















	std::vector<VkCommandBuffer> commandBuffers;
	commandBuffers.resize(engineInstance->imageCount);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkSemaphore imageAvailableSemaphore;
	vkCreateSemaphore(engineInstance->logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
	VkSemaphore renderFinishedSemaphore;
	vkCreateSemaphore(engineInstance->logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
	VkFence Fence;
	VkFenceCreateInfo fenceCreateInfo={};
	fenceCreateInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(engineInstance->logicalDevice, &fenceCreateInfo, nullptr, &Fence);

	uniformBufferStruct uniformBufferObject={};
	uniformBufferObject.time=0;	

	for(unsigned int i=0;i<engineInstance->imageCount;i++){
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
		vkUpdateDescriptorSets(engineInstance->logicalDevice, 1, &writeDescriptor, 0, nullptr);
	}
	glm::vec3 camPos(0,0,0);
	glm::vec2 eyeAngles(0,0);
	glm::vec3 eyeDirection(0,0,0);
	glm::vec3 sideDirection(0,0,0);
	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	glfwSetInputMode(engineInstance->window,GLFW_CURSOR,GLFW_CURSOR_HIDDEN);
	bool FORWARD=false;
	bool BACK=false;
	bool RIGHT=false;
	bool LEFT=false;
	bool UP=false;
	bool DOWN=false;
	bool SPRINT=false;
	float speedMultiplier;
	while(!glfwWindowShouldClose(engineInstance->window)){
		int timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
		start = std::chrono::system_clock::now();
		if (timeElapsed<10){
			std::this_thread::sleep_for((std::chrono::duration<double, std::milli>)(10-timeElapsed));
		}
		glfwPollEvents();
		FORWARD=glfwGetKey(engineInstance->window,GLFW_KEY_W)==GLFW_PRESS;
		BACK=glfwGetKey(engineInstance->window,GLFW_KEY_S)==GLFW_PRESS;
		LEFT=glfwGetKey(engineInstance->window,GLFW_KEY_A)==GLFW_PRESS;
		RIGHT=glfwGetKey(engineInstance->window,GLFW_KEY_D)==GLFW_PRESS;
		UP=glfwGetKey(engineInstance->window,GLFW_KEY_SPACE)==GLFW_PRESS;
		DOWN=glfwGetKey(engineInstance->window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS;
		SPRINT=glfwGetKey(engineInstance->window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS;
		speedMultiplier=(0.1+SPRINT*0.4)*timeElapsed/10;
		double mx;
		double my;

		glfwGetCursorPos(engineInstance->window,&mx,&my);
		eyeAngles.x+=(mx-engineInstance->extent.width/2)*0.001;
		if (not ((eyeAngles.y>=PI/2 and (my-engineInstance->extent.height/2) > 0) or (eyeAngles.y<=-PI/2 and (my-engineInstance->extent.height/2) <0))){
			eyeAngles.y+=(my-engineInstance->extent.height/2)*0.001;
		}
		glfwSetCursorPos(engineInstance->window,engineInstance->extent.width/2,engineInstance->extent.height/2);
		while (eyeAngles.x>=2*PI){
			eyeAngles.x-=2*PI;
		}
		while (eyeAngles.x<=0){
			eyeAngles.x+=2*PI;
		}

		eyeDirection = glm::vec3(cos(eyeAngles.x),-sin(eyeAngles.y),sin(eyeAngles.x));
		sideDirection = glm::vec3(cos(eyeAngles.x+PI/2),0,sin(eyeAngles.x+PI/2));
		float forward_speed=(FORWARD-BACK)*speedMultiplier;
		float side_speed=(RIGHT-LEFT)*speedMultiplier;
		float height_speed=(UP-DOWN)*speedMultiplier;
		camPos+=glm::vec3((float)eyeDirection.x*forward_speed,0,eyeDirection.z*forward_speed);
		camPos+=glm::vec3((float)sideDirection.x*side_speed,0,sideDirection.z*side_speed);
		camPos+=glm::vec3(0,height_speed,0);

		VkCommandBufferAllocateInfo bufferAllocInfo = {};
		bufferAllocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocInfo.commandPool = commandPool;
		bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
		vkAllocateCommandBuffers(engineInstance->logicalDevice, &bufferAllocInfo, commandBuffers.data());
		uniformBufferObject.time+=timeElapsed*0.001;
		uniformBufferObject.viewMatrix = glm::lookAt(camPos,camPos+eyeDirection,glm::vec3(0.0f,1.0f,0.0f));
		uniformBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)engineInstance->extent.width/(float)engineInstance->extent.height, 0.1f, 1000.0f);
		uniformBufferObject.projectionMatrix[1][1] *= -1;
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = engineInstance->extent;
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = {0.53f, 0.81f, 0.92f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		for(unsigned int i=0;i<engineInstance->imageCount;i++){
			renderPassInfo.framebuffer = Framebuffers[i];

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSets[i], 0, nullptr);

			for (Entity &entity: engineInstance->Entities){
				uniformBufferObject.modelMatrix = glm::mat4(1.0f);
				uniformBufferObject.modelMatrix = glm::translate(uniformBufferObject.modelMatrix, entity.Position);
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.y,glm::vec3(0.0f,1.0f,0.0f));
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.x,glm::vec3(1.0f,0.0f,0.0f));
				uniformBufferObject.modelMatrix = glm::rotate(uniformBufferObject.modelMatrix,entity.Angle.z,glm::vec3(0.0f,0.0f,1.0f));


				void* data;
				vkMapMemory(engineInstance->logicalDevice, uniformBufferMemory, 0, sizeof(uniformBufferObject), 0, &data);
				memcpy(data, &uniformBufferObject, sizeof(uniformBufferObject));
				vkUnmapMemory(engineInstance->logicalDevice, uniformBufferMemory);

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
			vkEndCommandBuffer(commandBuffers[i]);
		}
		for(unsigned int i=0;i<engineInstance->imageCount;i++){
			vkWaitForFences(engineInstance->logicalDevice, 1, &Fence, VK_TRUE, UINT64_MAX);
			vkResetFences(engineInstance->logicalDevice, 1, &Fence);

			uint32_t imageIndex;
			vkAcquireNextImageKHR(engineInstance->logicalDevice, engineInstance->swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
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
			if(vkQueueSubmit(engineInstance->graphicsQueue, 1, &submitInfo, Fence)!=VK_SUCCESS){
				std::cerr << "Queue submit Failed" << std::endl;
			}


			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			VkSwapchainKHR swapchains[] = {engineInstance->swapChain};
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapchains;
			presentInfo.pImageIndices = &imageIndex;
			vkQueuePresentKHR(engineInstance->graphicsQueue, &presentInfo);
		}
		vkQueueWaitIdle(engineInstance->graphicsQueue);
		vkFreeCommandBuffers(engineInstance->logicalDevice,commandPool,engineInstance->imageCount,commandBuffers.data());
		vkResetCommandPool(engineInstance->logicalDevice,commandPool,VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);







		 end = std::chrono::system_clock::now();
	}


	vkDeviceWaitIdle(engineInstance->logicalDevice);
	for (Entity &entity : engineInstance->Entities){
		entity.destroyBuffers(engineInstance->logicalDevice);
	}
	
	vkDestroySemaphore(engineInstance->logicalDevice, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(engineInstance->logicalDevice, imageAvailableSemaphore, nullptr);
	vkDestroyFence(engineInstance->logicalDevice, Fence, nullptr);

	vkFreeMemory(engineInstance->logicalDevice,depthImageMemory,nullptr);
	vkDestroyImage(engineInstance->logicalDevice,depthImage,nullptr);
	vkDestroyImageView(engineInstance->logicalDevice,depthImageView,nullptr);
	vkDestroyCommandPool(engineInstance->logicalDevice, commandPool, nullptr);
	for(unsigned int i=0;i<engineInstance->imageCount;i++){
		vkDestroyFramebuffer(engineInstance->logicalDevice,Framebuffers[i],nullptr);
	}

	vkDestroyPipeline(engineInstance->logicalDevice, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(engineInstance->logicalDevice, pipelineLayout, nullptr);
	vkDestroyRenderPass(engineInstance->logicalDevice, renderPass, nullptr);
	vkDestroyShaderModule(engineInstance->logicalDevice, shaderPair.fragmentShader, nullptr);
	vkDestroyShaderModule(engineInstance->logicalDevice, shaderPair.vertexShader, nullptr);
	for(unsigned int i=0;i<engineInstance->imageCount;i++){
		vkDestroyImageView(engineInstance->logicalDevice,engineInstance->imageViews[i],nullptr);
	}
	vkDestroySwapchainKHR(engineInstance->logicalDevice, engineInstance->swapChain, nullptr);
	vkDestroyBuffer(engineInstance->logicalDevice, uniformBuffer, nullptr);
	vkFreeMemory(engineInstance->logicalDevice,uniformBufferMemory,nullptr);
	vkDestroyDescriptorPool(engineInstance->logicalDevice, descPool, nullptr);
	vkDestroyDescriptorSetLayout(engineInstance->logicalDevice, descriptorSetLayout, nullptr);
	vkDestroySurfaceKHR(engineInstance->vkinstance,engineInstance->surface, nullptr);
	vkDestroyDevice(engineInstance->logicalDevice,nullptr);
	vkDestroyInstance(engineInstance->vkinstance, NULL);
	glfwDestroyWindow(engineInstance->window);
	glfwTerminate();
	delete engineInstance;

}
