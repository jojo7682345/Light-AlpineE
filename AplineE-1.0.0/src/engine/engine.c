#include <AlpineEngine.h>
#include "engine.h"
#include "../core/core.h"
#include <core/swapchain/swapchain.h>

#pragma region system stuff
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

void engineRegisterComponentType_(EngineHandle handle, uint32_t typeID, size_t size) {
	ComponentTypeDesctription typeDescription = { .typeID = typeID, .size = size };
	uint32_t index = dynamicArrayAdd(&typeDescription, &handle->componentTypes);
	dynamicArraySet(index, &typeID, &handle->componentTypeMap);
}

size_t engineGetComponentSize(EngineHandle handle, uint32_t componentTypeID) {
	uint32_t index = enigneGetComponentTypeIndex(handle, componentTypeID);
	size_t size = 0;
	if (index == -1) {
		abort();
		return 0;
	}
	dynamicArrayGet(index, &size, &handle->componentTypes);
	return size;
}

uint32_t enigneGetComponentTypeIndex(EngineHandle handle, uint32_t componentTypeID) {
	return dynamicArrayFind(&componentTypeID, &handle->componentTypeMap);
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

#pragma endregion

void sceneCreate(EngineHandle handle, SceneCreateInfo info, Scene* pScene) {
	Scene scene = fsAllocate(sizeof(Scene_T));
	*pScene = scene;
	dynamicArrayAdd(&scene, &handle->scenes);
	scene->handle = handle;
	
	dynamicArrayCreate(sizeof(Entity), &scene->entities);

	uint32_t count = handle->componentTypeCount;
	scene->componentTypeGroups = fsAllocate(sizeof(_DynamicArray) * count);
	for (uint32_t i = 0; i < count; i++) {
		ComponentTypeDesctription typeDescription;
		dynamicArrayGet(i, &typeDescription, &handle->componentTypes);
		_DynamicArray arr = { .dataSize = sizeof(typeDescription.size) };
		memcpy(scene->componentTypeGroups + i, &arr, sizeof(_DynamicArray));
	}
	
}

void sceneDestroy(Scene scene) {
	uint32_t entityCount = dynamicArrayCountPopulated(&scene->entities);
	Entity* entities = fsAllocate(sizeof(Entity) * entityCount);
	dynamicArrayExtractPopulated(entities, &scene->entities);
	for (uint32_t i = 0; i < entityCount; i++) {
		sceneDestroyEntity(entities[i], scene);
	}
	dynamicArrayDestroy(&scene->entities);

	dynamicArrayRemove(dynamicArrayFind(&scene, &scene->handle->scenes), &scene->handle->scenes);
	for (uint32_t i = 0; i < scene->handle->componentTypeCount; i++) {
		dynamicArrayDestroy(scene->componentTypeGroups + i);
	}
	


	fsFree(scene->componentTypeGroups);
	fsFree(scene);
}

void sceneCreateEntity(Entity* entity, Scene scene) {
	Entity entt = fsAllocate(sizeof(Entity_T));
	uint32_t index = dynamicArrayAdd(entt, &scene->entities);
	entt->ID = index;
	entt->scene = scene;
	dynamicArrayCreate(sizeof(ComponentRef), &entt->components);

	*entity = entt;
}

void sceneDestroyEntity(Entity entity, Scene scene) {
	dynamicArrayRemove(dynamicArrayFind(entity,&scene->entities), &scene->entities);
	dynamicArrayDestroy(&entity->components);
	fsFree(entity);
}

uint32_t sceneGetEntityCount(Scene scene) {
	return dynamicArrayCountPopulated(&scene->entities);
}

EntityPrimitive createEntityPrimitive() {
	return fsAllocate(sizeof(EntityPrimitive_T));
}

void destroyEntityPrimitive(EntityPrimitive primitive) {
	fsFree(primitive);
}

uint32_t entityAddComponent(Entity entity, uint32_t componentTypeID, void* data) {
	size_t componentSize = engineGetComponentSize(entity->scene->handle, componentTypeID);
	
	uint32_t componentIndex = dynamicArrayAdd(data, entity->scene->componentTypeGroups);

		return 0;
}

bool_t entityHasComponent(Entity entity, uint32_t componentTypeID) {
	return 1;
}

void entityRemoveComponent(Entity entity, uint32_t componetTypeID) {}

void registerEngineDefaultComponentTypes(EngineHandle handle) {
	engineRegisterComponentType(handle, EmptyComponent);
	engineRegisterComponentType(handle, TransformComponent);

}
