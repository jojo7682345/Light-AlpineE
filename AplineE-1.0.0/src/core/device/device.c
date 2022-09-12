#include "device.h"
#include "../gpu/gpu.h"
#include "../debug/debug.h"
#include <stdlib.h>

const char* const requiredDeviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const uint32_t requiredDeviceExtensionCount = sizeof(requiredDeviceExtensions) / sizeof(const char*);

void getRequiredDeviceExtensions(uint32_t* extensionCount, char*** extensions, EngineSettings settings) {

	//get engine required extensions
	//from settings


	//
	*extensionCount =  requiredDeviceExtensionCount;
	*extensions = (char**)fsAllocate(sizeof(char*) * *extensionCount);

	//populate extensions
	memcpy(*extensions, requiredDeviceExtensions, sizeof(char*) * requiredDeviceExtensionCount);

}

void deviceCreate(EngineHandle handle, EngineSettings settings) {

	uint32_t validationLayerCount = 0;
	const char* const* validationLayers = debugGetRequiredValidationLayers(&validationLayerCount);

	QueueFamilyIndices indices = findQueueFamilies(PHYSICAL_DEVICE(handle), handle);

	VkPhysicalDeviceFeatures deviceFeatures = getSelectedDeviceFeatures(settings);

	VkDeviceQueueCreateInfo queueCreateInfos[QUEUE_FAMILY_COUNT] = { 0 };
	float queuePriority = 1.0f;
	int infoIndex = 0;
	for (int i = 0; i < QUEUE_FAMILY_COUNT; i++) {
		uint32_t index = (((QueueFamilyIndex*)&indices) + i)->index;
		VkBool32 unique = VK_TRUE;
		for (int j = 0; j <= i; j++) {
			uint32_t existingIndex = (((QueueFamilyIndex*)&indices) + j)->index;
			if (j == i) {
				continue;
			}
			if (existingIndex == index) {
				unique = VK_FALSE;
				break;
			}
		}
		if (!unique) {
			continue;
		}

		VkDeviceQueueCreateInfo* queueCreateInfo = &queueCreateInfos[infoIndex++];
		queueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo->queueFamilyIndex = index;
		queueCreateInfo->queueCount = 1;
		queueCreateInfo->pQueuePriorities = &queuePriority;
	}

	VkDeviceCreateInfo deviceCreateInfo = { 0 };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
	deviceCreateInfo.queueCreateInfoCount = infoIndex;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledLayerCount = validationLayerCount;
	deviceCreateInfo.ppEnabledLayerNames = validationLayers;

	uint32_t deviceExtensionCount = 0;
	char** deviceExtensions = { 0 };
	getRequiredDeviceExtensions(&deviceExtensionCount, &deviceExtensions, settings);
	deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

	VkCheck(vkCreateDevice(PHYSICAL_DEVICE(handle), &deviceCreateInfo, NULL, &DEVICE(handle)));
	fsFree(deviceExtensions);

	//get queues
	vkGetDeviceQueue(DEVICE(handle), indices.graphicsFamily.index, 0, &GRAPHICS_QUEUE(handle));
	vkGetDeviceQueue(DEVICE(handle), indices.presentFamily.index, 0, &PRESENT_QUEUE(handle));

	handle->graphicsQueue.index = indices.graphicsFamily.index;
	handle->presentQueue.index = indices.presentFamily.index;
	
}

void deviceDestroy(EngineHandle handle) {
	vkDestroyDevice(DEVICE(handle), nullptr);
}
