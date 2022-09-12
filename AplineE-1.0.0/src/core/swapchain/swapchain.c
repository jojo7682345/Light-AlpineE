#include "swapchain.h"
#include "../../util/math.h"
#include <stdlib.h>
#include "../gpu/gpu.h"
#include "../../util/util.h"



SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, EngineHandle handle) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, SURFACE(handle), &details.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, SURFACE(handle), &details.formatCount, nullptr);

	if (details.formatCount != 0) {

		details.formats = (VkSurfaceFormatKHR*)fsAllocate(sizeof(VkSurfaceFormatKHR) * details.formatCount);

		vkGetPhysicalDeviceSurfaceFormatsKHR(device, SURFACE(handle), &details.formatCount, details.formats);
	}


	vkGetPhysicalDeviceSurfacePresentModesKHR(device, SURFACE(handle), &details.presentModeCount, nullptr);

	if (details.presentModeCount != 0) {

		details.presentModes = (VkPresentModeKHR*)fsAllocate(sizeof(VkPresentModeKHR) * details.presentModeCount);

		vkGetPhysicalDeviceSurfacePresentModesKHR(device, SURFACE(handle), &details.presentModeCount, details.presentModes);
	}

	return details;
}

void freeSwapchainSupportDetails(SwapChainSupportDetails details) {
	if (details.formats != nullptr) {
		details.presentModeCount = 0;
		fsFree(details.formats);
	}
	if (details.presentModes != nullptr) {
		details.presentModeCount = 0;
		fsFree(details.presentModes);
	}
}

VkBool32 swapChainCheckSupport(VkPhysicalDevice device, EngineHandle handle) {

	SwapChainSupportDetails details = querySwapChainSupport(device, handle);

	// if there are no formats or present modes, then the device does not support swapchain
	VkBool32 swapchainSupported = details.formatCount != 0 && details.presentModeCount != 0;

	freeSwapchainSupportDetails(details);

	return swapchainSupported;

}

VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const uint32_t availableFormatCount, VkSurfaceFormatKHR* availableFormats, EngineSettings settings) {

	for (uint32_t i = 0; i < availableFormatCount; i++) {
		if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormats[i];
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR chooseSwapchainPresentMode(const uint32_t availablePresentModeCount, VkPresentModeKHR* availablePresentModes, EngineSettings settings) {

	for (uint32_t i = 0; i < availablePresentModeCount; i++) {
		if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentModes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooswSwapchainExtent(const VkSurfaceCapabilitiesKHR* capabilities, EngineSettings settings, EngineHandle handle) {

	if (capabilities->currentExtent.width != (uint32_t)-1) {
		return capabilities->currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(WINDOW(handle), &width, &height);

		VkExtent2D actualExtent = {
			(uint32_t)width,
			(uint32_t)height
		};

		actualExtent.width = clampUI32(actualExtent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
		actualExtent.height = clampUI32(actualExtent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

		return actualExtent;
	}
}

void selectSwapchainProperties(EngineHandle handle, EngineSettings settings) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(PHYSICAL_DEVICE(handle), handle);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapChainSupport.formatCount, swapChainSupport.formats, settings);
	VkExtent2D extent = chooswSwapchainExtent(&swapChainSupport.capabilities, settings, handle);


	SWAPCHAIN_IMAGE_FORMAT(handle) = surfaceFormat.format;
	SWAPCHAIN_EXTENT(handle) = extent;

}

void createSwapchainImageViews(EngineHandle handle, EngineSettings settigns) {
	const uint32_t imageCount = SWAPCHAIN_IMAGE_COUNT(handle);
	SWAPCHAIN_IMAGE_VIEWS(handle) = (VkImageView*)fsAllocate(sizeof(VkImageView) * imageCount);
	for (uint32_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = SWAPCHAIN_IMAGES(handle)[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = SWAPCHAIN_IMAGE_FORMAT(handle);
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkCheck(vkCreateImageView(DEVICE(handle), &createInfo, nullptr, &SWAPCHAIN_IMAGE_VIEWS(handle)[i]));
	}

}

void swapchainCreate(EngineHandle handle, EngineSettings settings) {


	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(PHYSICAL_DEVICE(handle), handle);

	selectSwapchainProperties(handle, settings);

	VkPresentModeKHR presentMode = chooseSwapchainPresentMode(swapChainSupport.presentModeCount, swapChainSupport.presentModes, settings);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapChainSupport.formatCount, swapChainSupport.formats, settings);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = SURFACE(handle);

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = SWAPCHAIN_IMAGE_FORMAT(handle);
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = SWAPCHAIN_EXTENT(handle);
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(PHYSICAL_DEVICE(handle), handle);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.index, indices.presentFamily.index };

	if (indices.graphicsFamily.index != indices.presentFamily.index) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkCheck(vkCreateSwapchainKHR(DEVICE(handle), &createInfo, nullptr, &SWAPCHAIN(handle)));

	vkGetSwapchainImagesKHR(DEVICE(handle), SWAPCHAIN(handle), &SWAPCHAIN_IMAGE_COUNT(handle), nullptr);
	SWAPCHAIN_IMAGES(handle) = (VkImage*)fsAllocate(sizeof(VkImage) * SWAPCHAIN_IMAGE_COUNT(handle));
	vkGetSwapchainImagesKHR(DEVICE(handle), SWAPCHAIN(handle), &SWAPCHAIN_IMAGE_COUNT(handle), SWAPCHAIN_IMAGES(handle));

	createSwapchainImageViews(handle, settings);

}

void swapchainRetrieveDesiredSettings(EngineHandle handle, SwapchainCreateInfo* createInfo) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(PHYSICAL_DEVICE(handle), handle);
	
	//get image count
	createInfo->imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && createInfo->imageCount > swapChainSupport.capabilities.maxImageCount) {
		createInfo->imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//get image format and colorspace
	if (swapChainSupport.formatCount > 0) {
		createInfo->format = swapChainSupport.formats[0].format;
		createInfo->colorSpace = swapChainSupport.formats[0].colorSpace;
	}
	for (uint32_t i = 0; i < swapChainSupport.formatCount; i++) {
		if (swapChainSupport.formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && swapChainSupport.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			createInfo->format = swapChainSupport.formats[i].format;
			createInfo->colorSpace = swapChainSupport.formats[i].colorSpace;
		}
	}

	//get image extent
	if (swapChainSupport.capabilities.currentExtent.width != (uint32_t)-1) {
		createInfo->extent =  swapChainSupport.capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(WINDOW(handle), &width, &height);

		VkExtent2D actualExtent = {
			(uint32_t)width,
			(uint32_t)height
		};

		actualExtent.width = clampUI32(actualExtent.width, swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width);
		actualExtent.height = clampUI32(actualExtent.height, swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height);

		createInfo->extent = actualExtent;
	}
}




void swapchainnDestroyRenderDisplay(EngineHandle handle, VkSwapchainKHR swapchain) {
	vkDestroySwapchainKHR(DEVICE(handle), swapchain, nullptr);
}

AeResult swapchainCreateRenderDisplay(EngineHandle handle, SwapchainCreateInfo info, VkSwapchainKHR* swapchain) {

	VkSwapchainCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = SURFACE(handle);
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.imageArrayLayers = 1;
	createInfo.clipped = VK_TRUE;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	uint32_t queueFamilyIndices[] = { ENGINE_HANDLE_GET(handle,graphicsQueue.index), ENGINE_HANDLE_GET(handle,presentQueue.index) };
	if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.minImageCount = info.imageCount;
	createInfo.imageFormat = info.format;
	createInfo.imageExtent = info.extent;
	createInfo.imageColorSpace = info.colorSpace;
	createInfo.oldSwapchain = info.oldSwapchain;
	createInfo.presentMode = info.presentMode;

	VkCheck(vkCreateSwapchainKHR(DEVICE(handle), &createInfo, nullptr, swapchain));

	return AE_SUCCESS;
}



void destroySwapchainImageViews(EngineHandle handle) {
	const uint32_t imageCount = SWAPCHAIN_IMAGE_COUNT(handle);
	for (uint32_t i = 0; i < imageCount; i++) {
		vkDestroyImageView(DEVICE(handle), SWAPCHAIN_IMAGE_VIEWS(handle)[i], nullptr);
	}
	fsFree(SWAPCHAIN_IMAGE_VIEWS(handle));
}

void swapchainDestroy(EngineHandle handle) {
	destroySwapchainImageViews(handle);
	vkDestroySwapchainKHR(DEVICE(handle), SWAPCHAIN(handle), nullptr);
	fsFree(SWAPCHAIN_IMAGES(handle));
}



bool_t vsyncOptionAvailable(EngineHandle handle, VsyncEnabled vsync) {
	if (vsync == VSYNC_DISABLED) {
		return VK_TRUE;
	}
	if (vsync == VSYNC_DOUBLE_BUFFERED) {
		return (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_DOUBLE_BUFFERED);
	}
	if (vsync == VSYNC_TRIPPLE_BUFFERED) {
		return (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_TRIPPLE_BUFFERED);
	}
	if (vsync == VSYNC_PREFER_DOUBLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE) {
		return (handle->system.vsyncCapabilities & (VSYNC_CAPABILITIES_DOUBLE_BUFFERED | VSYNC_CAPABILITIES_NOT_AVAILABLE));
	}
	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DOUBLE_BUFFERED) {
		return (handle->system.vsyncCapabilities & (VSYNC_CAPABILITIES_TRIPPLE_BUFFERED | VSYNC_CAPABILITIES_DOUBLE_BUFFERED));
	}
	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_PREFER_DOUBLE_BUFFERED_IF_BOTH_NOT_AVAILABLE_DISABLE) {
		return (handle->system.vsyncCapabilities & (VSYNC_CAPABILITIES_TRIPPLE_BUFFERED | VSYNC_CAPABILITIES_DOUBLE_BUFFERED | VSYNC_CAPABILITIES_NOT_AVAILABLE));
	}
	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE) {
		return (handle->system.vsyncCapabilities & (VSYNC_CAPABILITIES_TRIPPLE_BUFFERED | VSYNC_CAPABILITIES_NOT_AVAILABLE));
	}
	return VK_FALSE;
}

