#pragma once
#include "AlpineImageFormats.h"

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

typedef struct ImageResourceDescription {
	ImageReference* imageReference;
	ImageFormat imageFormat;
}ImageResourceDescription;

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


DEFINE_HANDLE(RenderModule);
DEFINE_HANDLE(ComputeModule);

typedef enum RenderModuleType {
	RENDER_MODULE_TYPE_RENDER,
	RENDER_MODULE_TYPE_POST_PROCESS,
}RenderModuleType;

typedef enum ComputeModuleType {
	COMPUTE_MODULE_TYPE_COMPUTE,
	COMPUTE_MODULE_TYPE_POST_PROCESS
} ComputeModuleType;

typedef struct RenderModuleDescription {
	RenderModuleType type;
	RenderModule* handle;

	ImageSampleCount sampleCount;

	uint32_t colorOutputCount;
	ImageResourceDescription* colorOutputImages;
	uint32_t colorInputCount;
	ImageResourceDescription* colorInputImages;

	bool_t depthBuffered;
	ImageReference* depthImage;

}RenderModuleDescription;

typedef struct ComputeModuleDescription {
	ComputeModuleType type;
	ComputeModule* handle;

} ComputeModuleDescription;

typedef enum ImageAccessType {
	IMAGE_ACCESS_TYPE_INPUT_ATTACHMENT,
	IMAGE_ACCESS_TYPE_OUTPUT_ATTACHMENT,
	IMAGE_ACCESS_TYPE_SHADER_READ,
	IMAGE_ACCESS_TYPE_SHADER_WRITE,
}ImageAccessType;

typedef enum ImageType {
	IMAGE_TYPE_COLOR,
	IMAGE_TYPE_DEPTH,
}ImageType;

typedef enum ResouceType {
	RESOURCE_TYPE_COLOR_IMAGE,
	RESOURCE_TYPE_BUFFER
}ResouceType;

typedef enum ExportStageFlag {
	EXPORT_STAGE_FRAGMENT_OUTPUT = 1 << 0,
	EXPORT_STAGE_VERTEXT_OUTPUT = 1 << 1,
	EXPORT_STAGE_DEPTH_OUTPUT = 1 << 2,
}ExportStageFlag;
typedef uint32_t ExportStage;

typedef enum ImportStageFlag {
	IMPORT_STAGE_VERTEX_SHADER_INPUT = 1 << 0,
	IMPORT_STAGE_FRAGMENT_SHADER_INPUT = 1 << 1,
}ImportStageFlag;
typedef uint32_t ImportStage;

typedef struct RenderModuleDependency {
	RenderModule* source;
	RenderModule* destination;

	ImageAccessType srcAccess;
	ImageAccessType dstAccess;

	ExportStage srcStage;
	ImportStage dstStage;

	uint32_t resourceCount;
	void* resources;
}RenderModuleDependency;

#define EXPORT_MODULE_EXTERNAL ((void*)0x00)
#define IMPORT_MODULE_EXTERNAL ((void*)0x00)

typedef struct RenderModuleExport {
	ResouceType resourceType;
	ExportStage exportStage;
	uint32_t resourceCount;
	void* usedResources;
	RenderModule* exportModule;
	ComputeModule* importModule;
}RenderModuleExport;

typedef struct RenderModuleImport {
	ResouceType resourceType;
	ImportStage importStage;
	uint32_t resourceCount;
	void* usedResources;
	RenderModule* importModule;
	ComputeModule* exportModule;
}RenderModuleImport;

DEFINE_HANDLE(Renderer);
DEFINE_HANDLE(RenderChain);

#define IMAGE_SIZE_SCREEN_SIZE 0

typedef struct RendererCreateInfo {

	uint32_t preRenderComputeModuleCount;
	ComputeModuleDescription* preRenderComputeModules;

	uint32_t renderModuleResourceImportCount;
	RenderModuleImport* renderModuleResourceImports;

	uint32_t imageWidth; // the height of the image rendered to, 0 for the size of the window
	uint32_t imageHeight; // the width of the image rendered to, 0 for the size of the window
	uint32_t renderModuleCount;
	RenderModuleDescription* renderModules;
	uint32_t dependencyCount;
	RenderModuleDependency* dependencies;

	uint32_t renderModuleResourceExportCount;
	RenderModuleExport* renderModuleResourceExports;
	
	uint32_t postRenderComputeModuleCount;
	ComputeModuleDescription* postRenderComputeModules;

} RendererCreateInfo;

void rendererCreate(EngineHandle handle, RendererCreateInfo info, Renderer* renderer);
void rendererDestroy(Renderer renderer);

void buildGraphics(EngineHandle handle);
void destroyGraphics();

