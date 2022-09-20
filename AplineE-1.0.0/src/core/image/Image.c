#include "Image.h"
#include <util/util.h>
#include <util/translate.h>
#include <math.h>
#include <stdio.h>
#include <MemoryUtilities/DataGrid.h>
#include <MemoryUtilities/DataTable.h>


#pragma region ImagePool

void imagePoolCreate(EngineHandle handle, ImagePoolCreateInfo info, ImagePool* pool) {
	const uint32_t stackCount = info.imageStackCount;

	ImagePool_T* imagePool = (ImagePool_T*)fsAllocate(sizeof(ImagePool_T));
	imagePool->handle = handle;
	imagePool->imageStackCount = stackCount;
	imagePool->imageStacks = (ImageStack_T*)fsAllocate(sizeof(ImageStack_T) * stackCount);

	uint32_t poolImageCount = 0;
	for (uint32_t i = 0; i < stackCount; i++) {
		const ImageStackLayout layout = info.imageStacks[i];
		poolImageCount += layout.imageCount;
	}
	ImageCreateInfo* imageCreateInfos = (ImageCreateInfo*)fsAllocate(sizeof(ImageCreateInfo) * poolImageCount);
	VkImageView** imageViews = (VkImageView**)fsAllocate(sizeof(VkImageView*) * poolImageCount);
	VkImageAspectFlags* aspectFlags = (VkImageAspectFlags*)fsAllocate(sizeof(VkImageAspectFlags) * poolImageCount);

	uint32_t imageIndex = 0;
	for (uint32_t i = 0; i < stackCount; i++) {
		ImageStackLayout layout = info.imageStacks[i];
		const uint32_t imageCount = layout.imageCount;

		const VkSampleCountFlags sampleCountFlags = translateSampleCount(layout.sampleCount);

		ImageStack_T* stack = &imagePool->imageStacks[i];
		stack->sampleCount = sampleCountFlags;
		stack->imageCount = imageCount;
		stack->extent = (VkExtent2D){.width = layout.width, .height = layout.height};

		stack->images = (VkImage*)fsAllocate(sizeof(VkImage) * imageCount);
		stack->imageViews = (VkImageView*)fsAllocate(sizeof(VkImageView) * imageCount);
		stack->formats = (VkFormat*)fsAllocate(sizeof(VkFormat) * imageCount);
		stack->usage = (VkImageUsageFlags*)fsAllocate(sizeof(VkImageUsageFlags) * imageCount);
		stack->imageReferences = (ImageReference_T*)fsAllocate(sizeof(ImageReference_T) * imageCount);
		stack->aspectFlags = (VkImageAspectFlags*)fsAllocate(sizeof(VkImageAspectFlags) * imageCount);

		stack->pool = imagePool;

		for (uint32_t j = 0; j < imageCount; j++) {
			const VkFormat format = tanslateImageFormat(layout.imageDescriptions[j].imageFormat);
			const VkImageUsageFlags usageFlags = translateImageUsage(layout.imageDescriptions[j].imageUsage);
			const VkImageAspectFlags imageAspectFlags = translateImageAspect(layout.imageDescriptions[j].imageAspect);

			ImageCreateInfo* imageCreateInfo = &imageCreateInfos[imageIndex + j];
			imageCreateInfo->format = format;
			imageCreateInfo->usage = usageFlags;
			imageCreateInfo->image = &stack->images[j];
			imageCreateInfo->samples = sampleCountFlags;
			imageCreateInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo->width = stack->extent.width;
			imageCreateInfo->height = stack->extent.height;

			stack->formats[j] = format;
			stack->usage[j] = usageFlags;
			stack->aspectFlags[j] = imageAspectFlags;

			imageViews[imageIndex + j] = &stack->imageViews[j];
			aspectFlags[imageIndex + j] = imageAspectFlags;

			ImageReference_T* image = &(imagePool->imageStacks[i].imageReferences[j]);
			image->imageIndex = j;
			image->image = &(imagePool->imageStacks[i].images[j]);
			image->extent = &(imagePool->imageStacks[i].extent);
			image->format = &(imagePool->imageStacks[i].formats[j]);
			image->imageView = &(imagePool->imageStacks[i].imageViews[j]);
			image->usage = &(imagePool->imageStacks[i].usage[j]);
			image->aspectFlags = &(imagePool->imageStacks[i].aspectFlags[j]);
			image->stack = &(imagePool->imageStacks[i]);

			*(layout.imageDescriptions[j].imageReference) = image;
			*layout.imageStack = stack;
		}
		imageIndex += imageCount;
	}

	createImages(handle, poolImageCount, imageCreateInfos, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imagePool->memory);
	for (uint32_t i = 0; i < poolImageCount; i++) {
		*(imageViews[i]) = createImageView(handle, *(imageCreateInfos[i].image), imageCreateInfos[i].format, aspectFlags[i]);
	}

	*pool = imagePool;

	fsFree(aspectFlags);
	fsFree(imageViews);
	fsFree(imageCreateInfos);
}

