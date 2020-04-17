#include "client/engine.h"

Engine::Engine(std::string name,int width,int height){
	this->WIDTH=width;
	this->HEIGHT=height;
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
        this->window = glfwCreateWindow(WIDTH, HEIGHT, "test", NULL,NULL);

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

}