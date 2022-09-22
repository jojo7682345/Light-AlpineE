
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

void buildGraphics(EngineHandle handle) {
	
	ImageAttachment deferedAttachments[] = {
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Albedo
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Normal
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Position
		{IMAGE_FORMAT_R8_SRGB}, //Specular
		
		{IMAGE_FORMAT_R8_SRGB}, //Depth

		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // Deferedpass output
		{IMAGE_FORMAT_B8G8R8A8_SRGB}, // TransparentPass output
	};

	ImageAttachmentRef mainGeometryImages[] = {
		{0},
		{1},
		{2},
		{3}
	};
	RenderModuleDescription geometrypass{};
	geometrypass.colorAttachmentCount = 4;
	geometrypass.colorAttachments = mainGeometryImages;
	geometrypass.depthAttachment = 4;
	geometrypass.sampleCount = IMAGE_SAMPLE_COUNT_1;

	ImageAttachmentRef deferedPassImages[] = {
		{5}
	};
	RenderModuleDescription deferedPass{};
	deferedPass.colorAttachmentCount = 1;
	deferedPass.colorAttachments = deferedPassImages;
	deferedPass.depthAttachment = ATTACHMENT_UNUSED;
	deferedPass.sampleCount = IMAGE_SAMPLE_COUNT_1;

	ImageAttachmentRef transparentPassImages[] = {
		{6}
	};
	RenderModuleDescription transparentPass{};
	transparentPass.colorAttachmentCount = 1;
	transparentPass.colorAttachments = transparentPassImages;
	transparentPass.depthAttachment = 4;
	transparentPass.sampleCount = IMAGE_SAMPLE_COUNT_1;





}


void destroyGraphics() {



}





