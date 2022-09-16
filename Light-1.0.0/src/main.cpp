
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

ImageReference subResource;

RenderChain renderChain;

void buildGraphics(EngineHandle handle) {

	ImageReferenceHandle outputImages[] = {
		&transparentRender,
		&uiRender,
		&subResource
	};
	RenderChainOutput output{};
	output.type = RENDER_NODE_OUTPUT;
	output.exportImageCount = sizeof(outputImages)/sizeof(ImageDataDescription);
	output.exportImages = outputImages;


	RenderChainNode transparentPassDependants[] = {
		&output
	};
	ImageReferenceHandle transparentPassOutputImages[] = {
		&transparentRender
	};
	ImageReferenceHandle deferedPassOutputImages[] = {
		&deferedRender
	};
	RenderChainRenderModule transparentPass{};
	transparentPass.type = RENDER_NODE_RENDER;
	transparentPass.dependantCount = sizeof(transparentPassDependants) / sizeof(RenderChainNode);
	transparentPass.dependants = transparentPassDependants;
	transparentPass.sampleCount = IMAGE_SAMPLE_COUNT_1;

	transparentPass.outputImageCount = sizeof(transparentPassOutputImages) / sizeof(ImageReferenceHandle);
	transparentPass.outputImages = transparentPassOutputImages;
	transparentPass.inputImageCount = sizeof(deferedPassOutputImages) / sizeof(ImageReferenceHandle);
	transparentPass.inputImages = deferedPassOutputImages;

	transparentPass.depthBuffered = true;
	transparentPass.depthImage = &depth;
	transparentPass.clearDepthBuffer = false;


	RenderChainNode deferedPassDependants[] = {
		&transparentPass
	};

	ImageReferenceHandle mainPassOutputImages[] = {
		&albedo,
		&position,
		&specular,
		&normal
	};
	RenderChainPostProcessModule deferedPass{};
	deferedPass.type = RENDER_NODE_POST_PROCESS;
	deferedPass.dependantCount = sizeof(deferedPassDependants) / sizeof(RenderChainNode);
	deferedPass.dependants = deferedPassDependants;
	deferedPass.outputImageCount = sizeof(deferedPassOutputImages) / sizeof(ImageReferenceHandle);
	deferedPass.shaderReadImageCount = sizeof(mainPassOutputImages) / sizeof(ImageReferenceHandle);
	deferedPass.shaderReadImages = mainPassOutputImages;


	RenderChainNode mainPassDependants[] = {
		&deferedPass
	};
	
	RenderChainRenderModule mainPass{};
	mainPass.type = RENDER_NODE_RENDER;
	mainPass.dependantCount = sizeof(mainPassDependants) / sizeof(RenderChainNode);
	mainPass.dependants = mainPassDependants;
	mainPass.sampleCount = IMAGE_SAMPLE_COUNT_1;
	mainPass.outputImageCount = sizeof(mainPassOutputImages) / sizeof(ImageReferenceHandle);
	mainPass.outputImages = mainPassOutputImages;
	mainPass.depthBuffered = true;
	mainPass.depthImage = &depth;
	mainPass.clearDepthBuffer = false;


	RenderChainNode uiRenderDependants[] = {
		&output
	};
	ImageReferenceHandle uiRenderImages[] = {
		&uiRender
	};
	ImageReferenceHandle uiSubResourceImages[] = {
		&subResource
	};
	RenderChainRenderModule uiRenderPass{};
	uiRenderPass.type = RENDER_NODE_RENDER;
	uiRenderPass.dependantCount = sizeof(uiRenderDependants) / sizeof(RenderChainNode);
	uiRenderPass.dependants = uiRenderDependants;
	uiRenderPass.sampleCount = IMAGE_SAMPLE_COUNT_4;

	uiRenderPass.outputImageCount = sizeof(uiRenderImages) / sizeof(ImageReferenceHandle);
	uiRenderPass.outputImages = uiRenderImages;
	uiRenderPass.shaderWriteImageCount = sizeof(uiSubResourceImages)/sizeof(ImageReferenceHandle);
	uiRenderPass.shaderWriteImages = uiSubResourceImages;

	uiRenderPass.depthBuffered = true;
	uiRenderPass.depthImage = nullptr;
	uiRenderPass.clearDepthBuffer = false;


	RenderChainEntryPoint mainEntry{};
	mainEntry.type = RENDER_NODE_ENTRY;
	mainEntry.module = &mainPass;
	mainEntry.importImageCount = 0;
	mainEntry.importImages = nullptr;

	RenderChainEntryPoint uiEntry{};
	uiEntry.type = RENDER_NODE_ENTRY;
	uiEntry.module = &uiRenderPass;
	uiEntry.importImageCount = 0;
	uiEntry.importImages = nullptr;


	ImageResourceDescription images[] = {
		{&albedo, IMAGE_FORMAT_B8G8R8A8_SRGB},
		{&position, IMAGE_FORMAT_B8G8R8A8_SRGB},
		{&specular, IMAGE_FORMAT_B8G8R8A8_SRGB},
		{&normal, IMAGE_FORMAT_B8G8R8A8_SRGB},

		{&uiRender, IMAGE_FORMAT_B8G8R8A8_SRGB},
		{&deferedRender, IMAGE_FORMAT_B8G8R8A8_SRGB},
		{&transparentRender, IMAGE_FORMAT_B8G8R8A8_SRGB}
	};

	ImageSubResourceDescription subResourceImages[] = {
		{&subResource, IMAGE_FORMAT_B8G8R8A8_SRGB, 500,500 }
	};

	ImageReferenceHandle depthImages[] = {
		&depth
	};

	RenderChainEntryPoint entryPoints[] = {
		mainEntry,
		uiEntry
	};

	void* renderModules[] = {
		&mainPass,
		&deferedPass,
		&transparentPass,
		&uiRenderPass
	};

	RenderChainCreateInfo info = {};
	info.output = output;
	info.entryPointCount = sizeof(entryPoints)/sizeof(RenderChainEntryPoint);
	info.entryPoints = entryPoints;


	renderChainCreate(handle, info, &renderChain);
	
}


void destroyGraphics() {

	

}





