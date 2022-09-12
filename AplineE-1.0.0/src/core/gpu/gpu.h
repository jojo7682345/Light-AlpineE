#pragma once
#include "../core.h"

void gpuSelectSuitable(EngineHandle handle, EngineSettings settings);

typedef struct QueueFamilyIndex {
	uint32_t index;
	VkBool32 present;

}QueueFamilyIndex;
typedef struct QueueFamilyIndices {
	QueueFamilyIndex graphicsFamily;
	QueueFamilyIndex presentFamily;
}QueueFamilyIndices;

#define QUEUE_FAMILY_COUNT (sizeof(QueueFamilyIndices) / sizeof(QueueFamilyIndex))

VkBool32 queueFamilyIndicesComplete(QueueFamilyIndices indices);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, EngineHandle handle);

VkPhysicalDeviceFeatures getSelectedDeviceFeatures(EngineSettings settings);

void gpuUpdateSystemCapabilities(EngineHandle handle);

