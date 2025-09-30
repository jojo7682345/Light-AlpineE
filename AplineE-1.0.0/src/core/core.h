#pragma once

#include <AlpineCore.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define MU_DEBUG
#include <MemoryUtilities/MemoryUtilities.h>
#include <MemoryUtilities/DataGrid.h>
#include <memory.h>
#include <stdlib.h>
#ifndef __CORE_INCLUDED__
#define __CORE_INCLUDED__

#include "../engine/engine.h"

#define GET(type,handle,member) (((type##_T*)(handle))->member)
#define ENGINE_HANDLE_GET(handle,member) GET(EngineHandle,handle,member)

#define INSTANCE(handle) GET(EngineHandle,handle,instance)
#define WINDOW(handle) GET(EngineHandle,handle,window)
#define DEBUG_MESSENGER(handle) GET(EngineHandle,handle,debugMessenger)
#define PHYSICAL_DEVICE(handle) GET(EngineHandle,handle,physicalDevice)
#define DEVICE(handle) GET(EngineHandle,handle,device)
#define GRAPHICS_QUEUE(handle) GET(EngineHandle,handle,graphicsQueue.queue)
#define PRESENT_QUEUE(handle) GET(EngineHandle,handle,presentQueue.queue)
#define SURFACE(handle) GET(EngineHandle,handle,surface)
#define SWAPCHAIN(handle) GET(EngineHandle,handle,swapchain.swapchain)
#define SWAPCHAIN_IMAGE_COUNT(handle) GET(EngineHandle,handle,swapchain.imageCount)
#define SWAPCHAIN_IMAGE_FORMAT(handle) GET(EngineHandle,handle,swapchain.imageFormat)
#define SWAPCHAIN_EXTENT(handle) GET(EngineHandle,handle,swapchain.extent)
#define SWAPCHAIN_IMAGES(handle) GET(EngineHandle,handle,swapchain.images)
#define SWAPCHAIN_IMAGE_VIEWS(handle) GET(EngineHandle,handle,swapchain.imageViews)
#define PIPELINE_LAYOUT(handle) GET(EngineHandle,handle,graphicsPipeline.layout)
#define RENDERPASS(handle) GET(EngineHandle,handle,graphicsPipeline.renderPass)
#define GRAPHICS_PIPELINE(handle) GET(EngineHandle, handle, graphicsPipeline.pipeline)
#define FRAME_BUFFERS(handle) GET(EngineHandle, handle, swapchain.framebuffers)
#define COMMAND_POOL(handle) GET(EngineHandle, handle, command.pool)
#define COMMAND_BUFFERS(handle) GET(EngineHandle, handle, command.buffers)
#define IMAGE_AVAILABLE_SEMAPHORE(handle) GET(EngineHandle, handle, synchronisation.imageAvailable)
#define RENDER_FINISHED_SEMAPHORE(handle) GET(EngineHandle, handle, synchronisation.renderFinished)
#define IN_FLIGHT_FENCE(handle) ENGINE_HANDLE_GET(handle, synchronisation.inFlight)

#define nullptr ((void*)0)

#define MAX_FRAMES_IN_FLIGHT 2

void VkCheck(VkResult result);

typedef struct Swapchain {
	VkSwapchainKHR swapchain;
	uint32_t imageCount;
	VkImage* images;
	VkFormat imageFormat;
	VkExtent2D extent;
	VkImageView* imageViews;
	VkFramebuffer* framebuffers;
} Swapchain;


typedef struct GraphicsPipeline_ {
	VkPipelineLayout layout;
	VkRenderPass renderPass;
	VkPipeline pipeline;
}GraphicsPipeline_;

typedef struct Synchronisation {
	VkSemaphore imageAvailable[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinished[MAX_FRAMES_IN_FLIGHT];
	VkFence inFlight[MAX_FRAMES_IN_FLIGHT];
} Synchronisation;

typedef struct RenderCommand {
	VkCommandPool pool;
	VkCommandBuffer buffers[MAX_FRAMES_IN_FLIGHT];
} RenderCommand;

typedef struct DeviceQueue {
	VkQueue queue;
	uint32_t index;
} DeviceQueue;

typedef struct EngineHandle_T {
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
	DeviceQueue graphicsQueue;
	DeviceQueue presentQueue;
	DeviceQueue transferQueue;
	
	VkSurfaceKHR surface;
	
	Swapchain swapchain;
	GraphicsPipeline_ graphicsPipeline;
	RenderCommand command;
	Synchronisation synchronisation;

	uint32_t componentTypeCount;
	_DynamicArray componentTypes;
	_DynamicArray componentTypeMap;

	_DynamicArray scenes;

	//new shit
	EngineProperties system;

} EngineHandle_T;

typedef struct ShaderResource_T {
	int a;
}ShaderResource_T;


#endif // __CORE_INCLUDED__