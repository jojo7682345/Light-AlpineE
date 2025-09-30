#include "core.h"
#include "debug/debug.h"
#include "gpu/gpu.h"
#include "device/device.h"
#include "instance/instance.h"
#include "window/window.h"
#include "swapchain/swapchain.h"
#include "graphics/graphics.h"
#include "command/command.h"
#include "sync/sync.h"
#include <stdio.h>



void buildEngine(EngineSettings settings, EngineHandle handle) {

	//TODO: check if settings are valid
	if (settings.windowWidth == 0 || settings.windowHeight == 0) {
		printf("Error: window width and height must be greater than 0\n");
		abort();
	}

	handle->system.framerateLimit.enabled = settings.fpsLimitEnabled;
	handle->system.framerateLimit.max_fps = settings.fpsLimit;

	//
	debugCheckSupport();

	//create and open the window
	windowCreate(handle, settings);

	//create vulkan instance
	instanceCreate(handle, settings);

	//setup the debug mesenger
	debugMessengerCreate(handle, settings);

	//create surface
	surfaceCreate(handle, settings);

	//select physical device
	gpuSelectSuitable(handle, settings);

	//create logical device
	deviceCreate(handle, settings);

	//create command pool
	commandBufferPoolCreate(handle, settings);

	gpuUpdateSystemCapabilities(handle);
	if (!swapchainValidateSettings(handle, settings)) {
		printf("Error: swapchain settings are invalid\n");
		abort();
	}

	buildGraphics(handle);


	//renderTreesBuild(handle);

	//swapchainCreate(handle, settings);


	//create graphicspipeline
	//graphicsPipelineCreate(handle, settings);

	//allocate framebuffers
	//frameBuffersAllocate(handle, settings);

	//create command buffer
	//commandBufferCreate(handle, settings);

	//create sync objects
	//synchronisationCreate(handle, settings);

}


void destroyEngine(EngineHandle handle) {

	synchronisationDestroy(handle);

	commandBufferDestroy(handle);

	commandBufferPoolDestroy(handle);

	frameBuffersFree(handle);

	graphicsPipelineDestroy(handle);

	//renderSurfaceFreeDefaults(handle);
	destroyGraphics();

	deviceDestroy(handle);

	surfaceDestroy(handle);

	debugMessengerDestroy(handle);

	instanceDestroy(handle);

	windowDestroy(handle);
}

void destroyScenes(EngineHandle handle) {
	uint32_t count = dynamicArrayCountPopulated(&handle->scenes);
	Scene* scenes = fsAllocate(sizeof(Scene) * count);
	dynamicArrayExtractPopulated(scenes, &handle->scenes);
	for (uint32_t i = 0; i < count; i++) {
		sceneDestroy(scenes[i]);
	}
	fsFree(scenes);
	dynamicArrayDestroy(&handle->scenes);
}

void destroyComponentTypes(EngineHandle handle) {
	dynamicArrayDestroy(&handle->componentTypes);
	dynamicArrayDestroy(&handle->componentTypeMap);
	handle->componentTypeCount = 0;
}

uint32_t currentFrame = 0;

void drawFrame(EngineHandle handle) {
	vkWaitForFences(DEVICE(handle), 1, &IN_FLIGHT_FENCE(handle)[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(DEVICE(handle), 1, &IN_FLIGHT_FENCE(handle)[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(DEVICE(handle), SWAPCHAIN(handle), UINT64_MAX, IMAGE_AVAILABLE_SEMAPHORE(handle)[currentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(COMMAND_BUFFERS(handle)[currentFrame], 0);
	commandBufferRecord(COMMAND_BUFFERS(handle)[currentFrame], handle, imageIndex);

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { IMAGE_AVAILABLE_SEMAPHORE(handle)[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &COMMAND_BUFFERS(handle)[currentFrame];

	VkSemaphore signalSemaphores[] = { RENDER_FINISHED_SEMAPHORE(handle)[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkCheck(vkQueueSubmit(GRAPHICS_QUEUE(handle), 1, &submitInfo, IN_FLIGHT_FENCE(handle)[currentFrame]));

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { SWAPCHAIN(handle) };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(PRESENT_QUEUE(handle), &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void VkCheck(VkResult result) {
	if (result != VK_SUCCESS) {
		printf("Error: Vulkan API call resulted in failiure, with code %i\n", result);
	}
}


void mainLoop(EngineHandle handle) {
	while (!glfwWindowShouldClose(((EngineHandle_T*)handle)->window)) {
		glfwPollEvents();
		drawFrame(handle);
	}
	vkDeviceWaitIdle(DEVICE(handle));
}

int main() {

	EngineHandle_T handleData = {
		.componentTypes = {.dataSize = sizeof(ComponentTypeDesctription) },
		.componentTypeMap = {.dataSize = sizeof(uint32_t) },
		.scenes = {.dataSize = sizeof(Scene) },
	};
	EngineHandle handle = &handleData;

	registerEngineDefaultComponentTypes(handle);
	buildComponentTypes(handle);
	handle->componentTypeCount = dynamicArrayCountPopulated(&handle->componentTypes);

	EngineSettings settings = getEngineSettings();
	buildEngine(settings, handle);

	buildScenes(handle);

	//mainLoop(handle);

	destroyScenes(handle);

	destroyEngine(handle);

	destroyComponentTypes(handle);

	MUallocation* allocations = NULL;
	size_t allocCount;
	findMemoryLeaks(&allocations, &allocCount);
	printAllocations(allocations, allocCount);
}



void constructEngine(EngineHandle handle, EngineSettings settings) {








}