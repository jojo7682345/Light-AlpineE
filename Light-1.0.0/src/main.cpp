
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

RenderModule geometryPass;
RenderModule deferedPass;
RenderModule transparentPass;

void buildGraphics(EngineHandle handle) {

	

	ImageAttachmentRef mainGeometryImages[] = {
		{0},
		{1},
		{2},
		{3}
	};
	RenderModuleDescription geometrypassDescription{};
	geometrypassDescription.handle = &geometryPass;
	geometrypassDescription.colorAttachmentCount = 4;
	geometrypassDescription.colorAttachments = mainGeometryImages;
	geometrypassDescription.depthAttachment = 4;
	geometrypassDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;

	ImageAttachmentRef deferedPassImages[] = {
		{5}
	};
	RenderModuleDescription deferedPassDescription{};
	deferedPassDescription.handle = &deferedPass;
	deferedPassDescription.colorAttachmentCount = 1;
	deferedPassDescription.colorAttachments = deferedPassImages;
	deferedPassDescription.inputAttachmentCount = 4;
	deferedPassDescription.inputAttachments = mainGeometryImages;
	deferedPassDescription.depthAttachment = ATTACHMENT_UNUSED;
	deferedPassDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;

	ImageAttachmentRef transparentPassImages[] = {
		{6}
	};
	RenderModuleDescription transparentPassDescription{};
	transparentPassDescription.handle = &transparentPass;
	transparentPassDescription.colorAttachmentCount = 1;
	transparentPassDescription.colorAttachments = transparentPassImages;
	transparentPassDescription.depthAttachment = 4;
	transparentPassDescription.sampleCount = IMAGE_SAMPLE_COUNT_1;

	RenderModuleDescription renderModules[] = {
		geometrypassDescription,
		deferedPassDescription,
		transparentPassDescription,
	};
	
	ImageAttachment deferedAttachments[] = {
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Albedo
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Normal
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Position
		{IMAGE_FORMAT_R8_SRGB}, //Specular

		{IMAGE_FORMAT_R8_SRGB}, //Depth

		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Deferedpass output
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // TransparentPass output
	};

	uint32_t deferedChain[] = {
		0,
		1,
		2, 
	};

	uint32_t* rendererChains[] = {
		deferedChain
	};



}


void destroyGraphics() {



}