void imagePoolDestroy(ImagePool pool) {
	EngineHandle handle = pool->handle;

	for (uint32_t i = 0; i < pool->imageStackCount; i++) {
		const ImageStack_T stack = pool->imageStacks[i];
		for (uint32_t j = 0; j < stack.imageCount; j++) {
			vkDestroyImageView(handle->device, stack.imageViews[j], nullptr);
			vkDestroyImage(handle->device, stack.images[j], nullptr);
		}
		fsFree(stack.images);
		fsFree(stack.imageViews);
		fsFree(stack.formats);
		fsFree(stack.usage);
		fsFree(stack.aspectFlags);
		fsFree(stack.imageReferences);
	}

	fsFree(pool->imageStacks);
	vkFreeMemory(handle->device, pool->memory, nullptr);
	fsFree(pool);
}

#pragma endregion

void renderChainCreate(EngineHandle handle, RenderChainCreateInfo info, RenderChain* renderChain) {

	RenderChain chain = fsAllocate(sizeof(RenderChain_T));
	*renderChain = chain;
	chain->handle = handle;
	

	uint32_t attachmentIndex = 0;

	//create depth resources
	uint32_t impliedDepthImageCount = 0;
	for (uint32_t i = 0; i < info.renderModuleCount; i++) {
		RenderChainRenderModule* module = (info.renderModules + i);
		if (module->type != RENDER_MODULE_TYPE_RENDER) {
			continue;
		}
		if (module->depthBuffered) {
			continue;
		}
		if (module->depthImage) {
			continue;
		}
		impliedDepthImageCount++;
	}
	uint32_t definedDepthImageCount = info.depthImageResourceCount;
	uint32_t depthImageCount = definedDepthImageCount + impliedDepthImageCount;

	uint32_t impliedColorImageCount = 0;
	for (uint32_t i = 0; i < info.renderModuleCount; i++) {
		RenderChainRenderModule* module = (info.renderModules + i);
		if (module->type != RENDER_MODULE_TYPE_RENDER) {
			continue;
		}
		if (module->sampleCount!=IMAGE_SAMPLE_COUNT_1) {
			continue;
		}

		impliedDepthImageCount += module->outputImageCount;
	}
	uint32_t definedColorImageCount = info.imageResourceCount;
	uint32_t colorImageCount = definedColorImageCount + impliedColorImageCount;

	uint32_t subResourceImageCount = info.imageSubResourceCount;

	uint32_t imageCount = colorImageCount + depthImageCount + subResourceImageCount;
	chain->images = fsAllocate(sizeof(Image_T) * imageCount);

	_DataTable depthImageTable = { 0 };
	size_t imageDescriptorTableLayout[] = {
		sizeof(ImageReferenceHandle), //Image handle
		sizeof(ImageCreateInfo), //ImageCreateInfo
		sizeof(uint32_t) //attachment Index
	};
	dataTableCreate(depthImageCount, sizeof(imageDescriptorTableLayout) / sizeof(size_t), imageDescriptorTableLayout, &depthImageTable);
	uint32_t depthImageIndex = 0;
	for (uint32_t i = 0; i < definedDepthImageCount; depthImageIndex++ & i++) {
		ImageReferenceHandle* depthImageHandle = info.depthImageResources + i;
		dataTableSetElement(depthImageIndex, 0, depthImageHandle, &depthImageTable);

		ImageCreateInfo depthInfo = { 0 };
		depthInfo.format = VK_FORMAT_D32_SFLOAT;
		depthInfo.width = engineWindowGetWidth(handle);
		depthInfo.height = engineWindowGetHeight(handle);

	}



	//create image resources;

	uint32_t attachmentCount = 0;

	VkAttachmentDescription* imageAttachments = fsAllocate(sizeof(VkAttachmentDescription) * attachmentCount);

	VkRenderPassCreateInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = imageAttachments;

	VkCheck(vkCreateRenderPass(DEVICE(handle), &renderPassInfo, nullptr, &chain->renderPass));



}

void renderChainDestroy(RenderChain renderChain) {


}
