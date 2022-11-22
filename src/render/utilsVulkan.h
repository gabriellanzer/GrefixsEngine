#ifndef __UTILS_VULKAN__H__
#define __UTILS_VULKAN__H__

// StdLib Dependencies
#include <algorithm>
#include <optional>
#include <stdio.h>
#include <string>
#include <vector>
#include <set>

// Third Library Includes
#include <fmt/core.h>
#include <volk.h>

// Using Directives
using std::string;
template <typename T>
using vector = std::vector<T>;
template <typename T>
using optional = std::optional<T>;
template <typename T>
using set = std::set<T>;

namespace RenderUtils::Vulkan
{
	struct QueueFamilyIndices
	{
		optional<uint32_t> GraphicsFamily;
		optional<uint32_t> PresentFamily;

		bool HasAllFamilies() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities = {};
		vector<VkSurfaceFormatKHR> SurfaceFormats = {};
		vector<VkPresentModeKHR> PresentModes = {};

		bool IsAdequate() { return !SurfaceFormats.empty() && !PresentModes.empty(); }
	};

	inline QueueFamilyIndices GetQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.GraphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.PresentFamily = i;
			}

			// We found the queue indices that for all required queue bits
			if (indices.HasAllFamilies()) break;
		}

		// Assign index to queue families that could be found
		return indices;
	}

	inline bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const vector<const char*>& requiredExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		set<string> filteredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			filteredExtensionsSet.erase(extension.extensionName);
		}

		return filteredExtensionsSet.empty();
	}

	inline SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.SurfaceCapabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.SurfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.SurfaceFormats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

	inline void ChoseSwapchainAttributes(const SwapChainSupportDetails& supportDetails, const VkExtent2D& currentExtent,
										 VkSurfaceFormatKHR& outSurfaceFormat, VkPresentModeKHR& outPresentMode,
										 VkExtent2D& outPresentExtent)
	{
		// Chose surface formats
		uint32_t formatId = 0;
		const vector<VkSurfaceFormatKHR>& formats = supportDetails.SurfaceFormats;
		for (uint32_t id = 0; id < formats.size(); ++id)
		{
			const auto& availableFormat = formats[id];
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				formatId = id;
				break;
			}
		}
		outSurfaceFormat = formats[formatId];

		// We stick with immediate due to lower latency
		// TODO: Implement different swapchain modes
		outPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

		// Chose present framebuffer extent
		const VkSurfaceCapabilitiesKHR& capability = supportDetails.SurfaceCapabilities;
		if (capability.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			outPresentExtent = capability.currentExtent;
		}
		else
		{
			outPresentExtent = {};
			outPresentExtent.width = std::clamp(
				currentExtent.width, capability.minImageExtent.width, capability.maxImageExtent.width);
			outPresentExtent.height = std::clamp(
				currentExtent.height, capability.minImageExtent.height, capability.maxImageExtent.height);
		}
	}

	inline VkShaderModule CreateShaderModule(const VkDevice& device, const vector<unsigned int>& codeData)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = codeData.size() * sizeof(unsigned int);
		createInfo.pCode = reinterpret_cast<const uint32_t*>(codeData.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			fmt::print("Failed to create shader module!");
			fflush(stdout);
		}

		return shaderModule;
	}

	inline VkPipeline CreateDefaultRenderPipeline(const VkDevice& device, const VkRenderPass& renderPass,
												  const VkPipelineLayout& pipelineLayout, VkExtent2D surfaceExtent,
												  const VkShaderModule& vertModule, const VkShaderModule& fragModule)
	{
		vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)surfaceExtent.width;
		viewport.height = (float)surfaceExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = surfaceExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f;		   // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f;	   // Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;			// Optional
		multisampling.pSampleMask = nullptr;			// Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE;		// Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
											  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkPipelineShaderStageCreateInfo vertexStage = {};
		vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStage.module = vertModule;
		vertexStage.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentStage = {};
		fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentStage.module = fragModule;
		fragmentStage.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		VkPipeline graphicsPipeline;
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			fmt::print("Failed to create graphics pipeline!");
			fflush(stdout);
		}

		return graphicsPipeline;
	}

} // namespace RenderUtils::Vulkan

#endif