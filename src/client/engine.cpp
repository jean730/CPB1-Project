#include "client/engine.h"
#include "client/shaderloader.h"
Engine::Engine(std::string name,uint32_t width,uint32_t height,int max_framerate){
	this->WIDTH=width;
	this->HEIGHT=height;
	this->extent = {width,height};
	this->ENGINE_NAME=name;
	this->FRAMETIME = 1000/max_framerate;
}

Engine::~Engine(){
	vkDeviceWaitIdle(this->logicalDevice);
        for (Entity &entity : this->Entities){
                entity.destroyBuffers(this->logicalDevice);
        }

        vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphore, nullptr);
        vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphore, nullptr);
        vkDestroyFence(this->logicalDevice, this->Fence, nullptr);

        vkFreeMemory(this->logicalDevice,this->depthImageMemory,nullptr);
        vkDestroyImage(this->logicalDevice,this->depthImage,nullptr);
        vkDestroyImageView(this->logicalDevice,this->depthImageView,nullptr);
        vkDestroyCommandPool(this->logicalDevice, this->commandPool, nullptr);
        for(unsigned int i=0;i<this->imageCount;i++){
                vkDestroyFramebuffer(this->logicalDevice,this->Framebuffers[i],nullptr);
        }

        vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);
        vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);
        vkDestroyShaderModule(this->logicalDevice, this->shaderPair->fragmentShader, nullptr);
        vkDestroyShaderModule(this->logicalDevice, this->shaderPair->vertexShader, nullptr);
        for(unsigned int i=0;i<this->imageCount;i++){
                vkDestroyImageView(this->logicalDevice,this->imageViews[i],nullptr);
        }
        vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);
        vkDestroyBuffer(this->logicalDevice, this->uniformBuffer, nullptr);
        vkFreeMemory(this->logicalDevice,this->uniformBufferMemory,nullptr);
        vkDestroyDescriptorPool(this->logicalDevice, this->descPool, nullptr);
        vkDestroyDescriptorSetLayout(this->logicalDevice, this->descriptorSetLayout, nullptr);
        vkDestroySurfaceKHR(this->vkinstance,this->surface, nullptr);
        vkDestroyDevice(this->logicalDevice,nullptr);
        vkDestroyInstance(this->vkinstance, NULL);
        glfwDestroyWindow(this->window);
        glfwTerminate();
        delete this->shaderPair;
}

