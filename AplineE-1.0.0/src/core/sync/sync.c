#include "sync.h"

void synchronisationCreate(EngineHandle handle, EngineSettings settings) {
	VkSemaphoreCreateInfo semaphoreInfo = { 0 };
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo = { 0 };
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkCheck(vkCreateSemaphore(DEVICE(handle), &semaphoreInfo, nullptr, &IMAGE_AVAILABLE_SEMAPHORE(handle)[i]));
		VkCheck(vkCreateSemaphore(DEVICE(handle), &semaphoreInfo, nullptr, &RENDER_FINISHED_SEMAPHORE(handle)[i]));
		VkCheck(vkCreateFence(DEVICE(handle), &fenceInfo, nullptr, &IN_FLIGHT_FENCE(handle)[i]));
	}
}

void synchronisationDestroy(EngineHandle handle) {
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(DEVICE(handle), IMAGE_AVAILABLE_SEMAPHORE(handle)[i], nullptr);
		vkDestroySemaphore(DEVICE(handle), RENDER_FINISHED_SEMAPHORE(handle)[i], nullptr);
		vkDestroyFence(DEVICE(handle), IN_FLIGHT_FENCE(handle)[i], nullptr);
	}
}
