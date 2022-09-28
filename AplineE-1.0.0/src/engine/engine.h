#pragma once

#ifndef __ENGINE__
#define __ENGINE__

#include <vulkan/vulkan.h>
#include <AlpineCore.h>
#include <MemoryUtilities/DynamicArray.h>
#include <AlpineEngine.h>

#pragma region system stuff

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
	VkFormat colorImageFormat;
	VkFormat depthImageFormat;
	bool_t depthImageFormatHasStencilComponent;
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

#pragma endregion

typedef struct ComponentTypeDesctription {
	uint32_t typeID;
	size_t size;
}ComponentTypeDesctription;

void registerEngineDefaultComponentTypes(EngineHandle handle);

typedef struct Scene_T {
	EngineHandle handle;

	_DynamicArray* componentTypeGroups;

	_DynamicArray entities;
	size_t entity;
}Scene_T;

typedef struct Entity_T {
	Scene scene;
	uint32_t ID;
	_DynamicArray components;
} Entity_T;

typedef struct EntityPrimitive_T {

	uint32_t* componentCount;
	uint32_t* componentTypes;
	void* componentData;

}EntityPrimitive_T;


#endif // !__ENGINE__



