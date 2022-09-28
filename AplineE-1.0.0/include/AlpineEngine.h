#pragma once
#include "dataTypes.h"

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


DEFINE_HANDLE(Scene);

#define COMPONENT(type, members) typedef struct type { \
									const uint32_t typeID; \
									uint32_t objectID; \
									members\
								 } type;\
								 enum { type##TypeID=__COUNTER__ };\
								 enum { type##TypeSize=sizeof(type) };


void engineRegisterComponentType_(EngineHandle handle, uint32_t typeID, size_t size);
#define engineRegisterComponentType(handle, type) engineRegisterComponentType_(handle,type##TypeID,type##TypeSize)

size_t engineGetComponentSize(EngineHandle handle, uint32_t componentTypeID);
uint32_t enigneGetComponentTypeIndex(EngineHandle handle, uint32_t componentTypeID);


typedef struct ComponentRef {
	uint32_t componentTypeID;
	uint32_t component;
	uint32_t componentID;
}ComponentRef;


typedef struct SceneCreateInfo {

	int a;
}SceneCreateInfo;

DEFINE_HANDLE(Entity);
DEFINE_HANDLE(EntityPrimitive);

void sceneCreate(EngineHandle handle, SceneCreateInfo info, Scene* scene);
void sceneDestroy(Scene scene);

void sceneCreateEntity(Entity* entity, Scene scene);
void sceneDestroyEntity(Entity entity, Scene scene);
uint32_t sceneGetEntityCount(Scene scene);


EntityPrimitive createEntityPrimitive();
void destroyEntityPrimitive(EntityPrimitive primitive);

uint32_t entityAddComponent(Entity entity, uint32_t componentTypeID, void* data);
bool_t entityHasComponent(Entity entity, uint32_t componentTypeID);
void entityRemoveComponent(Entity entity, uint32_t componetTypeID);

void buildComponentTypes(EngineHandle handle);
void buildScenes(EngineHandle handle);
