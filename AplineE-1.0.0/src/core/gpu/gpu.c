#include "gpu.h"
#include <stdio.h>
#include <stdlib.h>
#include "../device/device.h"
#include <string.h>
#include "../swapchain/swapchain.h"
#include <util/util.h>

VkBool32 checkDeviceExtensionSupport(VkPhysicalDevice device, EngineSettings settings) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	VkExtensionProperties* availableExtensions = (VkExtensionProperties*)fsAllocate(sizeof(VkExtensionProperties) * extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions);

	uint32_t requiredExtensionCount = 0;
	char** requiredExtensions = { 0 };
	getRequiredDeviceExtensions(&requiredExtensionCount, &requiredExtensions, settings);

	for (uint32_t i = 0; i < extensionCount; i++) {
		for (uint32_t j = 0; j < requiredExtensionCount; j++) {
			if (!requiredExtensions[j]) {
				continue;
			}
			if (strcmp(availableExtensions[i].extensionName, requiredExtensions[j])) {
				requiredExtensions[j] = nullptr;
			}
		}
	}
	uint16_t remainingExtensions = 0;
	for (uint32_t i = 0; i < requiredExtensionCount; i++) {
		if (requiredExtensions[i]) {
			remainingExtensions++;
		}
	}
	fsFree(requiredExtensions);
	uint32_t supported = remainingExtensions == 0;
	fsFree(availableExtensions);
	return supported;
}

int isDeviceSuitable(VkPhysicalDevice device, EngineSettings settings, EngineHandle handle) {

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


	//check type
	VkPhysicalDeviceType type = deviceProperties.deviceType;
	switch (settings.gpuSelectPreference.type) {
	case GPU_TYPE_SELECT_REQUIRE_DEDICATED:
		if (type != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return 0;
		}
		break;
	case GPU_TYPE_SELECT_REQUIRE_CPU:
		if (type != VK_PHYSICAL_DEVICE_TYPE_CPU) {
			return 0;
		}
		break;
	default:
		break;
	}

	//check required features
	//TODO: add check for required features

	//check device extensions
	if (!checkDeviceExtensionSupport(device, settings)) {
		return 0;
	}

	//check queue families
	if (!queueFamilyIndicesComplete(findQueueFamilies(device, handle))) {
		return 0;
	}

	//check swapchain supported
	if (!swapChainCheckSupport(device, handle)) {
		return 0;
	}

	return 1;
}

int evaluateDevice(VkPhysicalDevice device, EngineSettings settings) {
	int score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	//check type
	VkPhysicalDeviceType type = deviceProperties.deviceType;
	switch (settings.gpuSelectPreference.type) {
	case GPU_TYPE_SELECT_PREFER_DEDICATED:
		if (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += settings.gpuSelectPreference.scalers.isDedicatedScore;
		}
		break;
	case GPU_TYPE_SELECT_PREFER_CPU:
		if (type == VK_PHYSICAL_DEVICE_TYPE_CPU) {
			score += settings.gpuSelectPreference.scalers.isCpuScore;
		}
		break;
	default:
		break;
	}

	//evaluate features
	score += (int32_t)(deviceProperties.limits.maxImageDimension1D * settings.gpuSelectPreference.scalers.maxImageDimensions1Dscaler);
	score += (int32_t)(deviceProperties.limits.maxImageDimension2D * settings.gpuSelectPreference.scalers.maxImageDimensions2Dscaler);
	score += (int32_t)(deviceProperties.limits.maxImageDimension3D * settings.gpuSelectPreference.scalers.maxImageDimensions3Dscaler);
	score += (int32_t)(deviceProperties.limits.maxImageDimensionCube * settings.gpuSelectPreference.scalers.maxImageDimensionsCubescaler);

	return score;
}

GpuPropertyScoreScale gpuSelectPreferencesScalersGetDefault() {
	GpuPropertyScoreScale scaler;
	scaler.isDedicatedScore = 10000;
	scaler.isCpuScore = 5000;
	scaler.maxImageDimensions1Dscaler = 0.1f;
	scaler.maxImageDimensions2Dscaler = 2.0f;
	scaler.maxImageDimensions3Dscaler = 0.7f;
	scaler.maxImageDimensionsCubescaler = 0.2f;
	return scaler;
}

