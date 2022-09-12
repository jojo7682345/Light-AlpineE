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

typedef struct ImageSize {
	//VkExtent2D
	uint32_t width;
	uint32_t height;
	//

	SizeScalingType widthScalingType;
	SizeScalingType heightScalingType;
	uint8_t widthScaling;
	uint8_t heightScaling;
}ImageSize;

typedef struct ImageStackLayout {

	uint32_t imageCount;
	ImageDataDescription* imageDescriptions;

	ImageSampleCount sampleCount;

	ImageSize size;

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

#pragma region RenderModules

DEFINE_HANDLE(RenderModule);

typedef enum RenderModuleType {
	RENDER_MODULE_TYPE_GEOMETRY,
	RENDER_MODULE_TYPE_IMAGE_COMBINE,
	RENDER_MODULE_TYPE_POST_PROCESS,
}RenderModuleType;

#pragma region RenderModule Input Types
typedef struct DirectInputImages {
	ImageStack directInputStack;
	uint32_t directInputImageCount;
	ImageReference* directInputImages;
}DirectInputImages;

typedef struct RenderModuleOutput {
	ImageStack outputStack;
	uint32_t outputImageCount;
	ImageReference* outputImages;
}RenderModuleOutput;

typedef struct ShaderInputImages {
	uint32_t shaderInputImageCount;
	ImageReference* shaderInputImages;
} ShaderInputImages;

typedef struct ShaderOutputImages {
	uint32_t shaderOutputImageCount;
	ImageReference* shaderOutputImages;
} ShaderOutputImages;
#pragma endregion


typedef struct RenderModuleGeometryRenderInfo {
	ImageReference depthImage;
	ImageReference* resolveImages;
}RenderModuleGeometryRenderInfo;

typedef EMPTY_STRUCT RenderModuleImageCombineInfo;
typedef EMPTY_STRUCT RenderModulePostProcessInfo;

typedef struct RenderModuleCreateInf {
	RenderModuleType type;
	DirectInputImages directInputImages;
	ShaderInputImages shaderInputImages;
	ShaderOutputImages shaderOutputImages;
	RenderModuleOutput output;

	union {
		RenderModuleGeometryRenderInfo geometry;
		RenderModuleImageCombineInfo imageCombine;
		RenderModulePostProcessInfo postProcess;
	};

} RenderModuleCreateInfo;

void renderModuleCreate(EngineHandle handle, RenderModuleCreateInfo info, RenderModule* renderModule);
void renderModuleDestroy(RenderModule renderModule);

#pragma endregion

#pragma region ComputeModules

DEFINE_HANDLE(ComputeModule);

typedef enum ComputeModuleType {
	COMPUTE_MODULE_TYPE_COMPUTE,
	COMPUTE_MODULE_TYPE_POST_PROCESS,
	COMPUTE_MODULE_TYPE_IMAGE_COMBINE
} ComputeModuleType;

typedef struct ComputeModuleComputeInfo {
	uint32_t groupCountX;
	uint32_t groupCountY;
	uint32_t groupCountZ;
}ComputeModuleComputeInfo;
typedef EMPTY_STRUCT ComputeModulePostProcessInfo;
typedef EMPTY_STRUCT ComputeModuleImageCombineInfo;

typedef struct ComputeModuleCreateInfo {
	ComputeModuleType type;
	ShaderInputImages shaderInputImages;
	ShaderOutputImages shaderOutputImages;

	union {
		ComputeModuleComputeInfo compute;
		ComputeModuleImageCombineInfo imageCombine;
		ComputeModulePostProcessInfo postProcess;
	};

} ComputeModuleCreateInfo;

void computeModuleCreate(EngineHandle handle, ComputeModuleCreateInfo info, ComputeModule* computeModule);
void computeModuleDestroy(ComputeModule computeModule);

#pragma endregion

#pragma region RenderChain

DEFINE_HANDLE(RenderChain);

typedef struct RenderStepDependency {
	RenderModule src;
	RenderModule dst;
} RenderStepDependency;

typedef struct ComputeModuleDependency {
	ComputeModule src;
	ComputeModule dst;
}ComputeModuleDependency;

typedef struct ModuleDependency {
	void* src;
	void* dst;
} ModuleDependency;

typedef struct RenderOutput {
	uint32_t outputImageCount;
	ImageReference* outputImages;
}RenderOutput;

typedef struct RenderInput {
	uint32_t inputImageCount;
	ImageReference* inputImages;
}RenderInput;

typedef struct RenderChainCreateInfo {
	RenderInput input;

	uint32_t preRenderComputeModuleCount;
	ComputeModule* preRenderComputeModules;
	uint32_t preRenderDependencyCount;
	ComputeModuleDependency* preRenderDependencies;

	uint32_t renderStepCount;
	RenderModule* renderSteps;
	uint32_t renderStepDependencyCount;
	ImageSize renderStepImageSize;
	RenderStepDependency* renderStepDependencies;

	uint32_t postRenderComputeModuleCount;
	ComputeModule* postRenderComputeModules;
	uint32_t postRenderDependencyCount;
	ComputeModuleDependency* postRenderDependencies;

	RenderOutput output;
} RenderChainCreateInfo;

void renderChainCreate(EngineHandle handle, RenderChainCreateInfo info, RenderChain* renderChain);
void renderChainDestroy(RenderChain renderChain);

#pragma endregion

#pragma region GraphicsChain

DEFINE_HANDLE(GraphicsChain);





#pragma endregion

void buildGraphics(EngineHandle handle);
void destroyGraphics();

