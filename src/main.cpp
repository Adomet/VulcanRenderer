#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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



int main()
{
	int rc = glfwInit();
	assert(rc);

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;


	// Validation Layers Code start Here

#ifdef _DEBUG
	const char* debugLayer[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};

	createInfo.ppEnabledLayerNames = debugLayer;
	createInfo.enabledLayerCount = sizeof(debugLayer) / sizeof(debugLayer[0]);
#endif // _DEBUG


	const char* extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);


	VkDebugUtilsMessengerEXT debugMessenger = 0;
	VkDebugUtilsMessengerCreateInfoEXT debugcreateInfo = {};
	debugcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugcreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugcreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugcreateInfo.pfnUserCallback = debugCallback;


	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)& debugcreateInfo;


	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));


	//Validation Layer Code ends herer (which the guy not even tried to write debug messenger thing)

	GLFWwindow* window = glfwCreateWindow(1024,768,"Rogue Vulcan",0,0);
	assert(window);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}


	vkDestroyInstance(instance, 0);
	glfwDestroyWindow(window);
	glfwTerminate();
	


	return 0;
}