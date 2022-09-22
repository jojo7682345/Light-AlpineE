#include "Util.h"
#include "../core/core.h"

#include <stdio.h>

bool_t hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkImageView createImageView(EngineHandle handle, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo = { 0 };
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VkCheck(vkCreateImageView(DEVICE(handle), &viewInfo, nullptr, &imageView));

	return imageView;
}

uint32_t findMemoryType(EngineHandle handle, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(PHYSICAL_DEVICE(handle), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	VkCheck(VK_ERROR_OUT_OF_DEVICE_MEMORY);
	return 0;
}

void createImages(EngineHandle handle, uint32_t imageCount, ImageCreateInfo* info, VkMemoryPropertyFlags properties, VkDeviceMemory* imageMemory) {
	
	VkMemoryRequirements memoryRequirements = { 0 };

	VkDeviceSize* imageOffsets = (VkDeviceSize*)fsAllocate(sizeof(VkDeviceSize) * (imageCount+1U));

	for (uint32_t i = 0; i < imageCount; i++) {
	
		VkImageCreateInfo imageInfo = { 0 };
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = info[i].width;
		imageInfo.extent.height = info[i].height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = info[i].format;
		imageInfo.tiling = info[i].tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = info[i].usage;
		imageInfo.samples = info[i].samples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VkCheck(vkCreateImage(DEVICE(handle), &imageInfo, nullptr, (info[i].image)));

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(DEVICE(handle), *(info[i].image), &memRequirements);

		if (memoryRequirements.size != 0) {
			if (memoryRequirements.alignment != memRequirements.alignment) {
				printf("Incompatible memory alignment");
				abort();
			}
			if (memoryRequirements.memoryTypeBits != memRequirements.memoryTypeBits) {
				printf("Incompatible memory type bits");
				abort();
			}
		} else {
			memoryRequirements.alignment = memRequirements.alignment;
			memoryRequirements.memoryTypeBits = memRequirements.memoryTypeBits;
		}
		memoryRequirements.size += memRequirements.size;
		imageOffsets[i+1] = memRequirements.size;
	}

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(handle, memoryRequirements.memoryTypeBits, properties);

	VkCheck(vkAllocateMemory(DEVICE(handle), &allocInfo, nullptr, imageMemory));
	for (uint32_t i = 0; i < imageCount; i++) {
		vkBindImageMemory(DEVICE(handle), *(info[i].image), *imageMemory, imageOffsets[i]);
	}
	fsFree(imageOffsets);
}

void createImage(EngineHandle handle, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* imageMemory) {

	VkImageCreateInfo imageInfo = { 0 };
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkCheck(vkCreateImage(DEVICE(handle), &imageInfo, nullptr, image));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(DEVICE(handle), *image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(handle, memRequirements.memoryTypeBits, properties);

	VkCheck(vkAllocateMemory(DEVICE(handle), &allocInfo, nullptr, imageMemory));

	vkBindImageMemory(DEVICE(handle), *image, *imageMemory, 0);
}



VkFormat findSupportedFormat(EngineHandle handle, uint32_t candidateCount, const VkFormat* candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (uint32_t i = 0; i < candidateCount; i++) {
		VkFormat format = candidates[i];
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(PHYSICAL_DEVICE(handle), format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	VkCheck(VK_ERROR_FORMAT_NOT_SUPPORTED);
	return VK_FORMAT_UNDEFINED;
}

VkCommandBuffer beginSingleTimeCommands(EngineHandle handle) {
	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = COMMAND_POOL(handle);
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	vkAllocateCommandBuffers(DEVICE(handle), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = { 0 };

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}


void endSingleTimeCommands(EngineHandle handle, VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(GRAPHICS_QUEUE(handle), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GRAPHICS_QUEUE(handle));

	vkFreeCommandBuffers(DEVICE(handle), COMMAND_POOL(handle), 1, &commandBuffer);
}

void transitionImageLayout(EngineHandle handle, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(handle);

	VkImageMemoryBarrier barrier = { 0 };
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_NONE;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_NONE;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		VkCheck(VK_ERROR_FORMAT_NOT_SUPPORTED);
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(handle, commandBuffer);
}