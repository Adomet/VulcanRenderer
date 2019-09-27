#include <assert.h>
#include <stdio.h>


#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while (0)

VkInstance CreateInstance()
{
	 

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.applicationVersion = VK_API_VERSION_1_1;


	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
	const char* debuglayers[] =
	{
	  "VK_LAYER_KHRONOS_validation"
	};

	createInfo.ppEnabledLayerNames = debuglayers;
	createInfo.enabledLayerCount = sizeof(debuglayers) / sizeof(debuglayers[0]);
#endif

	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		//VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};


	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);


	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

	return instance;
}





VkPhysicalDevice pickPhsicalDevice(VkPhysicalDevice* physicalDevices, uint32_t physicalDeviceCount)
{
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{


		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i],&props );

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			printf("Picking discreate GPU: %s\n", props.deviceName);
			return physicalDevices[i];
		}
	}

	if (physicalDeviceCount > 0)
	{

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[0], &props);

		printf("Picking fallback GPU: %s\n", props.deviceName);
		return physicalDevices[0];
	}


	printf("No phsical device available!!!");

	return VK_NULL_HANDLE;

}


VkSurfaceKHR createSurface(VkInstance instance,GLFWwindow* window)
{


#if defined(VK_USE_PLATFORM_WIN32_KHR)

	VkWin32SurfaceCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	createInfo.hinstance = GetModuleHandle(0);
	createInfo.hwnd = glfwGetWin32Window(window);

	VkSurfaceKHR surface = 0;
	vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface);
	return surface;

#else
#error Unsupported Platform
#endif
}



VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t* familyIndex)
{
	//*familyIndex = 0; 

	float queuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	const char* extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VkDevice device = 0;
	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

	return device;
}

VkSwapchainKHR createSwapChain(VkDevice device,VkSurfaceKHR surface,uint32_t windowWidth,uint32_t windowHeight,uint32_t familyIndex)
{

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = windowWidth;
	createInfo.imageExtent.height = windowHeight;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.queueFamilyIndexCount = 1,
	createInfo.pQueueFamilyIndices = &familyIndex;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;


	VkSwapchainKHR swapchain = 0;
	vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain);

	return swapchain;

}

VkSemaphore createSemaphore(VkDevice device)
{


	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };


	VkSemaphore semaphore;
	VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));
	return semaphore;
}

int main()
{

	int rc = glfwInit();
	assert(rc);

	VkInstance instance =  CreateInstance();



	VkPhysicalDevice phsicalDevices[16];
	uint32_t phsicalDeviceCount=sizeof(phsicalDevices)/sizeof(phsicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &phsicalDeviceCount, phsicalDevices));

	VkPhysicalDevice phsicalDevice = pickPhsicalDevice(phsicalDevices,phsicalDeviceCount);
	assert(phsicalDevice);
	
	uint32_t familyIndex = 0;
	VkDevice device = createDevice(instance, phsicalDevice,0);
	assert(device);




	GLFWwindow* window =  glfwCreateWindow(1024, 768, "Vulcan Renderer", 0, 0);
	assert(window);


	VkSurfaceKHR surface = createSurface(instance,window);
	assert(surface);

	int windowWidth=0, windowHeight=0;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	VkSwapchainKHR swapchain = createSwapChain(device, surface, windowWidth, windowHeight,familyIndex);



	VkSemaphore semaphore = createSemaphore(device);

	VkQueue queue;
	vkGetDeviceQueue(device, 0, familyIndex, &queue);
	
	VkImage swapchainImages[16];

	uint32_t swapchainCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainCount, swapchainImages));


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();


		uint32_t imageIndex = 0;
		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, NULL, semaphore, VK_NULL_HANDLE, &imageIndex));

		VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
		presentInfo.swapchainCount=1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;


	//	VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));
		
		VK_CHECK(vkDeviceWaitIdle(device));

	}

	glfwDestroyWindow(window);

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	
	return 0;
}