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

DEFINE_HANDLE(ImageHandle);

typedef enum ImageType {
	IMAGE_TYPE_DEPTH,
	IMAGE_TYPE_COLOR,
}ImageType;

typedef struct ImageAttachment {
	ImageFormat format;
	ImageHandle* imageHandle;
	ImageType type;
}ImageAttachment;

typedef struct ImageAttachmentRef {
	uint32_t attachment;
}ImageAttachmentRef;

#define ATTACHMENT_UNUSED ((uint32_t)-1)

DEFINE_HANDLE(RenderModule);

typedef struct RenderModuleDescription {
	RenderModule* handle;

	uint32_t colorAttachmentCount;
	ImageAttachmentRef* colorAttachments;

	uint32_t inputAttachmentCount;
	ImageAttachmentRef* inputAttachments;

	uint32_t shaderStoreAttachmentCount;
	ImageAttachmentRef* shaderStoreAttachments;
	
	uint32_t shaderSampleAttachmentCount;
	ImageAttachmentRef* shaderSampleAttachments;

	uint32_t depthAttachment;

	ImageSampleCount sampleCount;
};

typedef enum RenderModuleOutput {
	FRAGMENT_OUTPUT = 0xff000000U - 0x00000000U,
	VERTEX_OUTPUT = 0xff000000U - 0x01000000U
};
typedef enum RenderModuleInput {
	FRAGMENT_INPUT = 0x00ff0000U - 0x00000000U,
	VERTEX_INPUT = 0x00ff0000U - 0x00010000U
}RenderModuleInput;

typedef struct Renderer {
	uint32_t width;
	uint32_t height;

	uint32_t attachmentCount;
	ImageAttachment* attachments;
};


void buildGraphics(EngineHandle handle);
void destroyGraphics();