void Engine::initVulkan(){
	//Initialize GLFW
	glfwInit();

	//Check for Vulkan support
	if(glfwVulkanSupported() == GLFW_FALSE){
                std::cerr << "Vulkan Not Supported !" << std::endl;
                glfwTerminate();
		exit(-1);
        }

	//Create window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        this->window = glfwCreateWindow(WIDTH, HEIGHT, this->ENGINE_NAME.c_str(), NULL,NULL);

	//Initialize a vulkan instance:
	{
		this->applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		this->applicationInfo.pApplicationName = this->ENGINE_NAME.c_str();
		this->applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		this->applicationInfo.pEngineName = this->ENGINE_NAME.c_str();
		this->applicationInfo.engineVersion = VK_MAKE_VERSION(1, 1, 0);
		this->applicationInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	        createInfo.pApplicationInfo = &this->applicationInfo;

		//Load Extensions
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
		if(this->ENABLE_VALIDATION_LAYERS){
			selectedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		}
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

		vkCreateInstance(&createInfo, nullptr,&this->vkinstance);

	}

	//Select a physical device
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->vkinstance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(this->vkinstance, &deviceCount, devices.data());
		std::cout << "Available devices:" << std::endl;
		std::string deviceName;
		for (uint32_t i=0;i<devices.size();i++) {
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(devices[i],&physicalDeviceProperties);
			std::cout << "\t- " << physicalDeviceProperties.deviceName << std::endl;
			//Select the first GPU then search for a discrete one.
			if(this->physicalDevice==NULL or
					physicalDeviceProperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
				this->physicalDevice=devices[i];
				deviceName=physicalDeviceProperties.deviceName;
			}
		}
		std::cout << "Selected device: " << std::endl << "\t- " << deviceName << std::endl;

	}
	
	//Get queue families IDs
	{
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, queueFamilies.data());
		for (unsigned int i=0;i<queueFamilies.size();i++) {
			if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
				this->graphicsFamily = i;
				std::cout << "Graphics queue family ID:" << std::endl << "\t- " << i << std::endl;
			}
		}

	}

	//Initialize Logical Device
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = this->graphicsFamily;
		queueCreateInfo.queueCount = 1;
		float queuePrio = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePrio;

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

		if (vkCreateDevice(this->physicalDevice, &devCreateInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
			std::cerr << "Failed to create logical device." << std::endl ;
			glfwTerminate();
			exit(-1);
		}
	}

	//Get graphics Queue
	{
		vkGetDeviceQueue(this->logicalDevice, this->graphicsFamily, 0, &this->graphicsQueue);
	}

	//Create window surface
	{
		glfwCreateWindowSurface(this->vkinstance, this->window, nullptr, &this->surface);
	}

	//Create Swapchain
	{

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(this->physicalDevice, this->graphicsFamily, surface, &presentSupport);
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
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice, surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, surface, &formatCount, nullptr);
		if (formatCount != 0) {
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, surface, &formatCount, formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice, surface, &presentModeCount, presentModes.data());
		}

		this->format = formats[0];
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for(unsigned int i=0;i<presentModeCount;i++){
			if(presentModes[i]==VK_PRESENT_MODE_MAILBOX_KHR){
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				std::cout << "Surface supports triple buffering." << std::endl;
			}
		}
		this->imageCount=capabilities.minImageCount+1;
		if(capabilities.minImageCount == capabilities.maxImageCount){
			this->imageCount-=1;
		}

		VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.surface = this->surface;
		swapChainCreateInfo.minImageCount = this->imageCount;
		swapChainCreateInfo.imageFormat = this->format.format;
		swapChainCreateInfo.imageColorSpace = this->format.colorSpace;
		swapChainCreateInfo.imageExtent = this->extent;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.preTransform = capabilities.currentTransform;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = presentMode;
		swapChainCreateInfo.clipped = VK_TRUE;
		swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		vkCreateSwapchainKHR(this->logicalDevice,&swapChainCreateInfo,nullptr,&this->swapChain);
	}

	//Create Image Views
	{
		vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &this->imageCount, nullptr);
		this->swapChainImages.resize(this->imageCount);
		vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &this->imageCount, this->swapChainImages.data());
		VkFormat swapChainImageFormat = this->format.format;
		this->imageViews.resize(this->imageCount);

		for(unsigned int i=0;i<this->imageCount;i++){
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = this->swapChainImages[i];
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
			vkCreateImageView(this->logicalDevice,&imageViewCreateInfo,nullptr,&this->imageViews[i]);
		}
	}
	
	//Configure Vertex Input
	{
		this->vertexInputAttributeDescriptions.resize(4);
		for(unsigned int i=0;i<4;i++){
			this->vertexInputAttributeDescriptions[i].binding=0;
			this->vertexInputAttributeDescriptions[i].location=i;
			this->vertexInputAttributeDescriptions[i].format=VK_FORMAT_R32G32B32_SFLOAT;
		}
		this->vertexInputAttributeDescriptions[0].offset=offsetof(Vertex,Vertex::Position);
		this->vertexInputAttributeDescriptions[1].offset=offsetof(Vertex,Vertex::Color);
		this->vertexInputAttributeDescriptions[2].offset=offsetof(Vertex,Vertex::Normal);
		this->vertexInputAttributeDescriptions[3].offset=offsetof(Vertex,Vertex::UV);
		this->vertexInputAttributeDescriptions[3].format=VK_FORMAT_R32G32_SFLOAT;

		this->vertexInputBindingDescription.binding=0;
		this->vertexInputBindingDescription.stride=sizeof(Vertex);
		this->vertexInputBindingDescription.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;

		this->vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		this->vertexInputInfo.vertexBindingDescriptionCount = 1;
		this->vertexInputInfo.vertexAttributeDescriptionCount = 4;
		this->vertexInputInfo.pVertexBindingDescriptions=&this->vertexInputBindingDescription;
		this->vertexInputInfo.pVertexAttributeDescriptions=this->vertexInputAttributeDescriptions.data();

		this->inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		this->inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		this->inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	//Create Viewport State Info
	{
		this->viewport.x = 0.0f;
		this->viewport.y = 0.0f;
		this->viewport.width = (float)this->extent.width;
		this->viewport.height = (float)this->extent.height;
		this->viewport.minDepth = 0.0f;
		this->viewport.maxDepth = 1.0f;

		this->scissor.offset = {0, 0};
		this->scissor.extent = this->extent;

		this->viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		this->viewportStateInfo.viewportCount = 1;
		this->viewportStateInfo.pViewports = &this->viewport;
		this->viewportStateInfo.scissorCount = 1;
		this->viewportStateInfo.pScissors = &this->scissor;
	}

	//Create Rastezization State Info
	{
		this->rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		this->rasterizer.depthClampEnable = VK_FALSE;

		this->rasterizer.rasterizerDiscardEnable = VK_FALSE;

		if(!this->ENABLE_WIREFRAME){
			this->rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		}
		else{
			this->rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
		}
		this->rasterizer.lineWidth = 1.0f;
		this->rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		this->rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		this->rasterizer.depthBiasEnable = VK_FALSE;
	}

	//Create Multisampler State Info	
	{
		multisampler.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampler.sampleShadingEnable = VK_FALSE;
		multisampler.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	//Create Colorblending State Info
	{
		this->colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		this->colorBlendAttachment.blendEnable = VK_FALSE;

		this->colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		this->colorBlending.logicOpEnable = VK_FALSE;
		this->colorBlending.attachmentCount = 1;
		this->colorBlending.pAttachments = &this->colorBlendAttachment;
	}

	//Create Depth Stencil State Info
	{
		this->depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		this->depthStencil.depthTestEnable = VK_TRUE;
		this->depthStencil.depthWriteEnable = VK_TRUE;
		this->depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		this->depthStencil.depthBoundsTestEnable = VK_FALSE;
		this->depthStencil.stencilTestEnable = VK_FALSE;
	}

	//Create Descriptor Set Layout
	{
		VkDescriptorSetLayoutBinding uniformBufferLayoutBinding={};
		uniformBufferLayoutBinding.binding=0;
		uniformBufferLayoutBinding.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferLayoutBinding.descriptorCount=1;
		uniformBufferLayoutBinding.stageFlags=VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			1,
			&uniformBufferLayoutBinding
		};
		vkCreateDescriptorSetLayout(this->logicalDevice, &layoutCreateInfo, nullptr, &this->descriptorSetLayout);
	}

	//Create Pipeline Layout
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; 
		pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout; 
		pipelineLayoutInfo.pushConstantRangeCount = 0; 
		pipelineLayoutInfo.pPushConstantRanges = nullptr;


		if (vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
		    glfwTerminate();
		    exit(-1);
		}
	}

	//Create Depth Attachment Description + Reference
	{
		this->depthAttachment.format = VK_FORMAT_D32_SFLOAT;
		this->depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		this->depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		this->depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		this->depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		this->depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		this->depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		this->depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		this->depthAttachmentRef.attachment = 1;
		this->depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	//Create Color Attachment Description + Reference 
	{
		this->colorAttachment.format = this->format.format;
		this->colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		this->colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		this->colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		this->colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // No need for a stencil buffer for now, maybe later.
		this->colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		this->colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		this->colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		this->colorAttachmentRef.attachment = 0;
		this->colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	//Create Subpass dependency
	{
		this->subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		this->subpassDependency.dstSubpass = 0;
		this->subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		this->subpassDependency.srcAccessMask = 0;
		this->subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		this->subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	//Create Subpass Description
	{
		this->subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		this->subpass.colorAttachmentCount = 1;
		this->subpass.pColorAttachments = &this->colorAttachmentRef;
		this->subpass.pDepthStencilAttachment = &this->depthAttachmentRef;
	}

	//Create RenderPass Info
	{
		std::array<VkAttachmentDescription, 2> attachments = {this->colorAttachment, this->depthAttachment};
		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 2;
		renderPassCreateInfo.pAttachments =attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &this->subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &this->subpassDependency;

		if (vkCreateRenderPass(this->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
			std::cerr << "Could not create renderpass." << std::endl;
			exit(-1);
		}
	}

	//Create Graphics Pipeline
	{
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount=2;
		this->shaderPair = new ShaderPair(this,"assets/shaders/vert.sprv","assets/shaders/frag.sprv");
		pipelineCreateInfo.pStages=this->shaderPair->shaderCreateInfo;
		pipelineCreateInfo.pVertexInputState=&this->vertexInputInfo;
		pipelineCreateInfo.pInputAssemblyState=&this->inputAssembly;
		pipelineCreateInfo.pViewportState=&this->viewportStateInfo;
		pipelineCreateInfo.pRasterizationState=&this->rasterizer;
		pipelineCreateInfo.pMultisampleState=&this->multisampler;
		pipelineCreateInfo.pColorBlendState=&this->colorBlending;
		pipelineCreateInfo.pDepthStencilState=&this->depthStencil;
		pipelineCreateInfo.layout=this->pipelineLayout;
		pipelineCreateInfo.renderPass=this->renderPass;
		pipelineCreateInfo.subpass=0;

		if (vkCreateGraphicsPipelines(this->logicalDevice,VK_NULL_HANDLE,1, &pipelineCreateInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS) {
			std::cerr << "Could not create the graphics pipeline." << std::endl;
			exit(-1);
		}
		else{
		    std::cout << "Graphics Pipeline successfully created." << std::endl;
		}
	}

	//Configure Depth Image
	{
		VkImageCreateInfo depthImageInfo = {};
		depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImageInfo.extent.width = this->extent.width;
		depthImageInfo.extent.height = this->extent.height;
		depthImageInfo.extent.depth = 1;
		depthImageInfo.mipLevels = 1;
		depthImageInfo.arrayLayers = 1;
		depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
		depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateImage(this->logicalDevice, &depthImageInfo, nullptr, &this->depthImage);

		VkMemoryRequirements depthMemoryRequirements;
		vkGetImageMemoryRequirements(this->logicalDevice, this->depthImage, &depthMemoryRequirements);
		VkPhysicalDeviceMemoryProperties depthMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &depthMemoryProperties);

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

		vkAllocateMemory(this->logicalDevice, &depthBufferAllocInfo, nullptr, &this->depthImageMemory);
		vkBindImageMemory(this->logicalDevice, this->depthImage, this->depthImageMemory, 0);

		VkImageViewCreateInfo depthImageViewCreateInfo = {};
		depthImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthImageViewCreateInfo.image = this->depthImage;
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
		if(vkCreateImageView(this->logicalDevice,&depthImageViewCreateInfo,nullptr,&this->depthImageView)!=VK_SUCCESS){
			std::cerr << "Failed to create Depth Image View" << std::endl;
			glfwTerminate();
			exit(-1);
		}
	}

	//Create Framebuffers
	{
		this->Framebuffers.resize(this->imageCount);
		for(unsigned int i=0;i<this->imageCount;i++){

			VkImageView attachments[] = {
				this->imageViews[i],
				this->depthImageView
			};

			VkFramebufferCreateInfo framebufferCreateInfo = {};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = this->renderPass;
			framebufferCreateInfo.attachmentCount = 2;
			framebufferCreateInfo.pAttachments = attachments;
			framebufferCreateInfo.width = this->extent.width;
			framebufferCreateInfo.height = this->extent.height;
			framebufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(this->logicalDevice, &framebufferCreateInfo, nullptr, &this->Framebuffers[i]) != VK_SUCCESS) {
				std::cerr << "Could not create framebuffer "<< i << std::endl;
				glfwTerminate();
				exit(-1);
			}
		}
	}

	//Allocate Uniform Buffer
	{
		VkBufferCreateInfo uniformBufferCreateInfo = {};
		uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		uniformBufferCreateInfo.size = sizeof(uniformBufferStruct);
		uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(this->logicalDevice, &uniformBufferCreateInfo, nullptr, &this->uniformBuffer);
		VkMemoryRequirements memoryRequirements;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);
		vkGetBufferMemoryRequirements(this->logicalDevice, this->uniformBuffer, &memoryRequirements);
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
		if(vkAllocateMemory(this->logicalDevice,&memoryAllocateInfo,nullptr,&this->uniformBufferMemory) == VK_SUCCESS){
			std::cout << "Allocated Memory for uniform buffer: " << memoryRequirements.size << std::endl;
			vkBindBufferMemory(this->logicalDevice, this->uniformBuffer, this->uniformBufferMemory, 0);
		}
	}

	//Create Descriptor Pool
	{
		VkDescriptorPoolSize descPoolSize = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			this->imageCount
		};
		VkDescriptorPoolCreateInfo descPoolCreateInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			NULL,
			0,
			this->imageCount,
			1,
			&descPoolSize
		};

		if(vkCreateDescriptorPool(this->logicalDevice,&descPoolCreateInfo,nullptr,&this->descPool)  != VK_SUCCESS) {
			std::cerr << "Could not create Descriptor Pool " << std::endl;
			glfwTerminate();
			exit(-1);
		}
	}

	//Create Descriptor Set Layouts
	{
		std::vector<VkDescriptorSetLayout> layouts(this->imageCount, descriptorSetLayout);

		VkDescriptorSetAllocateInfo descAllocInfo = {
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			NULL,
			descPool,
			this->imageCount,
			layouts.data()
		};

		this->descSets.resize(this->imageCount);
		if(vkAllocateDescriptorSets(this->logicalDevice,&descAllocInfo,descSets.data())  != VK_SUCCESS) {
			std::cerr << "Could not create Descriptor Sets " << std::endl;
			glfwTerminate();
			exit(-1);
		}
	}

	//Create Command Pool + Buffers
	{
		VkCommandPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCreateInfo.queueFamilyIndex=this->graphicsFamily;
		poolCreateInfo.flags=VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		if (vkCreateCommandPool(this->logicalDevice, &poolCreateInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
			std::cerr << "Could not create Command Pool " << std::endl;
			glfwTerminate();
			exit(-1);
		}
		this->commandBuffers.resize(this->imageCount);
	}

	//Create Semaphores
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore);
		vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore);
	}

	//Create Fences
	{
		VkFenceCreateInfo fenceCreateInfo={};
		fenceCreateInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(this->logicalDevice, &fenceCreateInfo, nullptr, &this->Fence);
	}


	//Asign uniform buffer to descriptor sets
	{
		for(unsigned int i=0;i<this->imageCount;i++){
			VkDescriptorBufferInfo descBufferInfo = {
				this->uniformBuffer,
				0,
				VK_WHOLE_SIZE
			};

			VkWriteDescriptorSet writeDescriptor = {
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				NULL,
				this->descSets[i],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				nullptr,
				&descBufferInfo,
				nullptr
			};
			vkUpdateDescriptorSets(this->logicalDevice, 1, &writeDescriptor, 0, nullptr);
		}
	}


}

void Engine::initTerrain(int size){
	for(int16_t x=-size;x<size;x++){
                for(int16_t y=-size;y<size;y++){
                        this->Entities.push_back(createTerrain(4,1,x,y,20.0f,0.008,2));
			this->Entities[Entities.size()-1].createBuffers(this->logicalDevice,this->physicalDevice);
                }
        }

}
