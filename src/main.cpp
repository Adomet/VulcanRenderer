#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
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
	printf("Devices:\n");

	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties props;

		

		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

		printf("%s\n",props.deviceName);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			printf("Picking discreate GPU %s \n", props.deviceName);
			return physicalDevices[i];
		}
	}
	if (physicalDeviceCount > 0)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[0], &props);
		printf("Picking iGPU %s \n", props.deviceName);
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



VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, uint32_t width, uint32_t height)
{
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM; // SHORTCUT: some devices only support BGRA
	createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR; // VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = width;
	createInfo.imageExtent.height = height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; 
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = &familyIndex;
	createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped= VK_TRUE;


	VkSwapchainKHR swapchain = 0;
	vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain);

	return swapchain;
}


VkSemaphore createSemaphore(VkDevice device)
{

	VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO};
	VkSemaphore semaphore = 0;
	VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, 0, &semaphore));

	return semaphore;
}

VkCommandPool createCommandPool(VkDevice device,uint32_t familyindex )//Commmand Buffer Pool
{

	VkCommandPool commandPool;
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = familyindex;
	VK_CHECK( vkCreateCommandPool(device, &createInfo, 0,&commandPool));
	return commandPool;
}

int main()
{
	int rc = glfwInit();
	assert(rc);

	if (glfwVulkanSupported())
	{
		printf("supported glfw vulkan\n");
	}

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
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


	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Rogue Vulcan", 0, 0);
	assert(window);

	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (err)
	{
		printf("surface creation failed");
	}


	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supported);
	assert(supported);


	int windowWidth = 0, windowHeight = 0;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	VkSwapchainKHR swapchain = createSwapchain(device,surface,familyIndex,windowWidth,windowHeight);
	assert(swapchain);

	

	VkSemaphore acuquireSemaphore = createSemaphore(device);
	assert(acuquireSemaphore);


	VkSemaphore releaseSemaphore = createSemaphore(device);
	assert(releaseSemaphore);

	VkQueue queue = 0;
	vkGetDeviceQueue(device, familyIndex, 0, &queue);

	VkImage swapchainImages[16];
	uint32_t swapchainImageCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
	VK_CHECK( vkGetSwapchainImagesKHR(device, swapchain,&swapchainImageCount,swapchainImages));

	VkCommandPool commandPool;
	commandPool = createCommandPool(device, familyIndex);
	assert(commandPool);


	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = 0;
	VK_CHECK(vkAllocateCommandBuffers(device,&allocateInfo, &commandBuffer));

	

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		uint32_t ImageIndex = 0;
		VK_CHECK(vkAcquireNextImageKHR(device,swapchain,0,acuquireSemaphore,VK_NULL_HANDLE,&ImageIndex));

		VK_CHECK(vkResetCommandPool(device, commandPool, 0));

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK( vkBeginCommandBuffer(commandBuffer,&beginInfo));

		VkClearColorValue color = {1,0,1,1};
		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;

		vkCmdClearColorImage(commandBuffer, swapchainImages[ImageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);

		VK_CHECK (vkEndCommandBuffer(commandBuffer));


		VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &acuquireSemaphore;
		submitInfo.pWaitDstStageMask = &submitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &releaseSemaphore;




		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);


		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount;
		presentInfo.pWaitSemaphores = &releaseSemaphore;


		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &ImageIndex;


		VK_CHECK(vkQueuePresentKHR(queue,&presentInfo));

		VK_CHECK(vkDeviceWaitIdle(device));
	}



	glfwDestroyWindow(window);
	glfwTerminate();
	vkDestroyInstance(instance, 0);


	return 0;
 }