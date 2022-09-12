#pragma once
#include <vulkan/vulkan.h>
#include <AlpineCore.h>

typedef struct RenderTargetSpaceAllocation {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t renderSurfaceIndex;
} RenderTargetSpaceAllocation;

typedef struct RenderTree_T {
	uint32_t renderPassCount;
	VkRenderPass* renderPasses;

	const uint32_t indexSize;
	const bool_t* treeIndex;

} RenderTree_T;

void renderTreesBuild(EngineHandle handle);