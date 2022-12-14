#pragma once
#include <AlpineCore.h>
#include <vulkan/vulkan.h>

VkImageAspectFlags translateImageAspect(ImageAspectFlags aspectFlags);

VkImageUsageFlags translateImageUsage(ImageUsageFlags imageUsage);

VkSampleCountFlags translateSampleCount(ImageSampleCount sampleCount);

VkFormat tanslateImageFormat(ImageFormat format);
