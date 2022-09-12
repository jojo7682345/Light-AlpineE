#pragma once


DECL_HANDLE(RenderSurface,
			VkExtent2D renderExtent;

const VkExtent2D imageSize;
const VkOffset2D imageOffset;

uint32_t displayWidth; 
uint32_t displayHeight;
uint32_t displayX;
uint32_t displayY;

const bool_t unstable;
bool_t hidden;

);

//void renderSurfaceAllocateDefaults(EngineHandle handle);
//void renderSurfaceFreeDefaults(EngineHandle handle);


