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

#pragma region RenderModules

typedef struct RenderModuleGeometry_T {
	//render command collector;
	ImageReference depthImage;
	ImageReference* resolveImages;
}RenderModuleGeometry_T;

typedef struct RenderModuleImageCombine_T {
	VkPipeline pipeline;
	VkPipelineLayout layout;
}RenderModuleImageCombine_T;

typedef struct RenderModulePostProcess_T {
	VkPipeline pipeline;
	VkPipelineLayout layout;
}RenderModulePostProcess_T;


typedef struct RenderModule_T {
	RenderModuleType type;
	EngineHandle handle;

	DirectInputImages;
	ShaderInputImages;

	ShaderOutputImages;
	RenderModuleOutput;

	union {
		RenderModuleGeometry_T geometry;
		RenderModuleImageCombine_T imageCombine;
		RenderModulePostProcess_T postProcess;
	};
}RenderModule_T;

typedef struct RenderModuleCreateInfoType {
	RenderModuleType type;
}RenderModuleCreateInfoType;

#pragma endregion

#pragma region ComputeModules

typedef struct ComputeModuleCompute_T {
	uint32_t groupCountX;
	uint32_t groupCountY;
	uint32_t groupCountZ;
}ComputeModuleCompute_T;

typedef struct ComputeModulePostProcess_T {
	VkExtent2D* imageExtent;
}ComputeModulePostProcess_T;

typedef struct ComputeModuleImageCombine_T {
	VkExtent2D* imageExtent;
}ComputeModuleImageCombine_T;

typedef struct ComputeModule_T {
	ComputeModuleType type;
	EngineHandle handle;
	
	ShaderInputImages;
	ShaderOutputImages;

	VkPipeline pipeline;
	VkPipelineLayout layout;

	union {
		ComputeModuleCompute_T compute;
		ComputeModuleImageCombine_T imageCombine;
		ComputeModulePostProcess_T postProcess;
	};
}ComputeModule_T;

#pragma endregion

#pragma region RenderChain

typedef struct RenderChain_T{
	EngineHandle handle;

	uint32_t preRenderComputeModuleCount;
	ComputeModule* preRenderComputeModules;

	uint32_t renderModuleCount;
	RenderModule* renderModules;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
	ImageSize framebufferSize;

	uint32_t postRenderComputeModuleCount;
	ComputeModule* postRenderComputeModules;
}RenderChain_T;

#pragma endregion


#pragma region GraphicsChain



typedef struct GraphicsChain_T {

	uint32_t a;

}GraphicsChain_T;



#pragma endregion