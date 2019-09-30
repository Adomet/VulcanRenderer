#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#include <assert.h>
#include <iostream>
#include <vector>


#define VK_CHECK(call)\
do{\
 VkResult result_ =call;\
 assert(result_ == VK_SUCCESS);\
} while (0)


// standard way of call back
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount)
{
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			printf("Picking discreate GPU %s\n", props.deviceName);
			return physicalDevices[i];
		}
	}
	if (physicalDeviceCount > 0)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[0], &props);
		printf("Picking iGPU %s\n", props.deviceName);
		return physicalDevices[0];
	}

	printf("No physical Device available!");
	return 0;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
{
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = GetModuleHandle(0);
	createInfo.hwnd = glfwGetWin32Window(window);

	VkSurfaceKHR surface=0;
	vkCreateWin32SurfaceKHR(instance,&createInfo, 0,&surface);
	return surface;
#endif 
}

VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice,uint32_t* familyIndex)
{

	*familyIndex = 0;
	float queuePriorities[] = { 1.0f };
	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = *familyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;


	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};


	VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueInfo;

	deviceCreateInfo.ppEnabledExtensionNames = extensions;
	deviceCreateInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VkDevice device = 0;
	VK_CHECK(vkCreateDevice(physicalDevice, &deviceCreateInfo, 0, &device));

	return device;
}



int main()
{
	int rc = glfwInit();
	assert(rc);

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,//Window spesific
	};

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);



	// Validation Layers Code start Here


#ifdef _DEBUG
	const char* debugLayer[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	createInfo.ppEnabledLayerNames = debugLayer;
	createInfo.enabledLayerCount = sizeof(debugLayer) / sizeof(debugLayer[0]);
#endif // _DEBUG

	VkDebugUtilsMessengerEXT debugMessenger = 0;
	VkDebugUtilsMessengerCreateInfoEXT debugcreateInfo = {};
	debugcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugcreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugcreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugcreateInfo.pfnUserCallback = debugCallback;

	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugcreateInfo;


	//Validation Layer Code ends herer (which the guy not even tried to write debug messenger thing)



	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));


	VkPhysicalDevice physicalDevices[16];
	uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);

	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);

	uint32_t familyIndex = 0;
	VkDevice device = createDevice(instance, physicalDevice,&familyIndex);


	GLFWwindow* window = glfwCreateWindow(1024,768,"Rogue Vulcan",0,0);
	assert(window);


	VkSurfaceKHR surface = createSurface(instance, window);
	assert(surface);


	int windWidth = 0, windHeight = 0;
	glfwGetWindowSize(window, &windWidth, &windHeight);
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = 2;
	swapchainCreateInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent.width = windWidth;
	swapchainCreateInfo.imageExtent.height= windHeight;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.queueFamilyIndexCount = 1;
	swapchainCreateInfo.pQueueFamilyIndices = &familyIndex;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;




	VkSwapchainKHR swapchain=0;
	vkCreateSwapchainKHR(device, &swapchainCreateInfo, 0, &swapchain);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}


	vkDestroyInstance(instance, 0);
	glfwDestroyWindow(window);
	glfwTerminate();
	


	return 0;
}