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

typedef struct Image_T {
	VkImage image;
	VkImageView view;
	VkFormat format;
	VkExtent2D extent;
}Image_T;

typedef struct RenderModule_T {
	EngineHandle handle;
	RenderModuleType type;



}RenderModule_T;

typedef struct Renderer_T {
	EngineHandle handle;

	VkExtent2D size;

	uint32_t impliedImageCount;
	Image_T* impliedImages;
	Image* impliedImageRefs;

	uint32_t imageCount;
	Image_T* images;

	uint32_t depthImageCount;
	Image_T* depthImages;

	//uint32_t imageResourceCount;
	//ImageRef* imageResources;

	VkDeviceMemory imageMemory;

	uint32_t renderModuleCount;
	RenderModule renderModules;

	uint32_t indirectImageBarrierCount;
	VkImageMemoryBarrier indirectImageBarriers;

	VkRenderPass renderPass;
	VkFramebuffer framebuffer;

}Renderer_T;
