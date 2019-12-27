#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
void err(int errnum, const char* err){
	std::cerr << errnum << " -- " << err << std::endl;
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

	window = glfwCreateWindow(800, 600, "test", NULL,NULL);

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
	auto exts = glfwGetRequiredInstanceExtensions(&count);
	createInfo.enabledExtensionCount = count;
	createInfo.ppEnabledExtensionNames = exts;
	std::cout << "Loaded extensions:" << std::endl;
	for(unsigned int i=0;i<count;i++){
		std::cout << "\t- " << createInfo.ppEnabledExtensionNames[i] << std::endl;
	}
	
	vkCreateInstance(&createInfo, NULL,&vkinstance);
//	if (vkCreateInstance(&createInfo, nullptr, &vkinstance) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create instance!");
//	}
//	uint32_t extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
//	std::vector<VkExtensionProperties> extensions(extensionCount);
//	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions.data());
//
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
	VkDeviceCreateInfo devCreateInfo = {};
	devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	devCreateInfo.queueCreateInfoCount = 1;
	devCreateInfo.pEnabledFeatures = &deviceFeatures;
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
	
	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();

	}
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(vkinstance,surface, nullptr);
	vkDestroyInstance(vkinstance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();

}
