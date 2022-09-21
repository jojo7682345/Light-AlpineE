#include "engine.h"
#include "../core/core.h"
#include <core/swapchain/swapchain.h>
uint32_t engineGetFps(EngineHandle handle) {
	return handle->system.fps;
}

uint32_t engineWindowGetWidth(EngineHandle handle) {
	return handle->system.windowWidth;
}

uint32_t engineWindowGetHeight(EngineHandle handle) {
	return handle->system.windowHeight;
}

bool_t engineWindowGetFullscreen(EngineHandle handle) {
	return handle->system.windowFullscreen;
}

uint32_t getEngineVersion() {
	return MAKE_VERSION(1, 0, 0);
}

uint32_t engineGetDisplayWidth(EngineHandle handle) {
	return handle->system.displayWidth;
}

uint32_t engineGetDisplayHeight(EngineHandle handle) {
	return handle->system.displayHeight;
}

bool_t engineFrameRateGetLimitEnabled(EngineHandle handle) {
	return handle->system.framerateLimit.max_fps;
}

uint32_t engineFrameRateGetLimitMaxFps(EngineHandle handle) {
	return handle->system.framerateLimit.max_fps;
}

void engineFrameRateLimitSet(EngineHandle handle, uint32_t maxFps, bool_t enabled) {
	handle->system.framerateLimit.max_fps = maxFps;
	handle->system.framerateLimit.enabled = enabled;
}

bool_t engineVsyncSet(EngineHandle handle, VsyncEnabled enabled) {
	
	if (!updateSwapchainVsyncSettings(handle, enabled)) {
		return VK_FALSE;
	}

	//TODO: update rendertarget;

	return VK_TRUE;
}

bool_t engineVsyncGetEnabled(EngineHandle handle) {
	return handle->system.vsyncEnabled;
}

uint32_t engineGpuGetMaxImageWidth(EngineHandle handle) {
	return handle->system.maxImageWidth;
}

uint32_t engineGpuGetMaxImageHeight(EngineHandle handle) {
	return handle->system.maxImageHeight;
}

const char* engineGpuGetName(EngineHandle handle) {
	return handle->system.gpuName;
}

ImageFormat engineGetColorImageFormat(EngineHandle handle) {
	return handle->system.colorImageFormat;
}

ImageFormat engineGetDepthImageFormat(EngineHandle handle) {
	return handle->system.depthImageFormat;
}

