#include "command.h"
#include "../gpu/gpu.h"
void commandBufferPoolCreate(EngineHandle handle, EngineSettings settings) {
	QueueFamilyIndices queueFamilyIndicies = findQueueFamilies(PHYSICAL_DEVICE(handle), handle);

	VkCommandPoolCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = queueFamilyIndicies.graphicsFamily.index;

	VkCheck(vkCreateCommandPool(DEVICE(handle), &createInfo, nullptr, &COMMAND_POOL(handle)));

}

void commandBufferCreate(EngineHandle handle, EngineSettings settings) {
	VkCommandBufferAllocateInfo allocInfo = { 0 };
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = COMMAND_POOL(handle);
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
	VkCheck(vkAllocateCommandBuffers(DEVICE(handle), &allocInfo, COMMAND_BUFFERS(handle)));
}

void commandBufferRecord(VkCommandBuffer buffer, EngineHandle handle, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	VkCheck(vkBeginCommandBuffer(buffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RENDERPASS(handle);
	renderPassInfo.framebuffer = FRAME_BUFFERS(handle)[imageIndex];
	renderPassInfo.renderArea.offset = (VkOffset2D){ 0 };
	renderPassInfo.renderArea.extent = SWAPCHAIN_EXTENT(handle);

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GRAPHICS_PIPELINE(handle));

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)SWAPCHAIN_EXTENT(handle).width;
	viewport.height = (float)SWAPCHAIN_EXTENT(handle).height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer, 0, 1, &viewport);

	VkRect2D scissor = {0};
	scissor.offset = (VkOffset2D){ 0 };
	scissor.extent = SWAPCHAIN_EXTENT(handle);
	vkCmdSetScissor(buffer, 0, 1, &scissor);

	vkCmdDraw(buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(buffer);

	VkCheck(vkEndCommandBuffer(buffer));
}

void commandBufferPoolDestroy(EngineHandle handle) {
	vkDestroyCommandPool(DEVICE(handle), COMMAND_POOL(handle), nullptr);
}

void commandBufferDestroy(EngineHandle handle) {

}
