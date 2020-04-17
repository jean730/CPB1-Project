#include "client/engine.h"
Engine::Engine(std::string name,int width,int height){
	this->WIDTH=width;
	this->HEIGHT=height;
	this->extent = {width,height};
	this->engineName=name;
	this->initVulkan();
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
        this->window = glfwCreateWindow(WIDTH, HEIGHT, this->engineName.c_str(), NULL,NULL);

	//Initialize a vulkan instance:
	{
		this->applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		this->applicationInfo.pApplicationName = this->engineName.c_str();
		this->applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		this->applicationInfo.pEngineName = this->engineName.c_str();
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
		//selectedLayers.push_back("VK_LAYER_KHRONOS_validation");
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
		for (long unsigned int i=0;i<queueFamilies.size();i++) {
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

		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = this->surface;
		swapchainCreateInfo.minImageCount = this->imageCount;
		swapchainCreateInfo.imageFormat = this->format.format;
		swapchainCreateInfo.imageColorSpace = this->format.colorSpace;
		swapchainCreateInfo.imageExtent = this->extent;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.preTransform = capabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		vkCreateSwapchainKHR(this->logicalDevice,&swapchainCreateInfo,nullptr,&this->swapchain);
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