VkPresentModeKHR getVsyncPresentMode(VsyncEnabled vsync) {

	switch (vsync) {
	case VSYNC_DISABLED:
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	case VSYNC_DOUBLE_BUFFERED:
		return VK_PRESENT_MODE_FIFO_KHR;
	case VSYNC_TRIPPLE_BUFFERED:
		return VK_PRESENT_MODE_MAILBOX_KHR;
	default:
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

}

bool_t updateSwapchainVsyncSettings(EngineHandle handle, VsyncEnabled vsync) {
	//check for vsync capable;
	if (!vsyncOptionAvailable(handle, vsync)) {
		return VK_FALSE;
	}
	if (vsync == VSYNC_PREFER_DOUBLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE) {
		if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_DOUBLE_BUFFERED) {
			vsync = VSYNC_DOUBLE_BUFFERED;
		} else {
			vsync = VSYNC_DISABLED;
		}
	}

	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE) {
		if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_TRIPPLE_BUFFERED) {
			vsync = VSYNC_TRIPPLE_BUFFERED;
		} else {
			vsync = VSYNC_DISABLED;
		}
	}
	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DOUBLE_BUFFERED) {
		if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_TRIPPLE_BUFFERED) {
			vsync = VSYNC_TRIPPLE_BUFFERED;
		} else if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_DOUBLE_BUFFERED) {
			vsync = VSYNC_DOUBLE_BUFFERED;
		}
	}

	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_PREFER_DOUBLE_BUFFERED_IF_BOTH_NOT_AVAILABLE_DISABLE) {
		if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_TRIPPLE_BUFFERED) {
			vsync = VSYNC_TRIPPLE_BUFFERED;
		} else if (handle->system.vsyncCapabilities & VSYNC_CAPABILITIES_DOUBLE_BUFFERED) {
			vsync = VSYNC_DOUBLE_BUFFERED;
		} else {
			vsync = VSYNC_DISABLED;
		}
	}

	handle->system.vsyncEnabled = vsync;

	return VK_TRUE;
}

bool_t swapchainValidateSettings(EngineHandle handle, EngineSettings settings) {

	VsyncEnabled vsync = settings.vsyncEnabled;
	VsyncSettings vsyncSettings = settings.vsyncPreferences;

	if (vsync == VSYNC_DOUBLE_BUFFERED && (vsyncSettings == VSYNC_SETTINGS_REQUIRE_TRIPLE_BUFFERED || vsyncSettings == VSYNC_SETTINGS_DISABLED)) {
		return VK_FALSE;
	}
	if (vsync == VSYNC_TRIPPLE_BUFFERED && (vsyncSettings == VSYNC_SETTINGS_REQUIRE_DUAL_BUFFERED || vsyncSettings == VSYNC_SETTINGS_DISABLED)) {
		return VK_FALSE;
	}
	if (vsync == VSYNC_PREFER_DOUBLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE && (vsyncSettings == VSYNC_SETTINGS_REQUIRE_TRIPLE_BUFFERED)) {
		return VK_FALSE;
	}
	if (vsync == VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DOUBLE_BUFFERED && (vsyncSettings == VSYNC_SETTINGS_DISABLED)) {
		return VK_FALSE;
	}
	
	return updateSwapchainVsyncSettings(handle, vsync);
}
