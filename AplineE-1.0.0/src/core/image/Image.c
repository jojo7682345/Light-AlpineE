#include "Image.h"
#include <util/util.h>
#include <util/translate.h>
#include <math.h>
#include <stdio.h>
#include <MemoryUtilities/DataGrid.h>


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

typedef struct ImageLifetimeData {
	bool_t exported;
	ExportStage exportStage;


	VkFormat format;
	VkSampleCountFlags sampleCount;

	bool_t imported;

	ImageType type;
}ImageLifetimeData;

typedef struct DependencyData {
	uint32_t moduleSrc;
	uint32_t moduleDst;
}DependencyData;


void rendererCreate(EngineHandle handle, RendererCreateInfo info, Renderer* rendererHandle) {
	Renderer renderer = fsAllocate(sizeof(Renderer_T));
	*rendererHandle = renderer;

	renderer->renderModuleCount = info.renderModuleCount;
	renderer->renderModules = fsAllocate(sizeof(RenderModule_T) * renderer->renderModuleCount);
	for (uint32_t i = 0; i < renderer->renderModuleCount; i++) {
		RenderModule renderModule = renderer->renderModules + i;
		RenderModuleDescription description = info.renderModules[i];
		*description.handle = renderModule;

		renderModule->handle = handle;
		renderModule->type = description.type;
		

		renderer->renderModuleImageCount += description.colorOutputCount;
		renderer->renderModuleImageCount += description.colorInputCount;
		renderer->renderModuleImageCount += description.depthBuffered ? 1 : 0;
	}
	

	ImageLifetimeData* imageData;
	
	uint32_t attachmentCount = 0;
	VkAttachmentDescription* imageAttachments = fsAllocate(sizeof(VkAttachmentDescription) * attachmentCount);
	for (uint32_t i = 0; i < attachmentCount; i++) {
		VkAttachmentDescription* attachment = imageAttachments + i;
		
		attachment->flags = 0;
		
		attachment->finalLayout = imageData[i].exported ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
		attachment->initialLayout = imageData[i].imported ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_UNDEFINED;

		attachment->loadOp = imageData[i].imported ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment->storeOp = imageData[i].exported ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

		attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		attachment->format = imageData[i].format;
		attachment->samples = imageData[i].sampleCount;
	}

	DependencyData* dependencyData;

	uint32_t dependencyCount = 0;
	VkSubpassDependency* dependencies = fsAllocate(sizeof(VkSubpassDependency) * dependencyCount + 2);
	{
		uint32_t dependencyIndex = 0;
		for (uint32_t i = 0; i < dependencyCount; i++ & dependencyIndex++) {
			VkSubpassDependency* dependency = dependencies + dependencyIndex;
			DependencyData dep = dependencyData[i];
			dependency->dependencyFlags = 0;
			dependency->srcSubpass = dep.moduleSrc;
			dependency->dstSubpass = dep.moduleDst;
			
		}
		for (uint32_t i = 0; i < dependencyCount; i++ & dependencyIndex++) {
			VkSubpassDependency* dependency = dependencies + dependencyIndex;
			//import dependencies;
		}
		for (uint32_t i = 0; i < dependencyCount; i++ & dependencyIndex++) {
			VkSubpassDependency* dependency = dependencies + dependencyIndex;
			//export dependencies;
		}
		
	}

	uint32_t subpassCount = 0;
	VkSubpassDescription* subpasses = fsAllocate(sizeof(VkSubpassDescription) * subpassCount);
	for (uint32_t i = 0; i < subpassCount; i++) {
		VkSubpassDescription* subpass = subpasses + i;
		subpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		


	}

	VkRenderPassCreateInfo renderPassInfo = { 0 };
	renderPassInfo.attachmentCount = attachmentCount;
	renderPassInfo.pAttachments = imageAttachments;
	renderPassInfo.subpassCount = subpassCount;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = dependencyCount;
	renderPassInfo.pDependencies = dependencies;

	VkCheck(vkCreateRenderPass(DEVICE(handle), &renderPassInfo, nullptr, &renderer->renderPass));


}

void rendererDestroy(Renderer renderer) {

}
