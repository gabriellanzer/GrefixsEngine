#ifndef __APP_VULKAN__H__
#define __APP_VULKAN__H__

// StdLib Dependences
#include <vector>

// Third Party Dependencies
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <volk.h>

// Internal Dependencies
#include <core/iapp.h>
#include <render/utilsVulkan.h>

// Using directives
using namespace RenderUtils::Vulkan;

class Vulkan_App : public gefx::IApp
{
	template <typename TData>
	using vector = std::vector<TData>;

  public:
	Vulkan_App() : gefx::IApp("Grefix Vulkan"){};
	~Vulkan_App() override = default;
	Vulkan_App(Vulkan_App&&) = delete;
	Vulkan_App(const Vulkan_App&) = delete;
	Vulkan_App& operator=(Vulkan_App&&) = delete;
	Vulkan_App& operator=(const Vulkan_App&) = delete;

	void Awake() override;
	void Setup() override;
	void Shutdown() override;
	void Sleep() override;
	void Update(double deltaTime) override;

  private:
	const bool USE_VALIDATION_LAYERS = true;
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	static void OnGlfwErrorCallback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	static void OnGlfwWindowResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto* app = reinterpret_cast<Vulkan_App*>(glfwGetWindowUserPointer(window));
		app->_framebufferResized = true;
	}

	void GetRequiredAPIExtensions(vector<const char*>& outExtensions);
	void GetRequiredDeviceExtensions(vector<const char*>& outExtensions);
	bool CheckValidationLayerSupport(const vector<const char*>& validationLayers);

	void CreateInstance();
	void CreateValidationMessageHandler();
	void DestroyValidationMessageHandler();
	void PickPhysicalDevice(const vector<const char*>& deviceExtensions);
	void CreateLogicalDevice(const vector<const char*>& deviceExtensions);
	void CreateSwapChain();
	void CreateSwapChainImageViews();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSyncObjects();

	void CleanupSwapChain();
	void RecreateSwapChain();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	void DrawVulkanFrame(double deltaTime);

	GLFWwindow* _window = {nullptr};

	// ==================
	// Vulkan API Objects
	// ==================
	bool _hasValidationLayers = false;
	VkInstance _instance = {nullptr};
	VkPhysicalDevice _physicalDevice = {nullptr};
	VkDevice _device = {nullptr};

	// =============
	// Queue Objects
	// =============
	QueueFamilyIndices _queueIndices;
	VkQueue _graphicsQueue = {nullptr};
	VkQueue _presentQueue = {nullptr};

	// ===========================
	// Surface and Present Objects
	// ===========================
	SwapChainSupportDetails _swapchainDetails;
	VkSurfaceFormatKHR _surfaceFormat;
	VkExtent2D _surfaceExtent;
	VkSurfaceKHR _surface = {nullptr};
	VkSwapchainKHR _swapChain = {nullptr};
	vector<VkImage> _swapChainImages;
	vector<VkImageView> _swapChainImageViews;
	vector<VkFramebuffer> _swapChainFramebuffers;
	bool _framebufferResized = {false};

	// =================
	// Rendering Objects
	// =================
	VkRenderPass _renderPass;
	VkPipelineLayout _pipelineLayout;
	VkCommandPool _commandPool;
	vector<VkCommandBuffer> _commandBuffers;

	// ======================
	// Syncronization Objects
	// ======================
	vector<VkSemaphore> _imageAvailableSemaphores;
	vector<VkSemaphore> _renderFinishedSemaphores;
	vector<VkFence> _inFlightFences;

	VkDebugUtilsMessengerEXT _debugMessenger = {nullptr};
};

#endif