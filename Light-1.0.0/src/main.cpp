
#pragma warning(push, 0)
#include <AlpineCore.h>
//#include <MemoryUtilities/MemoryUtilities.h>
#pragma warning(pop)

#include "handles.h"

EngineSettings getEngineSettings() {

	

	GpuSelectPreferences gpuSelect;
	gpuSelect.type = GPU_TYPE_SELECT_PREFER_DEDICATED;
	gpuSelect.scalers = gpuSelectPreferencesScalersGetDefault();

	EngineSettings settings{};
	settings.gameVersion = MAKE_VERSION(1, 0, 0);
	settings.gameTitle = "Light-1.0.0";
	settings.windowWidth = 640;
	settings.windowHeight = 360;
	settings.windowResizable = false;
	settings.windowFullscreen = false;
	settings.windowTitle = "Light-1.0.0";
	settings.gpuSelectPreference = gpuSelect;
	settings.fpsLimit = 0;
	settings.fpsLimitEnabled = false;
	settings.vsyncEnabled = VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE;
	settings.vsyncPreferences = VSYNC_SETTINGS_PREFER_TRIPLE_BUFFERED_BUT_REQUIRE_DOUBLE_BUFFERED;

	return settings;
}


ImageReference albedo;
ImageReference position;
ImageReference specular;
ImageReference normal;

ImageReference depth;

ImageReference uiRender;

ImageReference deferedRender;
ImageReference transparentRender;


RenderModule mainRendering;
RenderModule uiRendering;

RenderModule deferedPass;
RenderModule transparentPass;

ComputeModule uiOverlay;

Renderer deferedRenderer;

RenderChain renderChain;

void buildGraphics(EngineHandle handle) {

	ImageResourceDescription mainGeometryImages[] = {
		{ &albedo, IMAGE_FORMAT_B8G8R8A8_SRGB },
		{ &normal, IMAGE_FORMAT_B8G8R8A8_SRGB },
		{ &position, IMAGE_FORMAT_B8G8R8A8_SRGB },
		{ &specular, IMAGE_FORMAT_R32_SFLOAT }
	};

	RenderModuleDescription mainGeometryDescription = {};
	mainGeometryDescription.type = RENDER_MODULE_TYPE_RENDER;
	mainGeometryDescription.handle = &mainRendering;
	mainGeometryDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;
	mainGeometryDescription.colorOutputCount = 4;
	mainGeometryDescription.colorOutputImages = mainGeometryImages;
	mainGeometryDescription.depthBuffered = true;
	mainGeometryDescription.depthImage = &depth;

	ImageResourceDescription uiRenderImages[] = {
		{ &uiRender, IMAGE_FORMAT_B8G8R8A8_SRGB }
	};

	RenderModuleDescription uiRenderDescription{};
	uiRenderDescription.type = RENDER_MODULE_TYPE_RENDER;
	uiRenderDescription.handle = &uiRendering;
	uiRenderDescription.sampleCount = IMAGE_SAMPLE_COUNT_4;
	mainGeometryDescription.colorOutputCount = 1;
	mainGeometryDescription.colorOutputImages = uiRenderImages;;
	mainGeometryDescription.depthBuffered = true;

	ImageResourceDescription deferedPassImages[] = {
		{ &deferedRender, IMAGE_FORMAT_B8G8R8A8_SRGB }
	};

	RenderModuleDescription deferedPassDescription{};
	deferedPassDescription.type = RENDER_MODULE_TYPE_POST_PROCESS;
	deferedPassDescription.handle = &deferedPass;
	deferedPassDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;
	deferedPassDescription.colorOutputCount = 1;
	deferedPassDescription.colorOutputImages = deferedPassImages;

	ImageResourceDescription transparentPassImages[] = {
		{ &transparentRender, IMAGE_FORMAT_B8G8R8A8_SRGB }
	};

	RenderModuleDescription transparentPassDescription{};
	transparentPassDescription.type = RENDER_MODULE_TYPE_RENDER;
	transparentPassDescription.handle = &transparentPass;
	transparentPassDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;
	transparentPassDescription.colorOutputCount = 1;
	transparentPassDescription.colorOutputImages = transparentPassImages;
	transparentPassDescription.depthBuffered = true;
	transparentPassDescription.depthImage = &depth;

	ComputeModuleDescription uiOverlayDescription{};
	uiOverlayDescription.type = COMPUTE_MODULE_TYPE_POST_PROCESS;
	uiOverlayDescription.handle = &uiOverlay;

	RenderModuleExport mainRenderExport{};
	mainRenderExport.resourceType = RESOURCE_TYPE_COLOR_IMAGE;
	mainRenderExport.exportStage = EXPORT_STAGE_FRAGMENT_OUTPUT;
	mainRenderExport.resourceCount = 1;
	mainRenderExport.exportModule = &transparentPass;
	mainRenderExport.importModule = &uiOverlay;

	RenderModuleExport uiRenderExport{};
	uiRenderExport.resourceType = RESOURCE_TYPE_COLOR_IMAGE;
	uiRenderExport.exportStage = EXPORT_STAGE_FRAGMENT_OUTPUT;
	uiRenderExport.resourceCount = 1;
	uiRenderExport.usedResources = uiRenderImages;
	uiRenderExport.exportModule = &uiRendering;
	uiRenderExport.importModule = &uiOverlay;

	RenderModuleExport exports[] = {
		mainRenderExport,
		uiRenderExport
	};

	RendererCreateInfo rendererCreateInfo{};
	rendererCreateInfo.preRenderComputeModuleCount = 0;
	rendererCreateInfo.preRenderComputeModules = nullptr;

	rendererCreateInfo.renderModuleResourceImportCount = 0;
	rendererCreateInfo.renderModuleResourceImports = nullptr;

	rendererCreateInfo.imageWidth = 0;
	rendererCreateInfo.imageHeight = 0;
	rendererCreateInfo.renderModuleCount = 0;
	rendererCreateInfo.renderModules = nullptr;

	rendererCreateInfo.renderModuleResourceExportCount = 0;
	rendererCreateInfo.renderModuleResourceExports = nullptr;

	rendererCreateInfo.postRenderComputeModuleCount = 0;
	rendererCreateInfo.postRenderComputeModules = nullptr;


	rendererCreate(handle, rendererCreateInfo, &deferedRenderer);


	renderChainCreate(handle, renderChainCreateInfo, &renderChain);
}


void destroyGraphics() {

	rendererDestroy(deferedRenderer);

}





