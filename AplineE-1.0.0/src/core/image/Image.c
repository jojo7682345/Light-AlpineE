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
		stack->extent = (VkExtent2D){ .width = layout.width, .height = layout.height };

		stack->images = (VkImage*)fsAllocate(sizeof(VkImage) * imageCount);
		stack->imageViews = (VkImageView*)fsAllocate(sizeof(VkImageView) * imageCount);
		stack->formats = (VkFormat*)fsAllocate(sizeof(VkFormat) * imageCount);
		stack->usage = (VkImageUsageFlags*)fsAllocate(sizeof(VkImageUsageFlags) * imageCount);
		stack->imageReferences = (ImageReference_T*)fsAllocate(sizeof(ImageReference_T) * imageCount);
		stack->aspectFlags = (VkImageAspectFlags*)fsAllocate(sizeof(VkImageAspectFlags) * imageCount);

		stack->pool = imagePool;

		for (uint32_t j = 0; j < imageCount; j++) {
			const VkFormat format = translateImageFormat(layout.imageDescriptions[j].imageFormat);
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

void imageAllocateHandle(ImageRef ref) {
	*ref = fsAllocate(sizeof(Image_T));
}

#pragma endregion

typedef enum ImageUsage {
	IMAGE_USE_COLOR,
	IMAGE_USE_DEPTH,
	IMAGE_USE_RESOLVE
}ImageUsage;

void rendererCreate(EngineHandle handle, RendererCreateInfo info, Renderer* pRenderer) {

	*pRenderer = fsAllocate(sizeof(Renderer_T));
	Renderer renderer = *pRenderer;
	renderer->handle = handle;
	uint32_t imageWidth = info.width ? info.width : engineWindowGetWidth(handle);
	uint32_t imageHeight = info.height ? info.height : engineWindowGetHeight(handle);
	VkExtent2D extent = { .width = imageWidth,.height = imageHeight };

	renderer->size = extent;

#pragma region Create Image and attachments
	uint32_t impliedImageCount = 0;

	//count additional resolve images
	for (uint32_t moduleIndex = 0; moduleIndex < info.renderModuleCount; moduleIndex++) {
		RenderModuleDescription module = info.renderModules[moduleIndex];
		if (module.sampleCount != IMAGE_SAMPLE_COUNT_1) {
			impliedImageCount += module.outputImageCount;
		}
	}

	renderer->impliedImageCount = impliedImageCount;
	renderer->impliedImages = fsAllocate(sizeof(Image_T) * impliedImageCount);
	renderer->impliedImageRefs = fsAllocate(sizeof(Image) * impliedImageCount);

	ImageResourceDescription* impliedResourceDescriptions = fsAllocate(sizeof(ImageResourceDescription) * impliedImageCount);
	uint32_t* impliedResourceModuleIndices = fsAllocate(sizeof(uint32_t) * impliedImageCount);
	VkSampleCountFlags* impliedResourceSampleCounts = fsAllocate(sizeof(VkSampleCountFlags) * impliedImageCount);

	uint32_t impliedImageIndex = 0;
	for (uint32_t i = 0; i < info.renderModuleCount; i++) {
		RenderModuleDescription module = info.renderModules[i];
		if (module.sampleCount != IMAGE_SAMPLE_COUNT_1) {
			for (uint32_t j = 0; j < module.outputImageCount; j++) {
				ImageResourceDescription* description = impliedResourceDescriptions;
				description->image = renderer->impliedImageRefs + impliedImageIndex;
				description->initialAccessType = IMAGE_ACCESS_TYPE_UNDEFINED;
				description->finalAccessType = IMAGE_ACCESS_TYPE_UNDEFINED;
				uint32_t* moduleIndex = impliedResourceModuleIndices + impliedImageIndex;
				*moduleIndex = i;
				VkSampleCountFlags* sampleCount = impliedResourceSampleCounts + impliedImageIndex;
				*sampleCount = translateSampleCount(module.sampleCount);
				impliedImageIndex++;
			}
		}
	}

	renderer->imageCount = info.imageResourceCount;
	renderer->images = fsAllocate(sizeof(Image_T) * info.imageResourceCount);

	renderer->depthImageCount = info.depthImageResourceCount;
	renderer->depthImages = fsAllocate(sizeof(Image_T) * info.depthImageResourceCount);

	uint32_t imageCount = 0;
	imageCount += impliedImageCount;
	imageCount += info.imageResourceCount;
	imageCount += info.depthImageResourceCount;

	VkAttachmentDescription* attachments = fsAllocate(sizeof(VkAttachmentDescription) * imageCount);
	VkImageView* framebufferAttachments = fsAllocate(sizeof(VkImageView) * imageCount);

	_DataTable imageCreationTable = { 0 };
	size_t imageCreationTableSizes[] = {
		sizeof(ImageRef),
		sizeof(ImageCreateInfo),
		sizeof(VkImageAspectFlags)
	};
	dataTableCreate(imageCount, sizeof(imageCreationTableSizes) / sizeof(size_t), imageCreationTableSizes, &imageCreationTable);

	{
		uint32_t attachmentIndex = 0;
		for (uint32_t i = 0; i < impliedImageCount; i++) {
			VkAttachmentDescription* attachment = attachments + attachmentIndex;

			VkFormat format = translateImageFormat(engineGetDepthImageFormat(handle));
			VkSampleCountFlags sampleCount = translateSampleCount(impliedResourceSampleCounts[i]);

			attachment->flags = 0;
			attachment->format = format;
			attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment->samples = sampleCount;
			dataTableSetElement(attachmentIndex, 0, renderer->impliedImageRefs + i, &imageCreationTable);

			ImageCreateInfo createInfo = { 0 };
			createInfo.image = &(renderer->impliedImages + i)->image;
			createInfo.format = format;
			createInfo.samples = sampleCount;
			createInfo.width = imageWidth;
			createInfo.height = imageHeight;
			createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			dataTableSetElement(attachmentIndex, 1, &createInfo, &imageCreationTable);

			*(renderer->impliedImageRefs + i) = renderer->impliedImages + i;
			(*(renderer->impliedImageRefs + i))->format = format;
			(*(renderer->impliedImageRefs + i))->extent = extent;

			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			dataTableSetElement(attachmentIndex, 2, &aspect, &imageCreationTable);
			attachmentIndex++;
		}
		for (uint32_t i = 0; i < info.imageResourceCount; i++) {
			VkAttachmentDescription* attachment = attachments + attachmentIndex;
			ImageResourceDescription imageResource = info.imageResources[i];

			VkFormat format = translateImageFormat(engineGetColorImageFormat(handle));
			VkSampleCountFlags sampleCount = VK_SAMPLE_COUNT_1_BIT;

			attachment->format = format;
			attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			if (imageResource.initialAccessType == IMAGE_ACCESS_TYPE_RENDER_INPUT ||
				imageResource.initialAccessType == IMAGE_ACCESS_TYPE_RENDER_OUTPUT ||
				imageResource.initialAccessType == IMAGE_ACCESS_TYPE_SHADER_IMAGE_TRACK) {

				attachment->loadOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				attachment->loadOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}

			attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			if (imageResource.finalAccessType == IMAGE_ACCESS_TYPE_RENDER_OUTPUT ||
				imageResource.finalAccessType == IMAGE_ACCESS_TYPE_RENDER_INPUT ||
				imageResource.finalAccessType == IMAGE_ACCESS_TYPE_SHADER_IMAGE_TRACK) {

				attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			attachment->flags = 0;
			attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment->samples = sampleCount;
			dataTableSetElement(attachmentIndex, 0, &imageResource.image, &imageCreationTable);

			(*(imageResource.image)) = renderer->images + i;
			(*(imageResource.image))->format = format;
			(*(imageResource.image))->extent = extent;

			ImageCreateInfo createInfo = { 0 };
			createInfo.image = &((*imageResource.image)->image);
			createInfo.format = format;
			createInfo.samples = sampleCount;
			createInfo.width = imageWidth;
			createInfo.height = imageHeight;
			createInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			dataTableSetElement(attachmentIndex, 1, &createInfo, &imageCreationTable);

			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			dataTableSetElement(attachmentIndex, 2, &aspect, &imageCreationTable);

			attachmentIndex++;
		}
		for (uint32_t i = 0; i < info.depthImageResourceCount; i++) {
			VkAttachmentDescription* attachment = attachments + attachmentIndex;
			ImageDepthResourceDescription imageResource = info.depthImageResources[i];

			VkFormat format = translateImageFormat(engineGetDepthImageFormat(handle));
			VkSampleCountFlags sampleCount = translateSampleCount(imageResource.sampleCount);

			attachment->format = format;
			attachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			if (imageResource.initialAccessType == IMAGE_ACCESS_TYPE_RENDER_INPUT ||
				imageResource.initialAccessType == IMAGE_ACCESS_TYPE_RENDER_OUTPUT ||
				imageResource.initialAccessType == IMAGE_ACCESS_TYPE_SHADER_IMAGE_TRACK) {

				attachment->loadOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				attachment->loadOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}

			attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			if (imageResource.finalAccessType == IMAGE_ACCESS_TYPE_RENDER_OUTPUT ||
				imageResource.finalAccessType == IMAGE_ACCESS_TYPE_RENDER_INPUT ||
				imageResource.finalAccessType == IMAGE_ACCESS_TYPE_SHADER_IMAGE_TRACK) {

				attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			} else {
				attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			attachment->flags = 0;
			attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment->samples = sampleCount;

			dataTableSetElement(attachmentIndex, 0, &imageResource.image, &imageCreationTable);

			*(imageResource.image) = renderer->depthImages + i;
			(*(imageResource.image))->format = format;
			(*(imageResource.image))->extent = extent;


			ImageCreateInfo createInfo = { 0 };
			createInfo.image = &((*imageResource.image)->image);
			createInfo.format = format;
			createInfo.samples = sampleCount;
			createInfo.width = imageWidth;
			createInfo.height = imageHeight;
			createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			dataTableSetElement(attachmentIndex, 1, &createInfo, &imageCreationTable);

			VkImageAspectFlags aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			dataTableSetElement(attachmentIndex, 2, &aspect, &imageCreationTable);

			attachmentIndex++;
		}
	}


	ImageCreateInfo* imageCreateInfos = fsAllocate(sizeof(ImageCreateInfo) * imageCount);
	dataTableGetColumn(1, imageCreateInfos, &imageCreationTable);
	createImages(handle, imageCount, imageCreateInfos, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderer->imageMemory);

	for (uint32_t i = 0; i < imageCount; i++) {
		ImageRef ref;
		dataTableGetElement(i, 0, &ref, &imageCreationTable);
		VkImageAspectFlags aspect;
		dataTableGetElement(i, 2, &aspect, &imageCreationTable);

		Image image = *ref;
		image->view = createImageView(handle, image->image, image->format, aspect);
		framebufferAttachments[i] = image->view;
	}
#pragma endregion
#pragma region build subpasses
	VkSubpassDescription* subpasses = fsAllocate(sizeof(VkSubpassDescription) * info.renderModuleCount);

	uint32_t resolveIndex = 0;

	for (uint32_t i = 0; i < info.renderModuleCount; i++) {
		RenderModuleDescription moduleDescription = info.renderModules[i];

		VkSubpassDescription* subpass = subpasses + i;

		subpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass->inputAttachmentCount = moduleDescription.inputImageCount;
		subpass->colorAttachmentCount = moduleDescription.outputImageCount;
		subpass->preserveAttachmentCount = moduleDescription.preserveImageCount;

		VkAttachmentReference* inputAttachments = fsAllocate(sizeof(VkAttachmentReference) * moduleDescription.inputImageCount);
		for (uint32_t j = 0; j < moduleDescription.inputImageCount; j++) {
			VkAttachmentReference* attchRef = inputAttachments + j;
			attchRef->attachment = dataTableFindRow(0, moduleDescription.inputImages + j, &imageCreationTable);
			attchRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference* outputAttachments = fsAllocate(sizeof(VkAttachmentReference) * moduleDescription.outputImageCount);
		for (uint32_t j = 0; j < moduleDescription.outputImageCount; j++) {
			VkAttachmentReference* attchRef = outputAttachments + j;
			attchRef->attachment = dataTableFindRow(0, moduleDescription.outputImages + j, &imageCreationTable);
			attchRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		bool_t multisampled = moduleDescription.sampleCount != IMAGE_SAMPLE_COUNT_1;

		if (multisampled) {
			VkAttachmentReference* resolveAttachments = fsAllocate(sizeof(VkAttachmentReference) * moduleDescription.outputImageCount);
			for (uint32_t j = 0; j < moduleDescription.outputImageCount; j++) {
				VkAttachmentReference* attchRef = resolveAttachments + j;
				attchRef->attachment = resolveIndex++;
				attchRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			subpass->pColorAttachments = subpass->pColorAttachments;
		}

		uint32_t* preserveAttachments = fsAllocate(sizeof(uint32_t) * moduleDescription.preserveImageCount);
		for (uint32_t j = 0; j < moduleDescription.preserveImageCount; j++) {
			*(preserveAttachments + j) = dataTableFindRow(0, moduleDescription.preserveImages + j, &imageCreationTable);
		}

		bool_t depthBuffered = moduleDescription.depthImage != nullptr;
		if (depthBuffered) {
			VkAttachmentReference* depthAttachment = fsAllocate(sizeof(VkAttachmentDescription));
			depthAttachment->attachment = dataTableFindRow(0, moduleDescription.depthImage, &imageCreationTable);
			depthAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			subpass->pDepthStencilAttachment = depthAttachment;
		}


		subpass->pColorAttachments = multisampled ? subpass->pColorAttachments : outputAttachments;
		subpass->pResolveAttachments = multisampled ? outputAttachments : nullptr;
		subpass->pPreserveAttachments = preserveAttachments;
	}
#pragma endregion
#pragma region build dependencies
	VkSubpassDependency* dependencies = fsAllocate(sizeof(VkSubpassDependency) * info.dependencyCount);
	for (uint32_t i = 0; i < info.dependencyCount; i++) {
		VkSubpassDependency* dependency = dependencies + i;
		RenderModuleDependency dep = info.dependencies[i];

		for (uint32_t j = 0; j < info.renderModuleCount; j++) {
			RenderModuleDescription descript = info.renderModules[j];

			if (descript.handle == dep.src) {
				dependencies->srcSubpass = j;
			}
			if (descript.handle == dep.dst) {
				dependencies->dstSubpass = j;
			}
		}

		if (dep.src == RENDERER_INPUT || dep.src == RENDERER_OUTPUT) {
			dependencies->srcSubpass = VK_SUBPASS_EXTERNAL;
		}
		if (dep.dst == RENDERER_INPUT || dep.dst == RENDERER_OUTPUT) {
			dependencies->dstSubpass = VK_SUBPASS_EXTERNAL;
		}

		dependency->dependencyFlags = 0;

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE;
		switch (dep.srcStage) {
		case RENDER_MODULE_STAGE_VERTEX_OUTPUT:
			srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			break;
		case RENDER_MODULE_STAGE_DEPTH_OUTPUT:
			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		case RENDER_MODULE_STAGE_FRAGMENT_OUTPUT:
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		dependency->srcStageMask = srcStage;

		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE;
		switch (dep.dstStage) {
		case RENDER_MODULE_STAGE_VERTEX_INPUT:
			dstStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			break;
		case RENDER_MODULE_STAGE_DEPTH_INPUT:
			dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			break;
		case RENDER_MODULE_STAGE_FRAGMENT_INPUT:
			dstStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		}
		dependency->dstStageMask = dstStage;


		dependency->srcAccessMask = translateAccessFlags(dep.srcAccess);
		dependency->dstAccessMask = translateAccessFlags(dep.dstAccess);
	}
#pragma endregion

	VkRenderPassCreateInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = imageCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = info.renderModuleCount;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = info.dependencyCount;
	renderPassInfo.pDependencies = dependencies;
	VkCheck(vkCreateRenderPass(DEVICE(handle), &renderPassInfo, nullptr, &renderer->renderPass));

	VkFramebufferCreateInfo framebufferInfo = { 0 };
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.flags = 0;
	framebufferInfo.width = imageWidth;
	framebufferInfo.height = imageHeight;
	framebufferInfo.layers = 1;
	framebufferInfo.renderPass = renderer->renderPass;
	framebufferInfo.pAttachments = framebufferAttachments;
	VkCheck(vkCreateFramebuffer(DEVICE(handle), &framebufferInfo, nullptr, &renderer->framebuffer));

}

void rendererDestroy(Renderer renderer) {


}
