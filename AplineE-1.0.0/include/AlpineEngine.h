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

typedef struct ComponentRef {
	uint32_t componentTypeID;
	uint32_t component;
}ComponentRef;

typedef struct Entity {
	uint32_t ID;
	uint32_t componentCount;
	ComponentRef* components;
} Entity;

typedef struct SceneCreateInfo {

	int a;
}SceneCreateInfo;



void sceneCreate(EngineHandle handle, SceneCreateInfo info, Scene* scene);


void buildComponentTypes(EngineHandle handle);
void buildScenes(EngineHandle handle);




