#pragma once
#include <vulkan/vulkan.h>
#include <AlpineCore.h>
#include <engine/rendering/surface/renderSurface.h>

typedef enum RenderTargetType {
	RENDER_TARGET_TYPE_3D,
	RENDER_TARGET_TYPE_2D,
	RENDER_TARGET_TYPE_DISPLAY
} RenderTargetType;

//image structs
typedef struct RenderTargetImage {
	VkImage image;
	VkImageView imageView;
}RenderTargetImage;

typedef struct RenderTargetImageDepth {
	RenderTargetImage depth;
	VkClearColorValue depthClearColor;
}RenderTargetImageDepth;

typedef struct RenderTargetFrame {
	RenderTargetImage;
	VkFramebuffer buffer;
}RenderTargetFrame;

typedef struct RenderTargetImageData {
	VkFormat format;
	VkExtent2D extent;
	VkClearColorValue clearColor;
}RenderTargetImageData;

typedef struct RenderTargetSurfaceData {
	const uint32_t surfaceCount;
	const RenderSurface_T* surfaces;
}RenderTargetSurfaceData;

DEFINE_HANDLE(RenderTarget);
DEFINE_HANDLE(RenderTarget2D);
DEFINE_HANDLE(RenderTarget3D);
DEFINE_HANDLE(RenderTargetDisplay);

//render targets
typedef struct RenderTarget_T {
	RenderTargetType type;

	RenderTargetImageData;
}RenderTarget_T;

typedef struct RenderTarget2D_T {
	RenderTarget_T;

	VkDeviceMemory memory;
	RenderTargetImage;

	RenderTargetSurfaceData;

} RenderTarget2D_T;

typedef struct RenderTarget3D_T {
	RenderTarget2D_T;

	RenderTargetImageDepth;
	
}RenderTarget3D_T;


typedef struct RenderTargetDisplay_T {
	RenderTarget_T;

	uint32_t imageCount;
	RenderTargetFrame* swapchainImages;

	RenderTargetImageDepth;
	VkDeviceMemory memory;

	VkSwapchainKHR swapchain;

	RenderSurface_T surface;

}RenderTargetDisplay_T;

typedef struct RenderTargetCreateInfo {
	RenderTargetType type;
	uint32_t width;
	uint32_t height;
	VkClearColorValue clearColor;
	VkClearColorValue depthClearColor;
	uint32_t surfaceCount;
} RenderTargetCreateInfo;


void renderTargetCreateEngineDefaults(EngineHandle handle);

void renderTargetDestroyEngineDefaults(EngineHandle handle);

AeResult renderTargetDisplayCreate(EngineHandle handle);
AeResult renderTargetCreate(EngineHandle handle, RenderTargetCreateInfo info, RenderTarget_T* renderTarget);

void renderTargetDisplayDestroy(EngineHandle handle);

void renderTargetDestroy(EngineHandle handle, RenderTarget_T renderTarget);


