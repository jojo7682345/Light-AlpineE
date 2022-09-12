#pragma once
#include "../core.h"
#include "../../engine/engine.h"

typedef struct SwapchainCreateInfo {
	VkFormat format;
	uint32_t imageCount;
	VkExtent2D extent;
	VkColorSpaceKHR colorSpace;
	VkPresentModeKHR presentMode;
	VkSwapchainKHR oldSwapchain;
}SwapchainCreateInfo;

typedef struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formatCount;
	VkSurfaceFormatKHR* formats;
	uint32_t presentModeCount;
	VkPresentModeKHR* presentModes;
}SwapChainSupportDetails;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, EngineHandle handle);

void freeSwapchainSupportDetails(SwapChainSupportDetails details);

VkBool32 swapChainCheckSupport(VkPhysicalDevice device, EngineHandle handle);

VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const uint32_t availableFormatCount, VkSurfaceFormatKHR* availableFormats, EngineSettings settings);

VkPresentModeKHR chooseSwapchainPresentMode(const uint32_t availablePresentModeCount, VkPresentModeKHR* availablePresentModes, EngineSettings settings);

VkExtent2D chooswSwapchainExtent(const VkSurfaceCapabilitiesKHR* capabilities, EngineSettings settings, EngineHandle handle);

void selectSwapchainProperties(EngineHandle handle, EngineSettings settings);

void swapchainCreate(EngineHandle handle, EngineSettings settings);
void swapchainRetrieveDesiredSettings(EngineHandle handle, SwapchainCreateInfo* createInfo);

void swapchainnDestroyRenderDisplay(EngineHandle handle, VkSwapchainKHR swapchain);
AeResult swapchainCreateRenderDisplay(EngineHandle handle, SwapchainCreateInfo info, VkSwapchainKHR* swapchain);
void swapchainDestroy(EngineHandle handle);



bool_t vsyncOptionAvailable(EngineHandle handle, VsyncEnabled vsync);

VkPresentModeKHR getVsyncPresentMode(VsyncEnabled vsync);

bool_t updateSwapchainVsyncSettings(EngineHandle handle, VsyncEnabled vsync);


bool_t swapchainValidateSettings(EngineHandle handle, EngineSettings settings);