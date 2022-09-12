//#include <engine/engine.h>
//#include <core/core.h>
//#include <util/util.h>
//#include <core/swapchain/swapchain.h>
//
//void renderTargetCreateEngineDefaults(EngineHandle handle) {
//
//	//create applicationsurface
//	renderTargetDisplayCreate(handle);
//
//	//create ui surface
//	RenderTargetCreateInfo uiCreateInfo = { 0 };
//	uiCreateInfo.width = handle->applicationRenderTarget.extent.width;
//	uiCreateInfo.height = handle->applicationRenderTarget.extent.height;
//	uiCreateInfo.type = RENDER_TARGET_TYPE_2D;
//	VkClearColorValue clearColor = { 0 };
//	clearColor.float32[0] = 0.0f;
//	clearColor.float32[1] = 0.0f;
//	clearColor.float32[2] = 0.0f;
//	clearColor.float32[3] = 1.0f;
//	uiCreateInfo.clearColor = clearColor;
//	VkClearColorValue depthClearColor = { 0 };
//	depthClearColor.float32[0] = 1.0f;
//	depthClearColor.float32[1] = 0.0f;
//	depthClearColor.float32[2] = 0.0f;
//	depthClearColor.float32[3] = 0.0f;
//	uiCreateInfo.depthClearColor = depthClearColor;
//	uiCreateInfo.surfaceCount = 1;
//	renderTargetCreate(handle, uiCreateInfo, (RenderTarget)&handle->applicationGuiRenderTarget);
//
//	handle->applicationSurface = &handle->applicationRenderTarget.surface;
//	handle->applicationGuiSurface = handle->applicationGuiRenderTarget.surfaces;
//	
//}
//
//void renderTargetDestroyEngineDefaults(EngineHandle handle) {
//	renderTargetDisplayDestroy(handle);
//	renderTargetDestroy(handle, *(RenderTarget_T*)&handle->applicationGuiRenderTarget);
//}
//
//
//VkFormat findDepthFormat(EngineHandle handle) {
//	VkFormat formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
//
//	return findSupportedFormat(
//		handle,
//		sizeof(formats) / sizeof(VkFormat),
//		formats,
//		VK_IMAGE_TILING_OPTIMAL,
//		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//	);
//}
//
//void createRenderTarget2DImageResources(EngineHandle handle, RenderTarget2D renderTarget) {
//	createImage(
//		handle,
//		renderTarget->extent.width,
//		renderTarget->extent.height,
//		renderTarget->format,
//		VK_IMAGE_TILING_OPTIMAL,
//		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//		&renderTarget->image,
//		&renderTarget->memory
//	);
//	renderTarget->imageView = createImageView(handle, renderTarget->image, renderTarget->format, VK_IMAGE_ASPECT_COLOR_BIT);
//	//transitionImageLayout(handle, renderTarget->image, renderTarget->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//}
//
//void createRenderTarget3DImageResources(EngineHandle handle, RenderTarget3D renderTarget) {
//	ImageCreateInfo imageInfo = { 0 };
//	imageInfo.width = renderTarget->extent.width;
//	imageInfo.height = renderTarget->extent.height;
//	imageInfo.format = renderTarget->format;
//	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//	imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//	imageInfo.image = &renderTarget->image;
//
//	VkFormat depthFormat = findDepthFormat(handle);
//	ImageCreateInfo depthInfo = { 0 };
//	imageInfo.width = renderTarget->extent.width;
//	imageInfo.height = renderTarget->extent.height;
//	imageInfo.format = depthFormat;
//	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//	imageInfo.image = &renderTarget->depth.image;
//
//	ImageCreateInfo createInfos[] = { imageInfo, depthInfo };
//
//	createImages(handle, 2, createInfos, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderTarget->memory);
//
//	renderTarget->imageView = createImageView(handle, renderTarget->image, renderTarget->format, VK_IMAGE_ASPECT_COLOR_BIT);
//	//transitionImageLayout(handle, renderTarget->image, renderTarget->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//	renderTarget->depth.imageView = createImageView(handle, renderTarget->depth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
//	//transitionImageLayout(handle, renderTarget->depth.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//}
//
//void createRenderTargetDisplayResources(EngineHandle handle, RenderTargetDisplay renderTarget) {
//	VkFormat depthFormat = findDepthFormat(handle);
//
//	createImage(
//		handle,
//		renderTarget->extent.width,
//		renderTarget->extent.height,
//		depthFormat,
//		VK_IMAGE_TILING_OPTIMAL,
//		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//		&renderTarget->depth.image,
//		&renderTarget->memory
//	);
//	renderTarget->depth.imageView = createImageView(handle, renderTarget->depth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
//	//transitionImageLayout(handle, renderTarget->depth.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//}
//
//
//AeResult renderTargetDisplayCreate(EngineHandle handle) {
//
//	RenderTargetDisplay_T* target = &ENGINE_HANDLE_GET(handle, applicationRenderTarget);
//
//	RenderTargetDisplay_T base = { .surface = {.imageSize = {0,0},.imageOffset = {0,0}} };
//	memcpy(target, &base, sizeof(RenderTarget2D));
//	
//	SwapchainCreateInfo swapchainInfo = { 0 };
//	swapchainInfo.presentMode = getVsyncPresentMode(handle->system.vsyncEnabled);
//
//	swapchainRetrieveDesiredSettings(handle, &swapchainInfo);
//
//	swapchainCreateRenderDisplay(handle, swapchainInfo, &target->swapchain);
//	swapchainRetrieveRenderDisplayImages(handle, &swapchainInfo, target->swapchain, &target->swapchainImages);
//	target->type = RENDER_TARGET_TYPE_DISPLAY;
//	target->extent = swapchainInfo.extent;
//	target->format = swapchainInfo.format;
//	target->imageCount = swapchainInfo.imageCount;
//
//	VkClearColorValue clearColor = { 0 };
//	clearColor.float32[0] = 0.0f;
//	clearColor.float32[1] = 0.0f;
//	clearColor.float32[2] = 0.0f;
//	clearColor.float32[3] = 1.0f;
//	target->clearColor = clearColor;
//	VkClearColorValue depthClearColor = { 0 };
//	depthClearColor.float32[0] = 1.0f;
//	depthClearColor.float32[1] = 0.0f;
//	depthClearColor.float32[2] = 0.0f;
//	depthClearColor.float32[3] = 0.0f;
//	target->depthClearColor = depthClearColor;
//
//	//create depth resources
//	createRenderTargetDisplayResources(handle, target);
//
//	return AE_SUCCESS;
//}
//
//AeResult renderTarget2DCreate(EngineHandle handle, RenderTargetCreateInfo info, RenderTarget2D_T* target) {
//
//	RenderTarget2D_T base = { .surfaceCount = info.surfaceCount, .surfaces = fsAllocate(sizeof(RenderSurface_T)*info.surfaceCount) };
//	memcpy(target, &base, sizeof(RenderTarget2D));
//
//	target->extent.width = info.width;
//	target->extent.height = info.height;
//	target->type = RENDER_TARGET_TYPE_2D;
//	target->format = VK_FORMAT_B8G8R8A8_SRGB;
//	VkClearColorValue clearColor = { 0 };
//	clearColor.float32[0] = 0.0f;
//	clearColor.float32[1] = 0.0f;
//	clearColor.float32[2] = 0.0f;
//	clearColor.float32[3] = 1.0f;
//	target->clearColor = clearColor;
//
//	createRenderTarget2DImageResources(handle, target);
//
//	return AE_SUCCESS;
//}
//
//AeResult renderTarget3DCreate(EngineHandle handle, RenderTargetCreateInfo info, RenderTarget3D_T* target) {
//
//	RenderTarget3D_T base = { .surfaceCount = info.surfaceCount, .surfaces = fsAllocate(sizeof(RenderSurface_T)*info.surfaceCount) };
//	memcpy(target, &base, sizeof(RenderTarget2D));
//
//	
//	target->extent.width = info.width;
//	target->extent.height = info.height;
//	target->type = RENDER_TARGET_TYPE_3D;
//	target->format = VK_FORMAT_B8G8R8A8_SRGB;
//
//	VkClearColorValue clearColor = { 0 };
//	clearColor.float32[0] = 0.0f;
//	clearColor.float32[1] = 0.0f;
//	clearColor.float32[2] = 0.0f;
//	clearColor.float32[3] = 1.0f;
//	target->clearColor = clearColor;
//	VkClearColorValue depthClearColor = { 0 };
//	depthClearColor.float32[0] = 1.0f;
//	depthClearColor.float32[1] = 0.0f;
//	depthClearColor.float32[2] = 0.0f;
//	depthClearColor.float32[3] = 0.0f;
//	target->depthClearColor = depthClearColor;
//
//	createRenderTarget3DImageResources(handle, target);
//
//	return AE_SUCCESS;
//}
//
//AeResult renderTargetCreate(EngineHandle handle, RenderTargetCreateInfo info, RenderTarget_T* renderTarget) {
//	//validate the create info
//	if (info.width == 0 || info.height == 0) {
//		return AE_ERROR;
//	}
//
//	switch (info.type) {
//	case RENDER_TARGET_TYPE_DISPLAY:
//		break;
//	case RENDER_TARGET_TYPE_2D:
//		return renderTarget2DCreate(handle, info, (RenderTarget2D_T*)renderTarget);
//		break;
//	case RENDER_TARGET_TYPE_3D:
//		return renderTarget3DCreate(handle, info, (RenderTarget3D_T*)renderTarget);
//		break;
//	}
//
//	return AE_ERROR;
//}
//
//void renderTargetDisplayDestroy(EngineHandle handle) {
//	swapchainReleaseRenderDisplayImages(handle, &ENGINE_HANDLE_GET(handle, applicationRenderTarget));
//	swapchainnDestroyRenderDisplay(handle, ENGINE_HANDLE_GET(handle, applicationRenderTarget).swapchain);
//
//	vkDestroyImageView(DEVICE(handle), ENGINE_HANDLE_GET(handle, applicationRenderTarget).depth.imageView, nullptr);
//	vkDestroyImage(DEVICE(handle), ENGINE_HANDLE_GET(handle, applicationRenderTarget).depth.image, nullptr);
//	vkFreeMemory(DEVICE(handle), ENGINE_HANDLE_GET(handle, applicationRenderTarget).memory, nullptr);
//}
//
//void renderTarget2DDestroy(EngineHandle handle, RenderTarget2D_T target) {
//	vkDestroyImageView(DEVICE(handle), target.imageView, nullptr);
//	vkDestroyImage(DEVICE(handle), target.image, nullptr);
//
//	vkFreeMemory(DEVICE(handle), target.memory, nullptr);
//}
//
//void renderTarget3DDestroy(EngineHandle handle, RenderTarget3D_T target) {
//	vkDestroyImageView(DEVICE(handle), target.imageView, nullptr);
//	vkDestroyImage(DEVICE(handle), target.image, nullptr);
//	vkDestroyImageView(DEVICE(handle), target.depth.imageView, nullptr);
//	vkDestroyImage(DEVICE(handle), target.depth.image, nullptr);
//
//	vkFreeMemory(DEVICE(handle), target.memory, nullptr);
//}
//
//void renderTargetDestroy(EngineHandle handle, RenderTarget_T renderTarget) {
//
//	switch (renderTarget.type) {
//	case RENDER_TARGET_TYPE_DISPLAY:
//		break;
//	case RENDER_TARGET_TYPE_2D:
//		renderTarget2DDestroy(handle, *(RenderTarget2D_T*)&renderTarget);
//		break;
//	case RENDER_TARGET_TYPE_3D:
//		renderTarget3DDestroy(handle, *(RenderTarget3D_T*)&renderTarget);
//		break;
//	}
//}
//
//void engineBuildRenderTargets(EngineHandle handle, RenderSurfaceCreationPool pool) {
//
//	//create default rendersurfaces
//
//	
//	
//	
//
//
//}
