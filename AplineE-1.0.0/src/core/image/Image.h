#pragma once
#include <core/core.h>

#pragma region ImageStack & ImagePool

typedef struct ImageReference_T {
	uint32_t imageIndex;
	VkImage* image;
	VkImageView* imageView;
	VkFormat* format;
	VkExtent2D* extent;
	VkImageAspectFlags* aspectFlags;
	VkImageUsageFlags* usage;
	ImageStack stack;
}ImageReference_T;

typedef struct ImageDescription {
	ImageReference* imageReference;
	ImageFormat imageFormat;
	ImageUsageFlags imageUsage;
	ImageAspectFlags imageAspect;
	VkSampleCountFlags sampleCount;
}ImageDescription;

typedef struct ImageStack_T {
	uint32_t imageCount;
	
	VkImage* images;
	VkImageView* imageViews;
	VkFormat* formats;
	VkImageUsageFlags* usage;
	VkImageAspectFlags* aspectFlags;
	ImageReference_T* imageReferences;

	VkExtent2D extent;

	VkSampleCountFlags sampleCount;
	
	ImagePool pool;
	
}ImageStack_T;

typedef struct ImagePool_T {
	EngineHandle handle;
	
	uint32_t imageStackCount;
	ImageStack_T* imageStacks;
	
	VkDeviceMemory memory;
}ImagePool_T;

#pragma endregion

