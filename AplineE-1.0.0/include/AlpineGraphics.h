#pragma once
#include "AlpineImageFormats.h"

#pragma region ImageStack & ImagePool

DEFINE_HANDLE(ImageReference);
DEFINE_HANDLE(ImageStack);
DEFINE_HANDLE(ImagePool);

typedef ImageReference* ImageReferenceHandle;

typedef enum SizeScalingType {
	SIZE_SCALING_TYPE_FIXED = 0,
	SIZE_SCALING_TYPE_MULTIPLY = 1,
	SIZE_SCALING_TYPE_DIVIDE_BY_EXP2 = 2
}SizeScalingType;

typedef struct ImageDataDescription {
	ImageReference* imageReference;
	ImageFormat imageFormat;
	ImageUsageFlags imageUsage;
	ImageAspectFlags imageAspect;
}ImageDataDescription;

typedef struct ImageResourceDescription {
	ImageReference* imageReference;
	ImageFormat imageFormat;
}ImageResourceDescription;

typedef struct ImageSubResourceDescription {
	ImageReference* imageReference;
	ImageFormat imageFormat;
	uint32_t width;
	uint32_t height;
};

typedef struct ImageStackLayout {

	uint32_t imageCount;
	ImageDataDescription* imageDescriptions;

	ImageSampleCount sampleCount;

	uint32_t width;
	uint32_t height;

	ImageStack* imageStack;

} ImageStackLayout;

typedef struct ImagePoolCreateInfo {
	uint32_t imageStackCount;
	ImageStackLayout* imageStacks;
} ImagePoolCreateInfo;

void imagePoolCreate(EngineHandle handle, ImagePoolCreateInfo info, ImagePool* pool);
void imagePoolDestroy(ImagePool pool);

#define IMAGE_REF_COUNT(imageRefArray) (sizeof(imageRefArray)/sizeof(ImageReference))
#define IMAGE_DATA_COUNT(imageDescrArray) (sizeof(imageDescrArray)/sizeof(ImageDataDescription))
#define IMAGE_STACK_LAYOUT_COUNT(imageStackArray) (sizeof(imageStackArray)/sizeof(ImageStackLayout))

#pragma endregion

typedef enum RenderChainNodeType {
	RENDER_NODE_OUTPUT,
	RENDER_NODE_RENDER,
	RENDER_NODE_POST_PROCESS,
	RENDER_NODE_ENTRY
}RenderChainNodeType;

typedef void* RenderChainNode;

typedef struct RenderChainOutput {
	RenderChainNodeType type;

	uint32_t exportImageCount;
	ImageReferenceHandle* exportImages;
}RenderChainOutput;

typedef struct RenderChainRenderModule {
	RenderChainNodeType type;
	uint32_t dependantCount;
	RenderChainNode* dependants;
	
	uint32_t outputImageCount;
	ImageReferenceHandle* outputImages;

	uint32_t inputImageCount;
	ImageReferenceHandle* inputImages;

	uint32_t shaderReadImageCount;
	ImageReferenceHandle* shaderReadImages;
	
	uint32_t shaderWriteImageCount;
	ImageReferenceHandle* shaderWriteImages;

	ImageSampleCount sampleCount;
	uint32_t viewportX;
	uint32_t viewportY;
	uint32_t viewportWidth;
	uint32_t viewportHeight;

	bool_t depthBuffered;
	ImageReferenceHandle depthImage;
	bool_t clearDepthBuffer;

}RenderChainRenderModule;

typedef struct RenderChainPostProcessModule {
	RenderChainNodeType type;
	uint32_t dependantCount;
	RenderChainNode* dependants;

	uint32_t outputImageCount;
	ImageReferenceHandle* outputImages;

	uint32_t inputImageCount;
	ImageReferenceHandle* inputImages;

	uint32_t shaderReadImageCount;
	ImageReferenceHandle* shaderReadImages;

	uint32_t shaderWriteImageCount;
	ImageReferenceHandle* shaderWriteImages;

	uint32_t viewportX;
	uint32_t viewportY;
	uint32_t viewportWidth;
	uint32_t viewportHeight;

}RenderChainPostProcessModule;

typedef struct RenderChainEntryPoint {
	RenderChainNodeType type;
	RenderChainNode module;

	uint32_t importImageCount;
	ImageReferenceHandle* importImages;
};

DEFINE_HANDLE(RenderChain);
DEFINE_HANDLE(RenderModule);

typedef struct RenderChainCreateInfo {
	RenderChainOutput output;

	uint32_t entryPointCount;
	RenderChainEntryPoint* entryPoints;

	uint32_t renderModuleCount;
	RenderChainNode** renderModules;


}RenderChainCreateInfo;

void renderChainCreate(EngineHandle handle, RenderChainCreateInfo info, RenderChain* renderChain);
void renderChainDestroy(RenderChain renderChain);

void buildGraphics(EngineHandle handle);
void destroyGraphics();

