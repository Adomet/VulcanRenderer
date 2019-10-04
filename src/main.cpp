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

VkFormat getSwapchainFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{

	VkSurfaceFormatKHR formats[16];
	uint32_t formatCount = sizeof(formats)/sizeof(formats[0]);


	assert(formatCount > 0); // if no support on any color format or space thing

	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,surface,&formatCount,formats));

	return formats[0].format;
}



VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t familyIndex, uint32_t width, uint32_t height,VkFormat format)
{
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = format; 
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

	VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
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

VkRenderPass createRenderPass(VkDevice device,VkFormat format)
{
	


	VkAttachmentDescription attachments[1] = {};

	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp= VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;




	VkAttachmentReference colorAttachments = { 0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };


	VkSubpassDescription subpass = {};

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount=1;
	subpass.pColorAttachments=&colorAttachments;

	VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	  createInfo.attachmentCount=sizeof(attachments)/sizeof(attachments[0]);
	  createInfo.pAttachments=attachments;
	  createInfo.subpassCount=1;
	  createInfo.pSubpasses=&subpass;






	VkRenderPass RenderPass = 0;
	VK_CHECK(vkCreateRenderPass(device,&createInfo,0,&RenderPass));

	return RenderPass;

}

VkFramebuffer createFrameBuffer(VkDevice device,VkRenderPass renderPass,VkImageView imageView,uint32_t width, uint32_t height)
{

	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	
	
	createInfo.renderPass=renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &imageView;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;


	VkFramebuffer frameBuffer = 0;
	VK_CHECK(vkCreateFramebuffer(device,&createInfo,0,&frameBuffer));

	return  frameBuffer;
}


VkImageView creaeteImageView(VkDevice device, VkImage image,VkFormat format)
{

	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView view = 0;
	VK_CHECK(vkCreateImageView(device, &createInfo, 0, &view));
	return view;
}

VkShaderModule loadShader(VkDevice device, const char* path)
{
	FILE* file = fopen(path, "rb");
	assert(file);
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	char* buffer = new char[length];
	assert(buffer);

	size_t rc = fread(buffer, 1, length, file);
	assert(rc == size_t(length));
	fclose(file);


	VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	createInfo.codeSize = length;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

	VkShaderModule shaderModule=0;
	vkCreateShaderModule(device, &createInfo, 0, &shaderModule);

	return shaderModule;
}

VkPipelineLayout createPipeLineLayout(VkDevice device)
{

	VkPipelineLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	VkPipelineLayout layout = 0;


	VK_CHECK(vkCreatePipelineLayout(device,&createInfo,0,&layout));

	return layout;

}

VkPipeline createGraphicsPipeline(VkDevice device,VkPipelineCache pipelineCache,VkRenderPass renderPass,VkShaderModule vertShader,VkShaderModule fragShader,VkPipelineLayout layout)
{

	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vertShader;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fragShader;
	stages[1].pName = "main";


	createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	createInfo.pVertexInputState = &vertexInputCreateInfo;


	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;
	createInfo.pViewportState = &viewportStateCreateInfo;

	VkPipelineRasterizationStateCreateInfo resterizationState = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	resterizationState.lineWidth = 1.f;
	createInfo.pRasterizationState = &resterizationState;

	VkPipelineMultisampleStateCreateInfo  MultisampleState   = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &MultisampleState;

    VkPipelineDepthStencilStateCreateInfo DepthStencilState	 = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	createInfo.pDepthStencilState = &DepthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


    VkPipelineColorBlendStateCreateInfo   ColorBlendState	 = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	ColorBlendState.attachmentCount = 1;
	ColorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &ColorBlendState;
	

	VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
	

    VkPipelineDynamicStateCreateInfo      DynamicState		 = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	DynamicState.dynamicStateCount = sizeof(DynamicStates) / sizeof(DynamicStates[0]);
	DynamicState.pDynamicStates = DynamicStates;
	createInfo.pDynamicState = &DynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, 0, &pipeline));

	return pipeline;

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
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));


	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);

	uint32_t familyIndex = 0;
	VkDevice device = createDevice(instance, physicalDevice,&familyIndex);
	assert(device);


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


	VkFormat swapchainFormat = getSwapchainFormat(physicalDevice,surface);


	VkSwapchainKHR swapchain = createSwapchain(device,surface,familyIndex,windowWidth,windowHeight,swapchainFormat);
	assert(swapchain);

	

	VkSemaphore acuquireSemaphore = createSemaphore(device);
	assert(acuquireSemaphore);


	VkSemaphore releaseSemaphore = createSemaphore(device);
	assert(releaseSemaphore);

	VkQueue queue = 0;
	vkGetDeviceQueue(device, familyIndex, 0, &queue);



	VkRenderPass renderPass = createRenderPass(device, swapchainFormat);
	assert(renderPass);

	VkShaderModule vertshader = loadShader(device, "shaders/vert.spv");
	assert(vertshader);

	VkShaderModule fragshader = loadShader(device, "shaders/frag.spv");
	assert(fragshader);

	// TODO: this is critical for performance!
	VkPipelineCache pipelineCache = 0;

	VkPipelineLayout layout = createPipeLineLayout(device);
	assert(layout);

	VkPipeline Pipeline = createGraphicsPipeline(device, pipelineCache, renderPass, vertshader, fragshader, layout);
	assert(Pipeline);



	VkImage swapchainImages[16]; 
	uint32_t swapchainImageCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
	VK_CHECK( vkGetSwapchainImagesKHR(device, swapchain,&swapchainImageCount,swapchainImages));

	VkImageView swapchainImageViews[16];
	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		swapchainImageViews[i] = creaeteImageView(device, swapchainImages[i], swapchainFormat);
		assert(swapchainImageViews[i]);
	}

	VkFramebuffer swapchainFramebuffers[16];
	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		swapchainFramebuffers[i] = createFrameBuffer(device, renderPass, swapchainImageViews[i], windowWidth, windowHeight);
		assert(swapchainFramebuffers[i]);
	}


	VkCommandPool commandPool = createCommandPool(device, familyIndex);
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




		VkCommandBufferBeginInfo commandBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK( vkBeginCommandBuffer(commandBuffer,&commandBeginInfo));



		// (42,52,57)
		VkClearColorValue color = { 42.0f / 256.0f,52.0f / 256.0f,57.0f / 256.0f };
		VkClearValue clearcolor = { color };

		VkRenderPassBeginInfo renderBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderBeginInfo.renderPass = renderPass;
		renderBeginInfo.framebuffer = swapchainFramebuffers[ImageIndex];
		renderBeginInfo.renderArea.extent.width = windowWidth;
		renderBeginInfo.renderArea.extent.height = windowHeight;
		renderBeginInfo.clearValueCount = 1;
		renderBeginInfo.pClearValues = &clearcolor;


		vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = { 0, float(windowHeight), float(windowWidth), -float(windowHeight), 0, 1 };
		VkRect2D scissor = { {0, 0}, {uint32_t(windowWidth), uint32_t(windowHeight)} };


		vkCmdSetViewport(commandBuffer, 0, 1,&viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// draw calls
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

	
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