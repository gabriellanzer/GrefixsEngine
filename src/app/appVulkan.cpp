
// StdLib Includes
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <optional>
#include <set>

// Third Party Includes
#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define VOLK_IMPLEMENTATION
#include <volk.h>
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
#include <fmt/core.h>
#include <PerlinNoise.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Application Specific Includes
#include <app/appVulkan.h>
#include <render/utilsCore.h>
#include <render/utilsVulkan.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;
template <typename T>
using optional = std::optional<T>;
template <typename T>
using set = std::set<T>;

void Vulkan_App::Setup()
{
	if (volkInitialize() != VK_SUCCESS)
	{
		throw std::runtime_error("Could not Initialize Vulkan Loader!");
	}

	// Setup Graphics APIs
	glfwSetErrorCallback(OnGlfwErrorCallback);
	if (!glfwInit())
	{
		throw std::runtime_error("GLFW initialization error!");
	}

	if (!glfwVulkanSupported())
	{
		throw std::runtime_error("Vulkan not available for GLFW surface!");
	}

	// Ensures GLFW doesn't initialize OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window = glfwCreateWindow(1920, 1080, GetName(), nullptr, nullptr);
	if (!_window)
	{
		glfwTerminate();
		return;
	}

	glfwSetInputMode(_window, GLFW_STICKY_KEYS, GLFW_TRUE);

	// Operating System Window Settings
	glfwSetWindowSizeLimits(_window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwSetWindowUserPointer(_window, this);
	glfwSetWindowSizeCallback(_window, OnGlfwWindowResizeCallback);
	glfwMaximizeWindow(_window);

	// ========================
	// VULKAN SETUP BLOCK START
	// ========================
	CreateInstance();
	CreateValidationMessageHandler();

	if (glfwCreateWindowSurface(_instance, _window, NULL, &_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Window Surface!");
	}

	vector<const char*> requiredExtensions;
	GetRequiredDeviceExtensions(requiredExtensions);
	PickPhysicalDevice(requiredExtensions);
	CreateLogicalDevice(requiredExtensions);
	CreateSwapChain();
	CreateSwapChainImageViews();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateSyncObjects();

	// ======================
	// VULKAN SETUP BLOCK END
	// ======================

	RenderUtils::InitGlslang();

	string vertData, fragData;
	vector<unsigned int> vertSpirv, fragSpirv;
	if (RenderUtils::TryLoadShaderFile("../../shaders/vert_col_vk.vs", vertData) &&
		RenderUtils::TryLoadShaderFile("../../shaders/vert_col_vk.fs", fragData))
	{
		RenderUtils::GLSLtoSPV(
			VK_SHADER_STAGE_VERTEX_BIT, vertData.c_str(), vertSpirv, RenderUtils::ShaderCompileTarget::Vulkan);
		RenderUtils::GLSLtoSPV(
			VK_SHADER_STAGE_FRAGMENT_BIT, fragData.c_str(), fragSpirv, RenderUtils::ShaderCompileTarget::Vulkan);
		VkShaderModule vertModule = RenderUtils::Vulkan::CreateShaderModule(_device, vertSpirv);
		VkShaderModule fragModule = RenderUtils::Vulkan::CreateShaderModule(_device, fragSpirv);

		CreateGraphicsPipeline(vertModule, fragModule);

		vkDestroyShaderModule(_device, vertModule, nullptr);
		vkDestroyShaderModule(_device, fragModule, nullptr);
	}

	fmt::print("Vulkan App Setup Finished");
}

void Vulkan_App::Awake() {}

void Vulkan_App::Sleep() {}

void Vulkan_App::Shutdown()
{
	fmt::print("Vulkan App Setup Shutdown");

	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);

	CleanupSwapChain();

	for (uint32_t frameIt = 0; frameIt < MAX_FRAMES_IN_FLIGHT; ++frameIt)
	{
		vkDestroySemaphore(_device, _imageAvailableSemaphores[frameIt], nullptr);
		vkDestroySemaphore(_device, _renderFinishedSemaphores[frameIt], nullptr);
		vkDestroyFence(_device, _inFlightFences[frameIt], nullptr);
	}

	vkDestroyRenderPass(_device, _renderPass, nullptr);
	vkDestroyCommandPool(_device, _commandPool, nullptr);

	vkDestroyDevice(_device, nullptr);
	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	DestroyValidationMessageHandler();
	vkDestroyInstance(_instance, nullptr);

	RenderUtils::FinalizeGlslang();

	// Graphics API shutdown
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void Vulkan_App::Update(double deltaTime)
{
	if (glfwWindowShouldClose(_window))
	{
		_shouldQuit = true;

		// Wait for device to become idle before shutting down
		// this is required because of resource cleaning
		vkDeviceWaitIdle(_device);
		return;
	}

	// Poll first so ImGUI has the events.
	// This performs some callbacks as well
	glfwPollEvents();

	DrawVulkanFrame(deltaTime);
}

void Vulkan_App::GetRequiredAPIExtensions(vector<const char*>& outExtensions)
{
	outExtensions.clear();

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	outExtensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (USE_VALIDATION_LAYERS)
	{
		outExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
}

void Vulkan_App::GetRequiredDeviceExtensions(vector<const char*>& outExtensions)
{
	outExtensions.clear();

	outExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

bool Vulkan_App::CheckValidationLayerSupport(const vector<const char*>& validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}
	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															  VkDebugUtilsMessageTypeFlagsEXT messageType,
															  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															  void* pUserData)
{

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		// We don't want to show verbose info here
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		fmt::print(stdout, "Validation Layer Info: {0}\n", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		fmt::print(stdout, "Validation Layer Warning: {0}\n", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		fmt::print(stderr, "Validation Layer Error: {0}\n", pCallbackData->pMessage);
		break;
	default:
		break;
	}

	return VK_FALSE;
}

void Vulkan_App::CreateInstance()
{
	// Query for available Vulkan extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	fmt::print("Available Vulkan Extensions:\n");
	for (const VkExtensionProperties& extension : extensions)
	{
		fmt::print("\t{0}\n", extension.extensionName);
	}
	fflush(stdout);

	// Required Extensions
	vector<const char*> requiredExtensions;
	GetRequiredAPIExtensions(requiredExtensions);
	fmt::print("Required Extensions:\n");
	for (uint32_t extIt = 0; extIt < requiredExtensions.size(); ++extIt)
	{
		fmt::print("\t{0}\n", requiredExtensions[extIt]);
	}
	fflush(stdout);

	// Create Vulkan Application and Instance
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Application";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Grefix Endine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	createInfo.enabledLayerCount = 0;

	// Check Validation Layer Support
	VkDebugUtilsMessengerCreateInfoEXT createMsgHandlerInfo = {};
	vector<const char*> validationLayers;
	if (USE_VALIDATION_LAYERS)
	{
		// Check if we there is validation layer support
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		if (CheckValidationLayerSupport(validationLayers))
		{
			createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();

			// Create message logger for VkCreateInstance
			createMsgHandlerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createMsgHandlerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createMsgHandlerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
											   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
											   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createMsgHandlerInfo.pfnUserCallback = ValidationLayerCallback;
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&createMsgHandlerInfo;
			_hasValidationLayers = true;
		}
		else
		{
			fmt::print(stderr, "Could not enable Vulkan validation layers! Program will keep-on without them...");
			fflush(stderr);
		}
	}

	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}

	volkLoadInstance(_instance);
}

void Vulkan_App::CreateValidationMessageHandler()
{
	if (!USE_VALIDATION_LAYERS || vkCreateDebugUtilsMessengerEXT == nullptr) return;

	// Create Message Callback Handler
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = ValidationLayerCallback;
	createInfo.pUserData = nullptr;

	if (vkCreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
	{
		fmt::print(stderr, "Could not create Validation Layer Message Handler!"
						   "The application won't have validation logs...");
	}
}

void Vulkan_App::DestroyValidationMessageHandler()
{
	if (!USE_VALIDATION_LAYERS || vkDestroyDebugUtilsMessengerEXT == nullptr) return;

	vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
}

void Vulkan_App::PickPhysicalDevice(const vector<const char*>& deviceExtensions)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		// Device is suitable if it has all our required family queues
		_queueIndices = GetQueueIndices(device, _surface);
		if (!_queueIndices.HasAllFamilies()) continue;

		// If it has all required extensions
		if (!CheckDeviceExtensionSupport(device, deviceExtensions)) continue;

		// And if it supports our swapchain surface requirements
		_swapchainDetails = QuerySwapChainSupport(device, _surface);
		if (!_swapchainDetails.IsAdequate()) continue;

		// Got our device!
		_physicalDevice = device;
		break;
	}

	// Check if the best candidate is suitable at all
	if (_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

void Vulkan_App::CreateLogicalDevice(const vector<const char*>& deviceExtensions)
{
	QueueFamilyIndices indices = GetQueueIndices(_physicalDevice, _surface);

	// Create graphics and present queues from queue family indices
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();

	vector<const char*> validationLayers;
	// Info: This is mostly for compatibility with older Vulkan APIs
	// Check if we there is validation layer support
	if (USE_VALIDATION_LAYERS && _hasValidationLayers)
	{
		validationLayers.push_back("VK_LAYER_KHRONOS_validation");
		createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	// Fetch operating queues
	vkGetDeviceQueue(_device, indices.GraphicsFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, indices.PresentFamily.value(), 0, &_presentQueue);
}

void Vulkan_App::CreateSwapChain()
{
	int width, height;
	glfwGetFramebufferSize(_window, &width, &height);

	// We query swap chain support again, because the surface might have changed
	// Aka.: window has resized and we are recreating the swapchain
	_swapchainDetails = QuerySwapChainSupport(_physicalDevice, _surface);

	// Fetch swapchain attributes
	VkPresentModeKHR presentMode;
	ChoseSwapchainAttributes(
		_swapchainDetails, {(uint32_t)width, (uint32_t)height}, _surfaceFormat, presentMode, _surfaceExtent);

	// Pick number of images
	uint32_t imageCount = _swapchainDetails.SurfaceCapabilities.minImageCount + 1;
	if (_swapchainDetails.SurfaceCapabilities.maxImageCount > 0 &&
		imageCount > _swapchainDetails.SurfaceCapabilities.maxImageCount)
	{
		imageCount = _swapchainDetails.SurfaceCapabilities.maxImageCount;
	}

	// Create swapchain object
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = _surfaceFormat.format;
	createInfo.imageColorSpace = _surfaceFormat.colorSpace;
	createInfo.imageExtent = _surfaceExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {_queueIndices.GraphicsFamily.value(), _queueIndices.PresentFamily.value()};
	if (_queueIndices.GraphicsFamily != _queueIndices.PresentFamily)
	{
		// Concurrent sharing mode requires sync between queues to
		// transfer swapchain image ownership. This is slower...
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		// Exclusive swapchain sharing mode is optimal and preferred
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	// No transform here, but we could flip the images or rotate 90 degrees.
	// The valida transformations are in SurfaceCapabilities.supportedTransforms.
	createInfo.preTransform = _swapchainDetails.SurfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	// TODO: look into that for swapchain rebuilding (for window resizes)
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	// Hold swapchaing VkImages handlers
	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
	_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());
}

void Vulkan_App::CreateSwapChainImageViews()
{
	_swapChainImageViews.resize(_swapChainImages.size());
	for (size_t i = 0; i < _swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _surfaceFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

void Vulkan_App::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = _surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Render Pass!");
	}
}

void Vulkan_App::CreateFramebuffers()
{
	_swapChainFramebuffers.resize(_swapChainImageViews.size());
	for (size_t i = 0; i < _swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {_swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = _surfaceExtent.width;
		framebufferInfo.height = _surfaceExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Framebuffer!");
		}
	}
}

void Vulkan_App::CreateGraphicsPipeline(VkShaderModule vertModule, VkShaderModule fragModule)
{

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			  // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;		  // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	  // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
	{
		fmt::print("Failed to create pipeline layout!");
		return;
	}

	_graphicsPipeline = RenderUtils::Vulkan::CreateDefaultRenderPipeline(
		_device, _renderPass, _pipelineLayout, _surfaceExtent, vertModule, fragModule);
}

void Vulkan_App::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = _queueIndices.GraphicsFamily.value();

	if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}
}

void Vulkan_App::CreateCommandBuffers()
{
	_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

	if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void Vulkan_App::CreateSyncObjects()
{
	_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
		{

			throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}
	}
}

void Vulkan_App::CleanupSwapChain()
{
	for (auto& frameBuf : _swapChainFramebuffers)
	{
		vkDestroyFramebuffer(_device, frameBuf, nullptr);
	}

	for (auto& imageViews : _swapChainImageViews)
	{
		vkDestroyImageView(_device, imageViews, nullptr);
	}

	vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void Vulkan_App::RecreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(_window, &width, &height);

	// Check for window minimization
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(_window, &width, &height);

		// Keep pooling events
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(_device);

	CleanupSwapChain();

	CreateSwapChain();
	CreateSwapChainImageViews();
	CreateFramebuffers();
}

void Vulkan_App::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;				  // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = _surfaceExtent;

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.4f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_surfaceExtent.width);
	viewport.height = static_cast<float>(_surfaceExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = _surfaceExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void Vulkan_App::DrawVulkanFrame(double deltaTime)
{
	static uint32_t frameIndex = 0;

	// Wait for the next frame fence to be available for us
	vkWaitForFences(_device, 1, &_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);

	// Feth image index from the swapchain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// If swapchain is recreated, there is no work to be submitted, we must only reset
	// the fence if we are pushing work to the GPU, otherwise we will cause a deadlock.
	vkResetFences(_device, 1, &_inFlightFences[frameIndex]);

	// Reset and record command buffer instructions
	vkResetCommandBuffer(_commandBuffers[frameIndex], 0);
	RecordCommandBuffer(_commandBuffers[frameIndex], imageIndex);

	// Submit command buffer instructions to GPU
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[frameIndex]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[frameIndex];
	VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[frameIndex]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[frameIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {_swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized)
	{
		_framebufferResized = false;
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}