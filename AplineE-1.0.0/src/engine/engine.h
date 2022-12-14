#pragma once

#ifndef __ENGINE__
#define __ENGINE__

#include <vulkan/vulkan.h>
#include <AlpineCore.h>


DECL_HANDLE(GraphicsPipeline,
			int a;
);

typedef struct FrameRateLimit {
	VkBool32 enabled;
	uint32_t max_fps;
} FrameRateLimit;

typedef struct EngineStatus {
	uint32_t fps;
} EngineStatus;

typedef enum VsyncCapabilities {
	VSYNC_CAPABILITIES_NOT_AVAILABLE = 0b000,
	VSYNC_CAPABILITIES_DOUBLE_BUFFERED = 0b001,
	VSYNC_CAPABILITIES_TRIPPLE_BUFFERED = 0b010,
	VSYNC_CAPABILITIES_LAZY_DOUBLE_BUFFERED = 0b100
}VsyncCapabilities;
typedef uint32_t VcyncCapabilityFlags;

typedef struct GpuProperties {

	uint32_t maxImageWidth;
	uint32_t maxImageHeight;

	const char* gpuName;

}GpuProperties;

typedef struct EngineProperties {

	//envoroment properties
	uint32_t displayWidth;
	uint32_t displayHeight;


	//system properties
	VcyncCapabilityFlags vsyncCapabilities;
	VkFormat imageFormat;
	GpuProperties;

	//settings
	FrameRateLimit framerateLimit;
	VsyncEnabled vsyncEnabled;

	//---VARIABLE---
	//window
	uint32_t windowWidth;
	uint32_t windowHeight;
	bool_t windowFullscreen;


	//volatile
	uint32_t fps;
	long double deltaT;


}EngineProperties;
#endif // !__ENGINE__



