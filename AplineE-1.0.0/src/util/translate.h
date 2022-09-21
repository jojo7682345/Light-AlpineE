#pragma once
#include <AlpineCore.h>
#include <vulkan/vulkan.h>

VkImageAspectFlags translateImageAspect(ImageAspectFlags aspectFlags);

VkImageUsageFlags translateImageUsage(ImageUsageFlags imageUsage);

VkSampleCountFlags translateSampleCount(ImageSampleCount sampleCount);

VkFormat translateImageFormat(ImageFormat format);

VkAccessFlags translateAccessFlags(RenderModuleAccessFlags access);