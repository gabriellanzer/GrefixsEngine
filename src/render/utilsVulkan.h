#ifndef __UTILS_VULKAN__H__
#define __UTILS_VULKAN__H__

// StdLib Dependencies
#include <algorithm>
#include <optional>
#include <string>
#include <vector>
#include <set>

// Thrid Library Includes
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

} // namespace RenderUtils::Vulkan

#endif