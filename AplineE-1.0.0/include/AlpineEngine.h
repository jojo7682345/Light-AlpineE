#pragma once

//enviroment functions
uint32_t engineGetDisplayWidth(EngineHandle handle);
uint32_t engineGetDisplayHeight(EngineHandle handle);

//engine functions
bool_t engineFrameRateGetLimitEnabled(EngineHandle handle);
uint32_t engineFrameRateGetLimitMaxFps(EngineHandle handle);
void engineFrameRateLimitSet(EngineHandle handle, uint32_t maxFps, bool_t enabled);
bool_t engineVsyncSet(EngineHandle handle, VsyncEnabled enabled);
bool_t engineVsyncGetEnabled(EngineHandle handle);

uint32_t engineGpuGetMaxImageWidth(EngineHandle handle);
uint32_t engineGpuGetMaxImageHeight(EngineHandle handle);
const char* engineGpuGetName(EngineHandle handle);

ImageFormat engineGetColorImageFormat(EngineHandle handle);
ImageFormat engineGetDepthImageFormat(EngineHandle handle);


//volatile functions
uint32_t engineGetFps(EngineHandle handle);
uint32_t engineWindowGetWidth(EngineHandle handle);
uint32_t engineWindowGetHeight(EngineHandle handle);
bool_t engineWindowGetFullscreen(EngineHandle handle);
