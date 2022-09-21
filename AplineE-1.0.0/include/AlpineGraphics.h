#pragma once
#include "AlpineEngine.h"

#pragma region ImageStack & ImagePool

DEFINE_HANDLE(ImageReference);
DEFINE_HANDLE(ImageStack);
DEFINE_HANDLE(ImagePool);

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

DEFINE_HANDLE(Image);
typedef Image* ImageRef;

void imageAllocateHandle(ImageRef ref);

typedef enum ShaderImageState {
	SHADER_IMAGE_STATE_READ_ONLY,
	SHADER_IMAGE_STATE_STORAGE
}ShaderImageState;

typedef struct ShaderImageTrack {
	uint32_t imageCount;
	ImageRef* images;
	uint32_t moduleCount;
	ShaderImageState* imageStates;
}ShaderImageTrack;

typedef enum ImageAccessType {
	IMAGE_ACCESS_TYPE_UNDEFINED,
	IMAGE_ACCESS_TYPE_SHADER_IMAGE_TRACK,
	IMAGE_ACCESS_TYPE_RENDER_INPUT,
	IMAGE_ACCESS_TYPE_RENDER_OUTPUT,
}ImageAccessType;

typedef struct ImageResourceDescription {
	ImageRef image;
	ImageAccessType initialAccessType;
	ImageAccessType finalAccessType;

}ImageResourceDescription;
typedef struct ImageDepthResourceDescription {
	ImageRef image;
	ImageAccessType initialAccessType;
	ImageAccessType finalAccessType;
	ImageSampleCount sampleCount;
}ImageDepthResourceDescription;

DEFINE_HANDLE(RenderModule);

typedef enum RenderModuleType {
	RENDER_MODULE_TYPE_RENDER,
	RENDER_MODULE_TYPE_POST_PROCESS
}RenderModuleType;

typedef struct RenderModuleDescription {

	RenderModule* handle;
	RenderModuleType type;

	uint32_t outputImageCount;
	ImageRef* outputImages;

	uint32_t inputImageCount;
	ImageRef* inputImages;

	uint32_t preserveImageCount;
	ImageRef* preserveImages;

	ImageRef depthImage;
	ImageSampleCount sampleCount;

}RenderModuleDescription;

typedef enum RenderModuleStage {
	RENDER_MODULE_STAGE_FRAGMENT_OUTPUT,
	RENDER_MODULE_STAGE_FRAGMENT_INPUT,
	RENDER_MODULE_STAGE_DEPTH_OUTPUT,
	RENDER_MODULE_STAGE_DEPTH_INPUT,
	RENDER_MODULE_STAGE_VERTEX_INPUT,
	RENDER_MODULE_STAGE_VERTEX_OUTPUT,
}RenderModuleStage;

#define RENDERER_OUTPUT ((RenderModule*)0x0001)
#define RENDERER_INPUT ((RenderModule*)0x0002)

typedef struct RenderModuleDependency {
	RenderModule* src;
	RenderModule* dst;
	RenderModuleStage srcStage;
	RenderModuleStage dstStage;
	RenderModuleAccessFlags srcAccess;
	RenderModuleAccessFlags dstAccess;
}RenderModuleDependency;

DEFINE_HANDLE(Renderer);

typedef struct RendererCreateInfo {

	uint32_t width; //0 for window width;
	uint32_t height; //0 for window height;

	uint32_t imageResourceCount;
	ImageResourceDescription* imageResources;

	uint32_t depthImageResourceCount;
	ImageDepthResourceDescription* depthImageResources;

	uint32_t renderModuleCount;
	RenderModuleDescription* renderModules;

	uint32_t dependencyCount;
	RenderModuleDependency* dependencies;

}RendererCreateInfo;

void rendererCreate(EngineHandle handle, RendererCreateInfo info, Renderer* renderer);
void rendererDestroy(Renderer renderer);


void buildGraphics(EngineHandle handle);
void destroyGraphics();

