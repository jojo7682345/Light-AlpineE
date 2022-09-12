#pragma once
#include "../core/core.h"

VkImageView createImageView(EngineHandle handle, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

uint32_t findMemoryType(EngineHandle handle, uint32_t typeFilter, VkMemoryPropertyFlags properties);

typedef struct ImageCreateInfo {

	uint32_t width;
	uint32_t height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkSampleCountFlags samples;
	VkImage* image;

}ImageCreateInfo;

void createImages(EngineHandle handle, uint32_t imageCount, ImageCreateInfo* info, VkMemoryPropertyFlags properties, VkDeviceMemory* imageMemory);

void createImage(EngineHandle handle, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory);

VkFormat findSupportedFormat(EngineHandle handle, uint32_t candidateCount, const VkFormat* candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

VkCommandBuffer beginSingleTimeCommands(EngineHandle handle);

void endSingleTimeCommands(EngineHandle handle, VkCommandBuffer commandBuffer);

bool_t hasStencilComponent(VkFormat format);

void transitionImageLayout(EngineHandle handle, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);


