//#include <engine/engine.h>
//#include <core/core.h>
//#include <util/util.h>
//
//
//void renderSurfaceSetHidden(EngineHandle handle, RenderSurface surface, bool_t visible) {
//
//}
//
//bool_t renderSurfaceGetHidden(EngineHandle handle, RenderSurface surface) {
//
//
//	return surface->hidden;
//}
//
//typedef enum RenderSurfaceTypeExt {
//	RENDER_SURFACE_TYPE_APPLICATION_SURFACE = 2,
//	RENDER_SURFACE_TYPE_APPLICATION_SURFACE_UI = 3
//}RenderSurfaceTypeExt;
//
//
//uint32_t renderSurfaceGetWidth(RenderSurface surface) {
//	return surface->renderExtent.width;
//}
//
//uint32_t renderSurfaceGetHeight(RenderSurface surface) {
//	return surface->renderExtent.height;
//}
//
//void renderSurfaceEnumerateDependants(RenderSurface surface, uint32_t* count, RenderSurface** dependencies) {
//
//}
//
//void renderSurfaceSetSize(EngineHandle handle, RenderSurface surface, uint32_t width, uint32_t height) {
//
//}
//
//void renderSurfaceSetDisplayPort(EngineHandle handle, RenderSurface surface, uint32_t x, uint32_t y, uint32_t widht, uint32_t height) {
//
//}
//
////void renderSurfaceAllocateDefaults(EngineHandle handle) {
////	handle->renderSurfaceCount = 2;
////	handle->renderSurfaces = (RenderSurface_T*)fsAllocate(sizeof(RenderSurface_T) * handle->renderSurfaceCount);
////
////	handle->applicationSurface = &handle->renderSurfaces[0];
////	handle->applicationGuiSurface = &handle->renderSurfaces[1];
////}
////
////
////void renderSurfaceFreeDefaults(EngineHandle handle) {
////	fsFree(handle->renderSurfaces);
////}
//
//RenderSurfaceCreateInfo convertApplicationSurfaceSettingsToSurfaceCreateInfo(EngineHandle handle, RenderSurfaceApplicationSurfaceSettings appSurfSettings) {
//	RenderSurfaceCreateInfo appSurfCreateInfo = { 0 };
//	appSurfCreateInfo.depth = appSurfSettings.depth;
//
//	appSurfCreateInfo.width = appSurfSettings.width;
//	appSurfCreateInfo.height = appSurfSettings.height;
//	appSurfCreateInfo.displayHeight = appSurfSettings.displayHeight;
//	appSurfCreateInfo.displayWidth = appSurfSettings.displayWidth;
//	appSurfCreateInfo.displayX = appSurfSettings.displayX;
//	appSurfCreateInfo.displayY = appSurfSettings.displayY;
//
//	appSurfCreateInfo.renderSurface = &handle->applicationSurface;
//	appSurfCreateInfo.type = RENDER_SURFACE_TYPE_APPLICATION_SURFACE;
//
//	uint32_t dependencyCount = appSurfSettings.dependencyCount + 1;
//	RenderSurface** dependencies = (RenderSurface**)fsAllocate(dependencyCount * sizeof(RenderSurface*));
//	dependencies[0] = &handle->applicationGuiSurface;
//	for (uint32_t i = 1; i < dependencyCount; i++) {
//		dependencies[i] = appSurfSettings.dependencies[i - 1];
//	}
//	appSurfCreateInfo.dependencyCount = dependencyCount;
//	appSurfCreateInfo.dependencies = dependencies;
//	
//	return appSurfCreateInfo;
//}
//
//RenderSurfaceCreateInfo convertApplicationSurfaceUiSettingsToSurfaceCreateInfo(EngineHandle handle, RenderSurfaceApplicationSurfaceUiSettings appSurfUiSettings) {
//	RenderSurfaceCreateInfo appSurfCreateInfo = { 0 };
//	appSurfCreateInfo.depth = appSurfUiSettings.depth;
//
//	appSurfCreateInfo.width = appSurfUiSettings.width;
//	appSurfCreateInfo.height = appSurfUiSettings.height;
//	appSurfCreateInfo.displayHeight = appSurfUiSettings.displayHeight;
//	appSurfCreateInfo.displayWidth = appSurfUiSettings.displayWidth;
//	appSurfCreateInfo.displayX = appSurfUiSettings.displayX;
//	appSurfCreateInfo.displayY = appSurfUiSettings.displayY;
//
//	appSurfCreateInfo.renderSurface = &handle->applicationGuiSurface;
//	appSurfCreateInfo.type = RENDER_SURFACE_TYPE_APPLICATION_SURFACE_UI;
//
//	appSurfCreateInfo.dependencies = appSurfUiSettings.dependencies;
//	appSurfCreateInfo.dependencyCount = appSurfUiSettings.dependencyCount;
//
//	return appSurfCreateInfo;
//}
//
//RenderSurfaceApplicationSurfaceSettings renderSurfaceGetDefaultApplicationSurfaceSettings() {
//	RenderSurfaceApplicationSurfaceSettings settings = { 0 };
//	settings.depth = 0;
//	settings.width = 0;
//	settings.height = 0;
//	settings.displayX = 0;
//	settings.displayY = 0;
//	settings.displayWidth = 0;
//	settings.displayHeight = 0;
//	return settings;
//}
//
//RenderSurfaceApplicationSurfaceSettings renderSurfaceGetDefaultApplicationSurfaceUiSettings() {
//	RenderSurfaceApplicationSurfaceSettings settings = { 0 };
//	settings.depth = (uint32_t)-1;
//	settings.width = 0;
//	settings.height = 0;
//	settings.displayX = 0;
//	settings.displayY = 0;
//	settings.displayWidth = 0;
//	settings.displayHeight = 0;
//	return settings;
//}
//
//