void gpuSelectSuitable(EngineHandle handle, EngineSettings settings) {
	//retrieve available gpus
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(INSTANCE(handle), &deviceCount, NULL);
	if (deviceCount == 0) {
		printf("Failed to find GPUs with Vulkan support!\n");
		abort();
	}
	VkPhysicalDevice* devices = (VkPhysicalDevice*)fsAllocate(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(INSTANCE(handle), &deviceCount, devices);

	int highestScore = -1;
	VkPhysicalDevice highestEvaluatedDevice = NULL;
	for (uint32_t i = 0; i < deviceCount; i++) {
		if (!isDeviceSuitable(devices[i], settings, handle)) {
			continue;
		}
		int score = evaluateDevice(devices[i], settings);
		if (score > highestScore) {
			highestEvaluatedDevice = devices[i];
			highestScore = score;
		}
	}
	if (highestScore <= 0 || highestEvaluatedDevice == NULL) {
		printf("Failed to find suitable GPU for this application\n");
		abort();
	}

	PHYSICAL_DEVICE(handle) = highestEvaluatedDevice;
	fsFree(devices);
}

VkBool32 queueFamilyIndicesComplete(QueueFamilyIndices indices) {
	return
		indices.graphicsFamily.present *
		indices.presentFamily.present;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, EngineHandle handle) {
	QueueFamilyIndices indices = { 0 };

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)fsAllocate(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	uint32_t index = 0;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		VkQueueFamilyProperties queueFamily = queueFamilies[index];

		VkBool32 presentSupport = { 0 };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, SURFACE(handle), &presentSupport);

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily.index = index;
			indices.graphicsFamily.present = VK_TRUE;
		}

		if (presentSupport) {
			indices.presentFamily.index = index;
			indices.presentFamily.present = VK_TRUE;
		}

		if (queueFamilyIndicesComplete(indices)) {
			break;
		}

		index++;
	}

	fsFree(queueFamilies);
	return indices;
}

VkPhysicalDeviceFeatures getSelectedDeviceFeatures(EngineSettings settings) {
	VkPhysicalDeviceFeatures features = { 0 };
	return features;
}

VkFormat selectColorImageFormat(EngineHandle handle) {
	return VK_FORMAT_B8G8R8A8_SRGB;
}

VkFormat selectDepthImageFormat(EngineHandle handle) {
	VkFormat depthFormats[] = {
		VK_FORMAT_D32_SFLOAT, 
		VK_FORMAT_D32_SFLOAT_S8_UINT, 
		VK_FORMAT_D24_UNORM_S8_UINT
	};
	return findSupportedFormat(
		handle,
		sizeof(depthFormats)/sizeof(VkFormat),
		depthFormats,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void gpuUpdateSystemCapabilities(EngineHandle handle) {
	SwapChainSupportDetails swapchainSupport = querySwapChainSupport(PHYSICAL_DEVICE(handle), handle);
	for (uint32_t i = 0; i < swapchainSupport.presentModeCount; i++) {
		switch (swapchainSupport.presentModes[i]) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			handle->system.vsyncCapabilities |= VSYNC_CAPABILITIES_NOT_AVAILABLE;
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			handle->system.vsyncCapabilities |= VSYNC_CAPABILITIES_DOUBLE_BUFFERED;
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			handle->system.vsyncCapabilities |= VSYNC_CAPABILITIES_LAZY_DOUBLE_BUFFERED;
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			handle->system.vsyncCapabilities |= VSYNC_CAPABILITIES_TRIPPLE_BUFFERED;
		}
	}

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(PHYSICAL_DEVICE(handle), &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(PHYSICAL_DEVICE(handle), &deviceFeatures);

	handle->system.maxImageWidth = deviceProperties.limits.maxImageDimension2D;
	handle->system.maxImageHeight = deviceProperties.limits.maxImageDimension2D;
	handle->system.gpuName = deviceProperties.deviceName;

	handle->system.colorImageFormat = selectColorImageFormat(handle);
	handle->system.depthImageFormat = selectDepthImageFormat(handle);
	handle->system.depthImageFormatHasStencilComponent = hasStencilComponent(handle->system.depthImageFormat);
	freeSwapchainSupportDetails(swapchainSupport);
}


