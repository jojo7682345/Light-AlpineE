
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

#pragma region IMAGE_RESOURCES
ImageStack worldRenderStack;
ImageReference albedo;
ImageReference normal;
ImageReference position;
ImageReference specular;
ImageReference depth;
ImageReference worldRenderResult;

ImageStack uiRenderStack;
ImageReference uiImage;
ImageReference uiDepth;

ImageStack resolveStack;
ImageReference uiResolve;

ImageStack renderResultStack;
ImageReference renderResult;

ImagePool imagePool;
#pragma endregion

RenderModule geometryModule;
RenderModule uiModule;

ComputeModule deferedLightingPassModule;
ComputeModule imageCombineModule;
RenderChain mainRenderChain;


void buildGraphics(EngineHandle handle) {
	
#pragma region Image_Resources

	ImageDataDescription worldRenderingImages[] = {
		{ &albedo, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR },
		{ &normal, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR },
		{ &position, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR },
		{ &specular, IMAGE_FORMAT_R32_SFLOAT, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR },
		{ &depth, IMAGE_FORMAT_D32_SFLOAT, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, IMAGE_ASPECT_DEPTH },
		{ &worldRenderResult, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR }
	};

	ImageSize screenSpace = { 0 };
	screenSpace.widthScaling = 1;
	screenSpace.heightScaling = 1;
	screenSpace.widthScalingType = SIZE_SCALING_TYPE_MULTIPLY;
	screenSpace.heightScalingType = SIZE_SCALING_TYPE_MULTIPLY;

	ImageStackLayout worldRendering = { 0 };
	worldRendering.size.widthScaling = 1;
	worldRendering.size = screenSpace;
	worldRendering.sampleCount = IMAGE_SAMPLE_COUNT_1;
	worldRendering.imageCount = IMAGE_DATA_COUNT(worldRenderingImages);
	worldRendering.imageDescriptions = worldRenderingImages;
	worldRendering.imageStack = &worldRenderStack;

	ImageDataDescription uiRenderingImages[] = {
		{ &uiImage, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT | IMAGE_USAGE_INPUT_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR},
		{ &uiDepth, IMAGE_FORMAT_D32_SFLOAT, IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, IMAGE_ASPECT_DEPTH }
	};
	ImageStackLayout uiRendering = { 0 };
	uiRendering.size = screenSpace;
	uiRendering.sampleCount = IMAGE_SAMPLE_COUNT_4;
	uiRendering.imageCount = IMAGE_DATA_COUNT(uiRenderingImages);
	uiRendering.imageDescriptions = uiRenderingImages;
	uiRendering.imageStack = &uiRenderStack;

	ImageDataDescription resolveImages[] = {
		{ &uiResolve, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR }
	};
	ImageStackLayout resolveRendering = { 0 };
	resolveRendering.size = screenSpace;
	resolveRendering.sampleCount = IMAGE_SAMPLE_COUNT_1;
	resolveRendering.imageCount = IMAGE_DATA_COUNT(resolveImages);
	resolveRendering.imageDescriptions = resolveImages;
	resolveRendering.imageStack = &resolveStack;

	ImageDataDescription renderResultImages[] = {
		{ &renderResult, IMAGE_FORMAT_B8G8R8A8_SRGB, IMAGE_USAGE_COLOR_ATTACHMENT_BIT, IMAGE_ASPECT_COLOR }
	};
	ImageStackLayout renderResultRendering = { 0 };
	renderResultRendering.size = screenSpace;
	renderResultRendering.sampleCount = IMAGE_SAMPLE_COUNT_1;
	renderResultRendering.imageCount = IMAGE_DATA_COUNT(renderResultImages);
	renderResultRendering.imageDescriptions = renderResultImages;
	renderResultRendering.imageStack = &renderResultStack;

	ImageStackLayout imageStacks[] = {
		worldRendering,
		uiRendering,
		resolveRendering,
		renderResultRendering
	};
	ImagePoolCreateInfo imagePoolCreateInfo = { 0 };
	imagePoolCreateInfo.imageStackCount = IMAGE_STACK_LAYOUT_COUNT(imageStacks);
	imagePoolCreateInfo.imageStacks = imageStacks;
	imagePoolCreate(handle, imagePoolCreateInfo, &imagePool);

#pragma endregion

#pragma region RenderModules

	ImageReference geometryOutputImages[] = {
		albedo,
		normal,
		position,
		specular
	};
	RenderModuleCreateInfo geometryModuleInfo = {};
	geometryModuleInfo.type = RENDER_MODULE_TYPE_GEOMETRY;
	geometryModuleInfo.directInputImages = { nullptr, 0, nullptr };
	geometryModuleInfo.shaderInputImages = { 0, nullptr };
	geometryModuleInfo.geometry.depthImage = depth;
	geometryModuleInfo.output.outputStack = worldRenderStack;
	geometryModuleInfo.output.outputImageCount = IMAGE_REF_COUNT(geometryOutputImages);
	geometryModuleInfo.output.outputImages = geometryOutputImages;
	geometryModuleInfo.geometry.resolveImages = nullptr;
	renderModuleCreate(handle, geometryModuleInfo, &geometryModule);

	ImageReference uiOutputImages[] = {
		uiImage
	};
	ImageReference uiResolveImages[] = {
		uiResolve
	};
	RenderModuleCreateInfo uiModuleInfo = {};
	uiModuleInfo.type = RENDER_MODULE_TYPE_GEOMETRY;
	uiModuleInfo.directInputImages = { nullptr, 0, nullptr };
	uiModuleInfo.shaderInputImages = { 0, nullptr };
	uiModuleInfo.geometry.depthImage = uiDepth;
	uiModuleInfo.output.outputStack = uiRenderStack;
	uiModuleInfo.output.outputImageCount = IMAGE_REF_COUNT(uiOutputImages);
	uiModuleInfo.output.outputImages = uiOutputImages;
	uiModuleInfo.geometry.resolveImages = uiResolveImages;
	renderModuleCreate(handle, uiModuleInfo, &uiModule);

	ImageReference deferedLightingPassOutput[] = {
		worldRenderResult
	};
	ComputeModuleCreateInfo deferedLightingPassModuleInfo = {};
	deferedLightingPassModuleInfo.type = COMPUTE_MODULE_TYPE_POST_PROCESS;
	deferedLightingPassModuleInfo.shaderInputImages = { IMAGE_REF_COUNT(geometryOutputImages), geometryOutputImages };
	deferedLightingPassModuleInfo.shaderOutputImages = { IMAGE_REF_COUNT(deferedLightingPassOutput), deferedLightingPassOutput };
	computeModuleCreate(handle, deferedLightingPassModuleInfo, &deferedLightingPassModule);

	ImageReference imageCombineInput[] = {
		worldRenderResult,
		uiResolve
	};
	ImageReference imageCombineOutput[] = {
		renderResult
	};
	ComputeModuleCreateInfo imageCombineModuleInfo = {};
	imageCombineModuleInfo.type = COMPUTE_MODULE_TYPE_IMAGE_COMBINE;
	imageCombineModuleInfo.shaderInputImages = { IMAGE_REF_COUNT(imageCombineInput), imageCombineInput };
	imageCombineModuleInfo.shaderOutputImages = { IMAGE_REF_COUNT(imageCombineOutput), imageCombineOutput };
	computeModuleCreate(handle, imageCombineModuleInfo, &imageCombineModule);

#pragma endregion

#pragma region RenderChain

	RenderModule renderModules[] = {
		geometryModule,
		uiModule
	};

	ComputeModule computeModules[] = {
		deferedLightingPassModule,
		imageCombineModule
	};

	ComputeModuleDependency dependencies[]{
		{  deferedLightingPassModule, imageCombineModule }
	};


	ImageReference outputImages[] = {
		renderResult
	};

	RenderChainCreateInfo renderChainCreateInfo = {};

	renderChainCreateInfo.input = { 0, nullptr };

	renderChainCreateInfo.preRenderComputeModuleCount = 0;
	renderChainCreateInfo.preRenderComputeModules = nullptr;
	renderChainCreateInfo.preRenderDependencyCount = 0;
	renderChainCreateInfo.preRenderDependencies = nullptr;

	renderChainCreateInfo.renderStepCount = sizeof(renderModules) / sizeof(RenderModule);
	renderChainCreateInfo.renderSteps = renderModules;
	renderChainCreateInfo.renderStepImageSize = screenSpace;
	renderChainCreateInfo.renderStepDependencyCount = 0;
	renderChainCreateInfo.renderStepDependencies = nullptr;

	renderChainCreateInfo.postRenderComputeModuleCount = sizeof(computeModules) / sizeof(ComputeModule);
	renderChainCreateInfo.postRenderComputeModules = computeModules;
	renderChainCreateInfo.postRenderDependencyCount = sizeof(dependencies) / sizeof(ComputeModuleDependency);
	renderChainCreateInfo.postRenderDependencies = dependencies;
	
	renderChainCreateInfo.output = { IMAGE_REF_COUNT(outputImages), outputImages};

	renderChainCreate(handle, renderChainCreateInfo, &mainRenderChain);
	

#pragma endregion

}



void destroyGraphics() {

	renderModuleDestroy(geometryModule);
	renderModuleDestroy(uiModule);

	computeModuleDestroy(deferedLightingPassModule);
	computeModuleDestroy(imageCombineModule);

	renderChainDestroy(mainRenderChain);

	imagePoolDestroy(imagePool);

}





