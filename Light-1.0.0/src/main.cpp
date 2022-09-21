
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

Image albedo;
Image normal;
Image position;
Image specular;

Image deferedRender;
Image transparentRender;

Image depthImage;

RenderModule geometryPass;
RenderModule deferedPass;
RenderModule transparentPass;

Renderer deferedRenderer;

void buildGraphics(EngineHandle handle) {
	
	ImageRef geometryPassOutput[] = {
		&albedo,
		&normal,
		&position,
		&specular
	};
	RenderModuleDescription geometryPassDescription{};
	geometryPassDescription.handle = &geometryPass;
	geometryPassDescription.type = RENDER_MODULE_TYPE_RENDER;
	geometryPassDescription.depthImage = &depthImage;
	geometryPassDescription.outputImageCount = sizeof(geometryPassOutput) / sizeof(ImageRef);
	geometryPassDescription.outputImages = geometryPassOutput;

	ImageRef deferedPassOutput[] = {
		&deferedRender
	};
	ImageRef deferedPassPreserve[] = {
		&depthImage
	};
	RenderModuleDescription deferedPassDescription{};
	deferedPassDescription.handle = &deferedPass;
	deferedPassDescription.type = RENDER_MODULE_TYPE_POST_PROCESS;
	deferedPassDescription.inputImageCount = geometryPassDescription.outputImageCount;
	deferedPassDescription.inputImages = geometryPassOutput;
	deferedPassDescription.preserveImageCount = sizeof(deferedPassPreserve) / sizeof(ImageRef);
	deferedPassDescription.preserveImages = deferedPassPreserve;
	deferedPassDescription.outputImageCount = sizeof(deferedPassOutput) / sizeof(ImageRef);
	deferedPassDescription.outputImages = deferedPassOutput;

	ImageRef transparentPassOutput[] = {
		&transparentRender
	};
	RenderModuleDescription transparentPassDescription{};
	transparentPassDescription.handle = &transparentPass;
	transparentPassDescription.type = RENDER_MODULE_TYPE_RENDER;
	transparentPassDescription.inputImageCount = deferedPassDescription.outputImageCount;
	transparentPassDescription.inputImages = deferedPassOutput;
	transparentPassDescription.outputImageCount = sizeof(transparentPassOutput) / sizeof(ImageRef);
	transparentPassDescription.outputImages = transparentPassOutput;

	RenderModuleDependency geometryDeferedDep{};
	geometryDeferedDep.src = &geometryPass;
	geometryDeferedDep.srcStage = RENDER_MODULE_STAGE_FRAGMENT_OUTPUT;
	geometryDeferedDep.dst = &deferedPass;
	geometryDeferedDep.dstStage = RENDER_MODULE_STAGE_FRAGMENT_INPUT;

	RenderModuleDependency deferedTransparentDep{};
	deferedTransparentDep.src = &deferedPass;
	deferedTransparentDep.srcStage = RENDER_MODULE_STAGE_FRAGMENT_OUTPUT;
	deferedTransparentDep.dst = &transparentPass;
	deferedTransparentDep.dstStage = RENDER_MODULE_STAGE_FRAGMENT_INPUT;

	RenderModuleDependency geometryTransparentDep{};
	deferedTransparentDep.src = &geometryPass;
	deferedTransparentDep.srcStage = RENDER_MODULE_STAGE_DEPTH_OUTPUT;
	deferedTransparentDep.dst = &transparentPass;
	deferedTransparentDep.dstStage = RENDER_MODULE_STAGE_DEPTH_INPUT;

	RenderModuleDependency outputDependency{};
	outputDependency.src = &transparentPass;
	outputDependency.srcStage = RENDER_MODULE_STAGE_FRAGMENT_OUTPUT;
	outputDependency.dst = RENDERER_OUTPUT;

	RenderModuleDescription renderModules[] = {
		geometryPassDescription,
		deferedPassDescription,
		transparentPassDescription
	};

	RenderModuleDependency dependencies[] = {
		geometryDeferedDep,
		deferedTransparentDep,
		geometryTransparentDep,
		outputDependency
	};

	ImageResourceDescription images[] = {
		{&albedo,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED},
		{&normal,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED},
		{&position,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED},
		{&specular,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED},
		{&deferedRender,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED},
		{&transparentRender,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_RENDER_OUTPUT}
	};

	ImageDepthResourceDescription depthImages[] = {
		{&depthImage,IMAGE_ACCESS_TYPE_UNDEFINED,IMAGE_ACCESS_TYPE_UNDEFINED, IMAGE_SAMPLE_COUNT_1}
	};

	RendererCreateInfo deferedRendererInfo{};
	deferedRendererInfo.width = 0;
	deferedRendererInfo.height = 0;
	deferedRendererInfo.imageResourceCount = sizeof(images) / sizeof(ImageResourceDescription);
	deferedRendererInfo.imageResources = images;
	deferedRendererInfo.depthImageResourceCount = sizeof(depthImages) / sizeof(ImageResourceDescription);
	deferedRendererInfo.depthImageResources = depthImages;

	deferedRendererInfo.renderModuleCount = sizeof(renderModules) / sizeof(RenderModuleDescription);
	deferedRendererInfo.renderModules = renderModules;
	deferedRendererInfo.dependencyCount = sizeof(dependencies) / sizeof(RenderModuleDependency);
	deferedRendererInfo.dependencies = dependencies;

	rendererCreate(handle, deferedRendererInfo, &deferedRenderer);

}


void destroyGraphics() {

	rendererDestroy(deferedRenderer);

}





