#include "Image.h"
#include <util/util.h>
#include <util/translate.h>
#include <math.h>
#include <stdio.h>
#include <MemoryUtilities/DataGrid.h>


#pragma region ImagePool

void updateImageSize(EngineHandle handle, ImageSize* size) {

	if (size->widthScalingType != SIZE_SCALING_TYPE_FIXED) {
		if (size->widthScalingType == SIZE_SCALING_TYPE_MULTIPLY) {
			size->width = engineWindowGetWidth(handle) * size->widthScaling;
		}
		if (size->widthScalingType == SIZE_SCALING_TYPE_DIVIDE_BY_EXP2) {
			size->width = engineWindowGetWidth(handle) / (uint32_t)exp2(size->widthScaling);
		}
	}
	if (size->heightScalingType != SIZE_SCALING_TYPE_FIXED) {
		if (size->heightScalingType == SIZE_SCALING_TYPE_MULTIPLY) {
			size->height = engineWindowGetHeight(handle) * size->heightScaling;
		}
		if (size->heightScalingType == SIZE_SCALING_TYPE_DIVIDE_BY_EXP2) {
			size->height = engineWindowGetHeight(handle) / (uint32_t)exp2(size->heightScaling);
		}
	}
}

void imagePoolCreate(EngineHandle handle, ImagePoolCreateInfo info, ImagePool* pool) {
	const uint32_t stackCount = info.imageStackCount;

	ImagePool_T* imagePool = (ImagePool_T*)fsAllocate(sizeof(ImagePool_T));
	imagePool->handle = handle;
	imagePool->imageStackCount = stackCount;
	imagePool->imageStacks = (ImageStack_T*)fsAllocate(sizeof(ImageStack_T) * stackCount);

	uint32_t poolImageCount = 0;
	for (uint32_t i = 0; i < stackCount; i++) {
		const ImageStackLayout layout = info.imageStacks[i];
		poolImageCount += layout.imageCount;
	}
	ImageCreateInfo* imageCreateInfos = (ImageCreateInfo*)fsAllocate(sizeof(ImageCreateInfo) * poolImageCount);
	VkImageView** imageViews = (VkImageView**)fsAllocate(sizeof(VkImageView*) * poolImageCount);
	VkImageAspectFlags* aspectFlags = (VkImageAspectFlags*)fsAllocate(sizeof(VkImageAspectFlags) * poolImageCount);

	uint32_t imageIndex = 0;
	for (uint32_t i = 0; i < stackCount; i++) {
		ImageStackLayout layout = info.imageStacks[i];
		const uint32_t imageCount = layout.imageCount;

		updateImageSize(handle, &layout.size);


		const VkSampleCountFlags sampleCountFlags = translateSampleCount(layout.sampleCount);

		ImageStack_T* stack = &imagePool->imageStacks[i];
		stack->size = layout.size;
		stack->sampleCount = sampleCountFlags;
		stack->imageCount = imageCount;
		stack->size = layout.size;

		stack->extent = (VkExtent2D*)&stack->size.width;

		stack->images = (VkImage*)fsAllocate(sizeof(VkImage) * imageCount);
		stack->imageViews = (VkImageView*)fsAllocate(sizeof(VkImageView) * imageCount);
		stack->formats = (VkFormat*)fsAllocate(sizeof(VkFormat) * imageCount);
		stack->usage = (VkImageUsageFlags*)fsAllocate(sizeof(VkImageUsageFlags) * imageCount);
		stack->imageReferences = (ImageReference_T*)fsAllocate(sizeof(ImageReference_T) * imageCount);
		stack->aspectFlags = (VkImageAspectFlags*)fsAllocate(sizeof(VkImageAspectFlags) * imageCount);

		stack->pool = imagePool;

		for (uint32_t j = 0; j < imageCount; j++) {
			const VkFormat format = tanslateImageFormat(layout.imageDescriptions[j].imageFormat);
			const VkImageUsageFlags usageFlags = translateImageUsage(layout.imageDescriptions[j].imageUsage);
			const VkImageAspectFlags imageAspectFlags = translateImageAspect(layout.imageDescriptions[j].imageAspect);

			ImageCreateInfo* imageCreateInfo = &imageCreateInfos[imageIndex + j];
			imageCreateInfo->format = format;
			imageCreateInfo->usage = usageFlags;
			imageCreateInfo->image = &stack->images[j];
			imageCreateInfo->samples = sampleCountFlags;
			imageCreateInfo->tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo->width = stack->size.width;
			imageCreateInfo->height = stack->size.height;

			stack->formats[j] = format;
			stack->usage[j] = usageFlags;
			stack->aspectFlags[j] = imageAspectFlags;

			imageViews[imageIndex + j] = &stack->imageViews[j];
			aspectFlags[imageIndex + j] = imageAspectFlags;

			ImageReference_T* image = &(imagePool->imageStacks[i].imageReferences[j]);
			image->imageIndex = j;
			image->image = &(imagePool->imageStacks[i].images[j]);
			image->extent = (VkExtent2D*)&(imagePool->imageStacks[i].size.width);
			image->format = &(imagePool->imageStacks[i].formats[j]);
			image->imageView = &(imagePool->imageStacks[i].imageViews[j]);
			image->usage = &(imagePool->imageStacks[i].usage[j]);
			image->aspectFlags = &(imagePool->imageStacks[i].aspectFlags[j]);
			image->stack = &(imagePool->imageStacks[i]);

			*(layout.imageDescriptions[j].imageReference) = image;
			*layout.imageStack = stack;
		}
		imageIndex += imageCount;
	}

	createImages(handle, poolImageCount, imageCreateInfos, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imagePool->memory);
	for (uint32_t i = 0; i < poolImageCount; i++) {
		*(imageViews[i]) = createImageView(handle, *(imageCreateInfos[i].image), imageCreateInfos[i].format, aspectFlags[i]);
	}

	*pool = imagePool;

	fsFree(aspectFlags);
	fsFree(imageViews);
	fsFree(imageCreateInfos);
}

void imagePoolDestroy(ImagePool pool) {
	EngineHandle handle = pool->handle;

	for (uint32_t i = 0; i < pool->imageStackCount; i++) {
		const ImageStack_T stack = pool->imageStacks[i];
		for (uint32_t j = 0; j < stack.imageCount; j++) {
			vkDestroyImageView(handle->device, stack.imageViews[j], nullptr);
			vkDestroyImage(handle->device, stack.images[j], nullptr);
		}
		fsFree(stack.images);
		fsFree(stack.imageViews);
		fsFree(stack.formats);
		fsFree(stack.usage);
		fsFree(stack.aspectFlags);
		fsFree(stack.imageReferences);
	}

	fsFree(pool->imageStacks);
	vkFreeMemory(handle->device, pool->memory, nullptr);
	fsFree(pool);
}

#pragma endregion

#pragma region Modules

#pragma region RenderModules

bool_t checkSameSizeImageStacks(ImageStack a, ImageStack b) {
	if (a->size.width != b->size.width) {
		printf("Stacks have different width values!\n");
		return 0;
	}
	if (a->size.height != b->size.height) {
		printf("Stacks have different height values!\n");
		return 0;
	}
	if (a->size.widthScaling != b->size.widthScaling) {
		printf("Stacks have different width scaling!\n");
		return 0;
	}
	if (a->size.heightScaling != b->size.heightScaling) {
		printf("Stacks have different height scaling!\n");
		return 0;
	}
	if (a->size.widthScalingType != b->size.widthScalingType) {
		printf("Stacks have different width scaling type!\n");
		return 0;
	}
	if (a->size.heightScalingType != b->size.heightScalingType) {
		printf("Stacks have different height scaling type!\n");
		return 0;
	}

	return 1;
}

bool_t checkCompatibleImageStacks(ImageStack a, ImageStack b) {
	if (!checkSameSizeImageStacks(a, b)) {
		return 0;
	}
	if (a->sampleCount != b->sampleCount) {
		printf("Stacks have different sample count values!\n");
		return 0;
	}
	return 1;
}

bool_t checkCompatibleOutputImages(ImageStack a, uint32_t imageCount, ImageReference* images) {
	for (uint32_t i = 0; i < imageCount; i++) {
		ImageReference ref = images[i];
		if (*(ref->aspectFlags) != VK_IMAGE_ASPECT_COLOR_BIT) {
			printf("Output image doesn't have the color aspect!\n");
			return 0;
		}
		if ((*(ref->usage) & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
			printf("Output image doesn't have a color attachment!\n");
			return 0;
		}
		if (!checkCompatibleImageStacks(a, ref->stack)) {
			printf("Output image stack it is allocated from a stack compatible with the output stack!\n");
			return 0;
		}
	}
	return 1;
}

bool_t checkCompatibleResolveImages(uint32_t imageCount, ImageReference* images) {
	for (uint32_t i = 0; i < imageCount; i++) {
		ImageReference ref = images[i];
		if (*(ref->aspectFlags) != VK_IMAGE_ASPECT_COLOR_BIT) {
			printf("Resolve image doesn't have the color aspect!\n");
			return 0;
		}
		if ((*(ref->usage) & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) {
			printf("Resolve image doesn't have a color attachment!\n");
			return 0;
		}
		if (ref->stack->sampleCount != VK_SAMPLE_COUNT_1_BIT) {
			printf("Resolve image still has multiple samples!\n");
			return 0;
		}
	}
	return 1;
}

bool_t checkCompatibleDepthImage(ImageStack stack, ImageReference image) {
	if (!checkCompatibleImageStacks(stack, image->stack)) {
		printf("Depth image stack it is allocated from a stack compatible with the output stack!\n");
		return 0;
	}
	if (*image->aspectFlags != VK_IMAGE_ASPECT_DEPTH_BIT) {
		printf("Depth image doesn't have the depth aspect!\n");
		return 0;
	}
	if ((*image->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
		printf("Depth image doesn't have depth attachment!\n");
		return 0;
	}
	return 1;
}

void renderModuleGeometryCreate(EngineHandle handle, RenderModuleCreateInfo info, RenderModuleGeometry_T* module) {

	if (info.geometry.resolveImages) {
		if (!checkCompatibleResolveImages(info.output.outputImageCount, info.geometry.resolveImages)) {
			printf("Incompatible resolve image\n");
			abort();
		}
	}
	if (info.geometry.depthImage) {
		if (!checkCompatibleDepthImage(info.output.outputStack, info.geometry.depthImage)) {
			printf("Incompatible depth image");
			abort();
		}
	}

	module->depthImage = info.geometry.depthImage;

	if (info.output.outputImageCount) {
		if (info.geometry.resolveImages) {
			module->resolveImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.output.outputImageCount);
			if (info.geometry.resolveImages) {
				memcpy(module->resolveImages, info.geometry.resolveImages, sizeof(ImageReference) * info.output.outputImageCount);
			}
		}
	}
}

void renderModuleImageCombineCreate(EngineHandle handle, RenderModuleCreateInfo info, RenderModuleImageCombine_T* module) {
	if (info.output.outputStack->sampleCount != IMAGE_SAMPLE_COUNT_1) {
		printf("Image Combine modules may not have multisampled outputs\n");
		abort();
	}
}

void renderModulePostProcessCreate(EngineHandle handle, RenderModuleCreateInfo info, RenderModulePostProcess_T* module) {
	if (info.output.outputStack->sampleCount != IMAGE_SAMPLE_COUNT_1) {
		printf("Post Process modules may not have multisampled outputs\n");
		abort();
	}
}

void renderModuleCreate(EngineHandle handle, RenderModuleCreateInfo info, RenderModule* renderModule) {

	RenderModule_T* module = (RenderModule_T*)fsAllocate(sizeof(RenderModule_T));
	module->type = info.type;;
	module->handle = handle;

	if (info.directInputImages.directInputStack) {
		if (!checkCompatibleImageStacks(info.output.outputStack, info.directInputImages.directInputStack)) {
			printf("Incompatible in and/or output stacks\n");
			abort();
		}
	}
	if (!checkCompatibleOutputImages(info.output.outputStack, info.output.outputImageCount, info.output.outputImages)) {
		printf("Incompatible output image\n");
		abort();
	}

	module->directInputStack = info.directInputImages.directInputStack;
	module->directInputImageCount = info.directInputImages.directInputImageCount;
	if (module->directInputImageCount) {
		module->directInputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.directInputImages.directInputImageCount);
		memcpy(module->directInputImages, info.directInputImages.directInputImages, sizeof(ImageReference) * info.directInputImages.directInputImageCount);
	}
	module->shaderInputImageCount = info.shaderInputImages.shaderInputImageCount;
	if (module->shaderInputImageCount) {
		module->shaderInputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.shaderInputImages.shaderInputImageCount);
		memcpy(module->shaderInputImages, info.shaderInputImages.shaderInputImages, sizeof(ImageReference) * info.shaderInputImages.shaderInputImageCount);
	}

	module->shaderOutputImageCount = info.shaderOutputImages.shaderOutputImageCount;
	if (module->shaderOutputImageCount) {
		module->shaderOutputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.shaderOutputImages.shaderOutputImageCount);
		memcpy(module->shaderOutputImages, info.shaderOutputImages.shaderOutputImages, sizeof(ImageReference) * info.shaderOutputImages.shaderOutputImageCount);
	}

	module->outputStack = info.output.outputStack;
	module->outputImageCount = info.output.outputImageCount;
	if (module->outputImageCount) {
		module->outputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.output.outputImageCount);
		memcpy(module->outputImages, info.output.outputImages, sizeof(ImageReference) * info.output.outputImageCount);
	}

	switch (info.type) {
	case RENDER_MODULE_TYPE_GEOMETRY:
		renderModuleGeometryCreate(handle, info, &module->geometry);
		break;
	case RENDER_MODULE_TYPE_IMAGE_COMBINE:
		renderModuleImageCombineCreate(handle, info, &module->imageCombine);
		break;
	case RENDER_MODULE_TYPE_POST_PROCESS:
		renderModulePostProcessCreate(handle, info, &module->postProcess);
		break;
	default:
		abort();
		break;
	}

	*renderModule = module;
}

void addImagesToArray(uint32_t toAddCount, ImageReference* toAdd, uint32_t* imageCount, ImageReference** images) {
	ImageReference* uniqueImages = fsAllocate(sizeof(ImageReference) * toAddCount);
	uint32_t index = 0;
	for (uint32_t i = 0; i < toAddCount; i++) {
		ImageReference img = toAdd[i];
		bool_t unique = 1;
		for (uint32_t j = 0; j < *imageCount; j++) {
			if (img == (*images)[j]) {
				unique = 0;
				break;
			}
		}
		if (unique) {
			uniqueImages[index++] = img;
		}
	}

	uint32_t newSize = *imageCount + index;
	ImageReference* newImages = fsReallocate(*images, sizeof(ImageReference) * newSize);
	memcpy(newImages + (*imageCount), uniqueImages, sizeof(ImageReference) * index);

	*imageCount = newSize;
	*images = newImages;

	fsFree(uniqueImages);
}

void printHandleArray(uint32_t count, void** arr) {
	for (uint32_t i = 0; i < count; i++) {
		printf("[");
		if (arr[i]) {
			printf("#");
		} else {
			printf(" ");
		}
		printf("]\n");
	}
}

void renderModuleCheckSameSizeImagesInDirectAccessMode(RenderModule module, ImageStack* checkStack) {
	if (*checkStack == 0) {
		*checkStack = module->outputStack;
	} else {
		if (!checkSameSizeImageStacks(*checkStack, module->outputStack)) {
			abort();
		}
	}

	if (module->outputImageCount) {
		for (uint32_t i = 0; i < module->outputImageCount; i++) {
			if (!checkSameSizeImageStacks(module->outputStack, module->outputImages[i]->stack)) {
				abort();
			}
		}
	}
	if (module->directInputImageCount) {
		for (uint32_t i = 0; i < module->outputImageCount; i++) {
			if (!checkSameSizeImageStacks(module->outputStack, module->directInputImages[i]->stack)) {
				abort();
			}
		}
	}
}

void renderModuleCountUniqueImages(RenderModule module, uint32_t* imageCount, ImageReference** images) {

	addImagesToArray(module->directInputImageCount, module->directInputImages, imageCount, images);
	addImagesToArray(module->shaderInputImageCount, module->shaderInputImages, imageCount, images);
	addImagesToArray(module->shaderOutputImageCount, module->shaderOutputImages, imageCount, images);
	addImagesToArray(module->outputImageCount, module->outputImages, imageCount, images);

	switch (module->type) {
	case RENDER_MODULE_TYPE_GEOMETRY:
		addImagesToArray(1, &module->geometry.depthImage, imageCount, images);
		if (module->geometry.resolveImages) {
			addImagesToArray(module->outputImageCount, module->geometry.resolveImages, imageCount, images);
		}
		break;
	case RENDER_MODULE_TYPE_IMAGE_COMBINE:
		break;
	case RENDER_MODULE_TYPE_POST_PROCESS:
		break;
	}
}

void renderModuleDestroy(RenderModule renderModule) {
	fsFree(renderModule->directInputImages);
	fsFree(renderModule->shaderInputImages);
	fsFree(renderModule->shaderOutputImages);
	fsFree(renderModule->outputImages);

	switch (renderModule->type) {
	case RENDER_MODULE_TYPE_GEOMETRY:
		fsFree(renderModule->geometry.resolveImages);
		break;
	case RENDER_MODULE_TYPE_IMAGE_COMBINE:
		break;
	case RENDER_MODULE_TYPE_POST_PROCESS:
		break;
	}

	fsFree(renderModule);
}

#pragma endregion

#pragma region ComputeModules

void computeModuleComputeCreate(EngineHandle handle, ComputeModuleCreateInfo info, ComputeModuleCompute_T* module) {}

void computeModuleImageCombineCreate(EngineHandle handle, ComputeModuleCreateInfo info, ComputeModuleImageCombine_T* module) {}

void computeModulePostProcessCreate(EngineHandle handle, ComputeModuleCreateInfo info, ComputeModulePostProcess_T* module) {}

void computeModuleCreate(EngineHandle handle, ComputeModuleCreateInfo info, ComputeModule* computeModule) {

	ComputeModule_T* module = (ComputeModule_T*)fsAllocate(sizeof(ComputeModule_T));
	module->type = info.type;
	module->handle = handle;

	module->shaderInputImageCount = info.shaderInputImages.shaderInputImageCount;
	if (module->shaderInputImageCount) {
		module->shaderInputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.shaderInputImages.shaderInputImageCount);
		memcpy(module->shaderInputImages, info.shaderInputImages.shaderInputImages, sizeof(ImageReference) * info.shaderInputImages.shaderInputImageCount);
	}

	module->shaderOutputImageCount = info.shaderOutputImages.shaderOutputImageCount;
	if (module->shaderOutputImageCount) {
		module->shaderOutputImages = (ImageReference*)fsAllocate(sizeof(ImageReference) * info.shaderOutputImages.shaderOutputImageCount);
		memcpy(module->shaderOutputImages, info.shaderOutputImages.shaderOutputImages, sizeof(ImageReference) * info.shaderOutputImages.shaderOutputImageCount);
	}

	switch (info.type) {
	case COMPUTE_MODULE_TYPE_COMPUTE:
		computeModuleComputeCreate(handle, info, &module->compute);
		break;
	case COMPUTE_MODULE_TYPE_IMAGE_COMBINE:
		computeModuleImageCombineCreate(handle, info, &module->imageCombine);
		break;
	case COMPUTE_MODULE_TYPE_POST_PROCESS:
		computeModulePostProcessCreate(handle, info, &module->postProcess);
		break;
	default:
		abort();
		break;
	}

	*computeModule = module;
}

void computeModuleCountUniqueImages(ComputeModule module, uint32_t* imageCount, ImageReference** images) {
	addImagesToArray(module->shaderInputImageCount, module->shaderInputImages, imageCount, images);
	addImagesToArray(module->shaderOutputImageCount, module->shaderOutputImages, imageCount, images);
	switch (module->type) {
	case COMPUTE_MODULE_TYPE_COMPUTE:
		break;
	case COMPUTE_MODULE_TYPE_IMAGE_COMBINE:
		break;
	case COMPUTE_MODULE_TYPE_POST_PROCESS:
		break;
	}
}

void computeModuleDestroy(ComputeModule computeModule) {
	fsFree(computeModule->shaderInputImages);
	fsFree(computeModule->shaderOutputImages);

	switch (computeModule->type) {
	case COMPUTE_MODULE_TYPE_COMPUTE:
		break;
	case COMPUTE_MODULE_TYPE_IMAGE_COMBINE:
		break;
	case COMPUTE_MODULE_TYPE_POST_PROCESS:
		break;
	}

	fsFree(computeModule);
}

#pragma endregion

#pragma endregion

#pragma region RenderChain

#pragma region helper structs

typedef enum ImageState {
	IMAGE_STATE_UNDEFINED = 0,
	IMAGE_STATE_READ = 1,
	IMAGE_STATE_PRESERVE = 2,
	IMAGE_STATE_WRITE = 3,
	IMAGE_STATE_OVERWRITE = 4,
} ImageState;

typedef enum ImageAccess {
	IMAGE_ACCESS_NONE = 0,
	IMAGE_ACCESS_DIRECT = 1,
	IMAGE_ACCESS_INDIRECT = 2,
}ImageAccess;

typedef struct ImageLifetimeState {
	ImageState state;
	ImageAccess access;
} ImageLifetimeState;

#pragma endregion

#pragma region helper functions

bool_t checkUniqueImages(uint32_t imageCountA, ImageReference* imagesA, uint32_t imageCountB, ImageReference* imagesB) {
	for (uint32_t i = 0; i < imageCountA; i++) {
		ImageReference a = imagesA[i];
		for (uint32_t j = 0; j < imageCountB; j++) {
			if (a == imagesB[j]) {
				return 0;
			}
		}
	}
	return 1;
}

void printImageState(void* data) {
	ImageLifetimeState state = *(ImageLifetimeState*)data;
	switch (state.state) {
	case IMAGE_STATE_UNDEFINED:
		printf(" ");
		break;
	case IMAGE_STATE_READ:
		printf("R");
		break;
	case IMAGE_STATE_PRESERVE:
		printf("#");
		break;
	case IMAGE_STATE_WRITE:
		printf("W");
		break;
	case IMAGE_STATE_OVERWRITE:
		printf("O");
		break;
	}

	switch (state.access) {
	case IMAGE_ACCESS_NONE:
		printf(" ");
		break;
	case IMAGE_ACCESS_DIRECT:
		printf("=");
		break;
	case IMAGE_ACCESS_INDIRECT:
		printf(":");
		break;
	}
}

void printModule(void* data) {
	ImageReference* module = (ImageReference*)data;
	if (*module) {
		printf("#");
	} else {
		printf(" ");
	}
}

#pragma endregion

#pragma region module order

typedef	void* Module;

void prepareModuleOrder(const uint32_t moduleCount, Module* modules, _DataGrid* const grid) {
	dataGridCreate(moduleCount, moduleCount, grid);
	for (uint32_t i = 0; i < moduleCount; i++) {
		dataGridSet(i, 0, (modules + i), grid);
	}
}

void buildModuleOrder(const uint32_t dependencyCount, ModuleDependency* const dependencies, _DataGrid* const grid) {
	const uint32_t moduleCount = dataGridGetWidth(grid);
	for (uint32_t srcY = 0; srcY < moduleCount; srcY++) {
		for (uint32_t srcX = 0; srcX < moduleCount; srcX++) {
			RenderModule module;
			dataGridGet(srcX, srcY, &module, grid);
			for (uint32_t i = 0; i < dependencyCount; i++) {
				ModuleDependency dep = dependencies[i];
				if (dep.src == module) {
					for (uint32_t dstY = srcY; dstY < moduleCount; dstY++) {
						for (uint32_t dstX = 0; dstX < moduleCount; dstX++) {
							RenderModule dst;
							dataGridGet(dstX, dstY, &dst, grid);
							if (dep.dst == dst) {
								dataGridSet(srcX, srcY, nullptr, grid);
								if (!dataGridSet(srcX, dstY + 1, &module, grid)) {
									printf("Error in building dependency tree!\n");
									abort();
								}
							}
						}
					}
				}
			}
		}
	}
}

void extractModuleOrder(uint32_t moduleCount, Module* modules, _DataGrid* const grid) {
	uint32_t cellCount = dataGridGetCellCount(grid);
	uint32_t moduleIndex = 0;
	for (uint32_t i = 0; i < cellCount; i++) {
		RenderModule cellContent;
		dataGridGetSerialHorizontalZpatern(i, &cellContent, grid);
		if (cellContent) {
			modules[moduleCount - ++moduleIndex] = cellContent;
		}
	}
	dataGridDestroy(grid);
}

void setImageStates(uint32_t imageCount, ImageReference* images, uint32_t usedImageCount, ImageReference* usedImages, uint32_t index, ImageLifetimeState state, _DataGrid* const imageStates) {
	for (uint32_t j = 0; j < imageCount; j++) {
		ImageReference img = images[j];
		for (uint32_t k = 0; k < usedImageCount; k++) {
			if (usedImages[k] == img) {
				dataGridSet(index, k, &state, imageStates);
			}
		}
	}
}

#pragma endregion

void renderChainCreate(EngineHandle handle, RenderChainCreateInfo info, RenderChain* renderChain) {

	RenderChain chain = (RenderChain)fsAllocate(sizeof(RenderChain_T));
	chain->handle = handle;

#pragma region set framebuffer size
	chain->framebufferSize = info.renderStepImageSize;
	updateImageSize(handle, &chain->framebufferSize);
#pragma endregion

#pragma region build module order
	//Modules and Order
	chain->preRenderComputeModuleCount = info.preRenderComputeModuleCount;
	chain->preRenderComputeModules = (ComputeModule*)fsAllocate(sizeof(ComputeModule) * info.preRenderComputeModuleCount);

	chain->renderModuleCount = info.renderStepCount;
	chain->renderModules = (RenderModule*)fsAllocate(sizeof(RenderModule) * info.renderStepCount);

	chain->postRenderComputeModuleCount = info.postRenderComputeModuleCount;
	chain->postRenderComputeModules = (ComputeModule*)fsAllocate(sizeof(ComputeModule) * info.postRenderComputeModuleCount);



	const uint32_t preRenderComputeModuleCount = chain->preRenderComputeModuleCount;
	if (preRenderComputeModuleCount) {
		_DataGrid preRenderComputeModuleOrderGrid = { .dataSize = sizeof(ComputeModule) };
		prepareModuleOrder(preRenderComputeModuleCount, info.preRenderComputeModules, &preRenderComputeModuleOrderGrid);
		buildModuleOrder(info.preRenderDependencyCount, (ModuleDependency*)info.preRenderDependencies, &preRenderComputeModuleOrderGrid);
		extractModuleOrder(preRenderComputeModuleCount, chain->preRenderComputeModules, &preRenderComputeModuleOrderGrid);
	}

	const uint32_t renderModuleCount = chain->renderModuleCount;
	_DataGrid renderModuleOrder = { .dataSize = sizeof(RenderModule) };
	if (renderModuleCount) {
		_DataGrid renderModuleOrderGrid = { .dataSize = sizeof(RenderModule) };
		prepareModuleOrder(renderModuleCount, info.renderSteps, &renderModuleOrderGrid);
		buildModuleOrder(info.renderStepDependencyCount, (ModuleDependency*)info.renderStepDependencies, &renderModuleOrderGrid);
		dataGridClone(&renderModuleOrderGrid, &renderModuleOrder);
		extractModuleOrder(renderModuleCount, chain->renderModules, &renderModuleOrderGrid);
	}

	const uint32_t postRenderComputeModuleCount = chain->postRenderComputeModuleCount;
	if (postRenderComputeModuleCount) {
		_DataGrid postRenderComputeModuleOrderGrid = { .dataSize = sizeof(ComputeModule) };
		prepareModuleOrder(postRenderComputeModuleCount, info.postRenderComputeModules, &postRenderComputeModuleOrderGrid);
		buildModuleOrder(info.postRenderDependencyCount, (ModuleDependency*)info.postRenderDependencies, &postRenderComputeModuleOrderGrid);
		extractModuleOrder(postRenderComputeModuleCount, chain->postRenderComputeModules, &postRenderComputeModuleOrderGrid);
	}

#pragma endregion

#pragma region aquire images

	uint32_t usedImageCount = info.input.inputImageCount + info.output.outputImageCount;
	ImageReference* usedImages = fsAllocate(sizeof(ImageReference) * usedImageCount);
	VkExtent2D renderModulesDirectImageSize = { 0 };
	{
		uint32_t index = 0;
		for (uint32_t i = 0; i < info.input.inputImageCount; i++) {
			ImageReference ref = info.input.inputImages[i];
			for (uint32_t j = 0; j < usedImageCount; j++) {
				if (ref == usedImages[j]) {
					continue;
				}
				usedImages[index++] = ref;
			}
		}
		for (uint32_t i = 0; i < info.output.outputImageCount; i++) {
			ImageReference ref = info.output.outputImages[i];
			for (uint32_t j = 0; j < usedImageCount; j++) {
				if (ref == usedImages[j]) {
					continue;
				}
				usedImages[index++] = ref;
			}
		}
		usedImageCount = index;
	}

	uint32_t renderModuleImageCount = 0;
	{
		for (uint32_t i = 0; i < preRenderComputeModuleCount; i++) {
			computeModuleCountUniqueImages(chain->preRenderComputeModules[i], &usedImageCount, &usedImages);
		}
		uint32_t startImageCount = usedImageCount;
		ImageStack checkStack = { 0 };
		for (uint32_t i = 0; i < renderModuleCount; i++) {
			renderModuleCountUniqueImages(chain->renderModules[i], &usedImageCount, &usedImages);
			renderModuleCheckSameSizeImagesInDirectAccessMode(chain->renderModules[i], &checkStack);
		}
		renderModuleImageCount = usedImageCount - startImageCount;

		for (uint32_t i = 0; i < postRenderComputeModuleCount; i++) {
			computeModuleCountUniqueImages(chain->postRenderComputeModules[i], &usedImageCount, &usedImages);
		}
	}

#pragma endregion

#pragma region generate image lifetimes 

	_DataGrid imageStates = { .dataSize = sizeof(ImageLifetimeState) };
	const uint32_t renderChainModuleCount = preRenderComputeModuleCount + renderModuleCount + postRenderComputeModuleCount + 2; /*2 extra for start and end of chain*/
	dataGridCreate(renderChainModuleCount, usedImageCount, &imageStates);

	ImageLifetimeState inputWriteState = { .state = IMAGE_STATE_WRITE, .access = IMAGE_ACCESS_NONE };
	ImageLifetimeState outputReadState = { .state = IMAGE_STATE_READ, .access = IMAGE_ACCESS_NONE };
	setImageStates(info.input.inputImageCount, info.input.inputImages, usedImageCount, usedImages, 0, inputWriteState, &imageStates);
	setImageStates(info.output.outputImageCount, info.output.outputImages, usedImageCount, usedImages, dataGridGetWidth(&imageStates) - 1, outputReadState, &imageStates);

	{
		const uint32_t startIndex = 1;
		uint32_t index = startIndex;
		for (uint32_t i = 0; i < preRenderComputeModuleCount; (index++ + i++)) {
			ComputeModule module = chain->preRenderComputeModules[i];
			ImageLifetimeState shaderReadState = { .state = IMAGE_STATE_READ, .access = IMAGE_ACCESS_INDIRECT };
			ImageLifetimeState shaderWriteState = { .state = IMAGE_STATE_WRITE, .access = IMAGE_ACCESS_INDIRECT };
			setImageStates(module->shaderInputImageCount, module->shaderInputImages, usedImageCount, usedImages, index, shaderReadState, &imageStates);
			setImageStates(module->shaderOutputImageCount, module->shaderOutputImages, usedImageCount, usedImages, index, shaderWriteState, &imageStates);
		}
		for (uint32_t i = 0; i < renderModuleCount; (index++ + i++)) {
			RenderModule module = chain->renderModules[i];

			ImageLifetimeState shaderReadState = { .state = IMAGE_STATE_READ, .access = IMAGE_ACCESS_INDIRECT };
			ImageLifetimeState shaderWriteState = { .state = IMAGE_STATE_WRITE, .access = IMAGE_ACCESS_INDIRECT };
			ImageLifetimeState directReadState = { .state = IMAGE_STATE_READ, .access = IMAGE_ACCESS_DIRECT };
			ImageLifetimeState directWriteState = { .state = IMAGE_STATE_WRITE, .access = IMAGE_ACCESS_DIRECT };


			setImageStates(module->directInputImageCount, module->directInputImages, usedImageCount, usedImages, index, directReadState, &imageStates);
			setImageStates(module->shaderInputImageCount, module->shaderInputImages, usedImageCount, usedImages, index, shaderReadState, &imageStates);

			setImageStates(module->shaderOutputImageCount, module->shaderOutputImages, usedImageCount, usedImages, index, shaderWriteState, &imageStates);
			setImageStates(module->outputImageCount, module->outputImages, usedImageCount, usedImages, index, directWriteState, &imageStates);
			if (module->type == RENDER_MODULE_TYPE_GEOMETRY) {
				setImageStates(1, &module->geometry.depthImage, usedImageCount, usedImages, index, directWriteState, &imageStates);
				if (module->geometry.resolveImages) {
					setImageStates(module->outputImageCount, module->geometry.resolveImages, usedImageCount, usedImages, index, directWriteState, &imageStates);
				}
			}
		}
		for (uint32_t i = 0; i < postRenderComputeModuleCount; (index++ + i++)) {
			ComputeModule module = chain->postRenderComputeModules[i];
			ImageLifetimeState shaderReadState = { .state = IMAGE_STATE_READ, .access = IMAGE_ACCESS_INDIRECT };
			ImageLifetimeState shaderWriteState = { .state = IMAGE_STATE_WRITE, .access = IMAGE_ACCESS_INDIRECT };
			setImageStates(module->shaderInputImageCount, module->shaderInputImages, usedImageCount, usedImages, index, shaderReadState, &imageStates);
			setImageStates(module->shaderOutputImageCount, module->shaderOutputImages, usedImageCount, usedImages, index, shaderWriteState, &imageStates);
		}
	}

	for (uint32_t imageIndex = 0; imageIndex < usedImageCount; imageIndex++) {
		ImageState currentState = IMAGE_STATE_UNDEFINED;

		for (uint32_t moduleIndex = 0; moduleIndex < renderChainModuleCount; moduleIndex++) {
			ImageLifetimeState state = { 0 };
			dataGridGet(moduleIndex, imageIndex, &state, &imageStates);

			switch (state.state) {
			case IMAGE_STATE_UNDEFINED:
				break;
			case IMAGE_STATE_READ:
				currentState = IMAGE_STATE_UNDEFINED;
				break;
			case IMAGE_STATE_WRITE:
				if (currentState == IMAGE_STATE_WRITE) {
					ImageLifetimeState setState = { .state = IMAGE_STATE_OVERWRITE,.access = state.access };
					dataGridSet(moduleIndex, imageIndex, &setState, &imageStates);
				}
				currentState = IMAGE_STATE_WRITE;
				break;
			}
		}
	}

	//check for images to be written to
	for (uint32_t imageIndex = 0; imageIndex < usedImageCount; imageIndex++) {
		ImageState currentState = IMAGE_STATE_UNDEFINED;

		for (uint32_t j = 0; j < renderChainModuleCount; j++) {
			uint32_t moduleIndex = renderChainModuleCount - j - 1;

			ImageLifetimeState state = { 0 };
			dataGridGet(moduleIndex, imageIndex, &state, &imageStates);

			switch (state.state) {
			case IMAGE_STATE_UNDEFINED:
				if (currentState == IMAGE_STATE_READ) {
					ImageLifetimeState setState = { .state = IMAGE_STATE_PRESERVE,.access = state.access };
					dataGridSet(moduleIndex, imageIndex, &setState, &imageStates);
				}
				break;
			case IMAGE_STATE_READ:
				currentState = IMAGE_STATE_READ;
				break;
			case IMAGE_STATE_WRITE:
				currentState = IMAGE_STATE_UNDEFINED;
				break;
			case IMAGE_STATE_OVERWRITE:
				break;
			}
		}
		if (currentState == IMAGE_STATE_READ) {
			printf("Image is read but not written to!");
			abort();
		}
	}

	//remove preserve states if an image is not written or read by the renderModules
	for (uint32_t imageIndex = 0; imageIndex < usedImageCount; imageIndex++) {

		//check if the images is edited by the renderModules
		bool_t justPreserved = 1;
		for (uint32_t j = 0; j < renderChainModuleCount; j++) {
			uint32_t moduleIndex = renderChainModuleCount - j - 1;

			if (moduleIndex < 1 + preRenderComputeModuleCount) {
				continue;
			}
			if (moduleIndex >= 1 + preRenderComputeModuleCount + renderModuleCount) {
				continue;
			}

			ImageLifetimeState state = { 0 };
			dataGridGet(moduleIndex, imageIndex, &state, &imageStates);

			if (state.state != IMAGE_STATE_PRESERVE) {
				justPreserved = 0;
			}
		}
		if (!justPreserved) {
			continue;
		}

		//remove preserve states
		for (uint32_t j = 0; j < renderChainModuleCount; j++) {
			uint32_t moduleIndex = renderChainModuleCount - j - 1;

			if (moduleIndex < 1 + preRenderComputeModuleCount) {
				continue;
			}
			if (moduleIndex >= 1 + preRenderComputeModuleCount + renderModuleCount) {
				continue;
			}
			ImageLifetimeState state = { .state = IMAGE_STATE_UNDEFINED, .access = IMAGE_ACCESS_NONE };
			dataGridSet(moduleIndex, imageIndex, &state, &imageStates);
		}
	}

#pragma endregion

#pragma region build image attachments

	//build image attachments
	VkAttachmentDescription* attachments = fsAllocate(sizeof(VkAttachmentDescription) * renderModuleImageCount);
	ImageReference* renderModuleUsedImages = fsAllocate(sizeof(ImageReference) * renderModuleImageCount);
	{
		uint32_t attachmentIndex = 0;
		for (uint32_t imageIndex = 0; imageIndex < usedImageCount; imageIndex++) {

			//check if image is used by any render modules
			bool_t imageUsed = 0;
			for (uint32_t moduleIndex = 1 + preRenderComputeModuleCount; moduleIndex < 1 + preRenderComputeModuleCount + renderModuleCount; moduleIndex++) {
				ImageLifetimeState state = { 0 };
				dataGridGet(moduleIndex, imageIndex, &state, &imageStates);
				if (state.state != IMAGE_STATE_UNDEFINED) {
					imageUsed = 1;
				}
			}
			if (!imageUsed) {
				continue;
			}
			renderModuleUsedImages[attachmentIndex] = usedImages[imageIndex];


			bool_t imageUsedInPostProcessing = 0;
			for (uint32_t moduleIndex = 1 + preRenderComputeModuleCount + renderModuleCount; moduleIndex < renderChainModuleCount - 1; moduleIndex++) {
				ImageLifetimeState postProcesState = { 0 };
				dataGridGet(moduleIndex, imageIndex, &postProcesState, &imageStates);
				if (postProcesState.state != IMAGE_STATE_UNDEFINED && postProcesState.state != IMAGE_STATE_PRESERVE) {
					imageUsedInPostProcessing = 1;
					break;
				}
			}

			bool_t imageGenerated = 0;
			for (uint32_t moduleIndex = 1; moduleIndex < 1 + preRenderComputeModuleCount; moduleIndex++) {
				ImageLifetimeState postProcesState = { 0 };
				dataGridGet(moduleIndex, imageIndex, &postProcesState, &imageStates);
				if (postProcesState.state != IMAGE_STATE_UNDEFINED && postProcesState.state != IMAGE_STATE_PRESERVE) {
					imageGenerated = 1;
					break;
				}
			}

			ImageLifetimeState firstAccess = { 0 };
			for (uint32_t moduleIndex = 1 + preRenderComputeModuleCount; moduleIndex < 1 + preRenderComputeModuleCount + renderModuleCount; moduleIndex++) {
				ImageLifetimeState _firstAccess = { 0 };
				dataGridGet(moduleIndex, imageIndex, &_firstAccess, &imageStates);
				if (_firstAccess.state != IMAGE_STATE_UNDEFINED && _firstAccess.state != IMAGE_STATE_PRESERVE) {
					firstAccess = _firstAccess;
					break;
				}

			}

			bool_t imageExported = 0;
			ImageLifetimeState exportState = { 0 };
			dataGridGet(renderChainModuleCount - 1, imageIndex, &exportState, &imageStates);
			imageExported = (exportState.state == IMAGE_STATE_READ);

			bool_t imageImported = 0;
			ImageLifetimeState importState = { 0 };
			dataGridGet(0, imageIndex, &importState, &imageStates);
			imageImported = (importState.state == IMAGE_STATE_READ);

			bool_t imageWritten = 0;
			for (uint32_t moduleIndex = 1; moduleIndex < 1 + preRenderComputeModuleCount; moduleIndex++) {
				ImageLifetimeState generatedState = { 0 };
				dataGridGet(moduleIndex, imageIndex, &generatedState, &imageStates);
				if (generatedState.state == IMAGE_STATE_WRITE || generatedState.state != IMAGE_STATE_OVERWRITE) {
					imageWritten = 1;
					break;
				}
				if (generatedState.state == IMAGE_STATE_READ) {
					break;
				}
			}

			ImageReference image = usedImages[imageIndex];
			bool_t depthImage = *(image->aspectFlags) == VK_IMAGE_ASPECT_DEPTH_BIT;

			VkAttachmentDescription* attachment = attachments + attachmentIndex++;
			attachment->format = *image->format;
			attachment->samples = image->stack->sampleCount;
			attachment->format = *usedImages[imageIndex]->format;
			attachment->loadOp = (imageWritten || imageImported) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->storeOp = (imageUsedInPostProcessing || imageExported) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment->flags = 0;
			if (imageWritten) {
				if (!depthImage) {
					if (firstAccess.access == IMAGE_ACCESS_INDIRECT) {
						if (firstAccess.state == IMAGE_STATE_READ) {
							attachment->initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						} else {
							attachment->initialLayout = VK_IMAGE_LAYOUT_GENERAL;
						}
					} else if (firstAccess.access == IMAGE_ACCESS_DIRECT) {
						attachment->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
				} else {
					if (firstAccess.access == IMAGE_ACCESS_INDIRECT) {
						attachment->initialLayout = VK_IMAGE_LAYOUT_GENERAL;
					} else if (firstAccess.access == IMAGE_ACCESS_DIRECT) {
						attachment->initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					}
				}
			}
			if (imageUsedInPostProcessing && imageWritten) {
				attachment->finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			} else if (imageUsedInPostProcessing && !imageWritten) {
				attachment->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			} else if (!imageExported && !imageUsedInPostProcessing && !depthImage) {
				attachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			} else if (!imageExported && !imageUsedInPostProcessing && depthImage) {
				attachment->finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
		}

		if (attachmentIndex != renderModuleImageCount) {
			printf("failed to build image attachments");
			abort();
		}
	}


#pragma endregion

#pragma region build subpasses

	//build renderPass for the renderModules
	VkSubpassDescription* subpasses = fsAllocate(sizeof(VkSubpassDescription) * renderModuleCount);
	for (uint32_t renderModuleIndex = 0; renderModuleIndex < renderModuleCount; renderModuleIndex++) {
		uint32_t moduleIndex = 1 + preRenderComputeModuleCount + renderModuleIndex;

		RenderModule module = chain->renderModules[renderModuleIndex];

		VkAttachmentReference* inputAttachments = nullptr;
		uint32_t inputAttachmentIndex = 0;
		if (module->directInputImageCount) {
			inputAttachments = fsAllocate(sizeof(VkAttachmentReference) * module->directInputImageCount);
			for (uint32_t directInputIndex = 0; directInputIndex < module->directInputImageCount; directInputIndex++) {
				for (uint32_t imageIndex = 0; imageIndex < renderModuleImageCount; imageIndex++) {
					if (module->directInputImages[directInputIndex] == renderModuleUsedImages[imageIndex]) {
						inputAttachments[inputAttachmentIndex].attachment = imageIndex;
						inputAttachments[inputAttachmentIndex].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						inputAttachmentIndex++;
					}
				}
			}
		}

		VkAttachmentReference* outputAttachments = nullptr;
		uint32_t outputAttachmentIndex = 0;
		if (module->outputImageCount) {
			outputAttachments = fsAllocate(sizeof(VkAttachmentReference) * module->outputImageCount);
			for (uint32_t directInputIndex = 0; directInputIndex < module->outputImageCount; directInputIndex++) {
				for (uint32_t imageIndex = 0; imageIndex < renderModuleImageCount; imageIndex++) {
					if (module->outputImages[directInputIndex] == renderModuleUsedImages[imageIndex]) {
						outputAttachments[outputAttachmentIndex].attachment = imageIndex;
						outputAttachments[outputAttachmentIndex].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						outputAttachmentIndex++;
					}
				}
			}
		}

		uint32_t preserveCount = 0;
		uint32_t* preserveAttachments = nullptr;

		for (uint32_t imageIndex = 0; imageIndex < usedImageCount; imageIndex++) {

			ImageLifetimeState state = { 0 };
			dataGridGet(moduleIndex, imageIndex, &state, &imageStates);

			if (state.state == IMAGE_STATE_PRESERVE) {

				for (uint32_t i = 0; i < renderModuleImageCount; i++) {
					if (usedImages[imageIndex] == renderModuleUsedImages[i]) {

						preserveAttachments = fsReallocate(preserveAttachments, sizeof(uint32_t) * ++preserveCount);

						preserveAttachments[preserveCount - 1] = i;


						break;
					}
				}

			}

		}

		VkSubpassDescription* subpass = subpasses + renderModuleIndex;
		subpass->flags = 0;
		subpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass->inputAttachmentCount = module->directInputImageCount;
		subpass->pInputAttachments = inputAttachments;
		subpass->colorAttachmentCount = module->outputImageCount;
		subpass->pColorAttachments = outputAttachments;
		subpass->preserveAttachmentCount = preserveCount;
		subpass->pPreserveAttachments = preserveAttachments;
		subpass->pDepthStencilAttachment = nullptr;
		subpass->pResolveAttachments = nullptr;


		//find depth and resolve attachments;
		if (module->type == RENDER_MODULE_TYPE_GEOMETRY) {
			uint32_t resolveIndex = 0;
			VkAttachmentReference* resolveRefs = nullptr;
			if (module->geometry.resolveImages) {
				resolveRefs = fsAllocate(sizeof(VkAttachmentReference) * module->outputImageCount);
			}
			for (uint32_t i = 0; i < renderModuleImageCount; i++) {
				if (renderModuleUsedImages[i] == module->geometry.depthImage) {
					VkAttachmentReference* depthRef = fsAllocate(sizeof(VkAttachmentDescription));
					depthRef->attachment = i;
					depthRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					subpass->pDepthStencilAttachment = depthRef;
					continue;
				}
				if (!module->geometry.resolveImages) {
					continue;
				}

				for (uint32_t j = 0; j < module->outputImageCount; j++) {
					if (renderModuleUsedImages[i] == module->geometry.resolveImages[j]) {
						VkAttachmentReference resolveRef = { 0 };
						resolveRef.attachment = i;
						resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						resolveRefs[resolveIndex++] = resolveRef;
					}
				}
			}

			subpass->pResolveAttachments = resolveRefs;
		}

	}

#pragma endregion 

#pragma region build subpass dependencies

	uint32_t dependencyCount = info.renderStepDependencyCount;
	VkSubpassDependency* dependencies = fsAllocate(sizeof(VkSubpassDependency) * dependencyCount);
	for (uint32_t i = 0; i < dependencyCount; i++) {
		RenderStepDependency dep = info.renderStepDependencies[i];
		VkSubpassDependency* pDep = dependencies + i;
		pDep->dependencyFlags = 0;
		for (uint32_t j = 0; j < renderModuleCount; j++) {
			if (chain->renderModules[j] == dep.src) {
				pDep->srcSubpass = j;
			}
			if (chain->renderModules[j] == dep.dst) {
				pDep->dstSubpass = j;
			}
		}
		pDep->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		pDep->dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		pDep->srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		pDep->dstAccessMask =
			VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
			VK_ACCESS_SHADER_READ_BIT |
			VK_ACCESS_SHADER_WRITE_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

#pragma endregion

#pragma region build renderpass

	VkRenderPassCreateInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.flags = 0;
	renderPassInfo.subpassCount = renderModuleCount;
	renderPassInfo.attachmentCount = renderModuleImageCount;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = renderModuleCount;
	renderPassInfo.pSubpasses = subpasses;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = nullptr;

	VkCheck(vkCreateRenderPass(DEVICE(handle), &renderPassInfo, nullptr, &chain->renderPass));

#pragma endregion

#pragma region build framebuffer

	VkImageView* imageViewAttachments = fsAllocate(sizeof(VkImageView) * renderModuleImageCount);
	for (uint32_t i = 0; i < renderModuleImageCount; i++) {
		imageViewAttachments[i] = *renderModuleUsedImages[i]->imageView;
	}

	VkFramebufferCreateInfo framebufferInfo = { 0 };
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.flags = 0;
	framebufferInfo.renderPass = chain->renderPass;
	framebufferInfo.layers = 1;
	framebufferInfo.width = chain->framebufferSize.width;
	framebufferInfo.height = chain->framebufferSize.height;
	framebufferInfo.attachmentCount = renderModuleImageCount;
	framebufferInfo.pAttachments = imageViewAttachments;

	VkCheck(vkCreateFramebuffer(DEVICE(handle), &framebufferInfo, nullptr, &chain->framebuffer));


#pragma endregion 

#pragma region cleanup

	fsFree(dependencies);
	for (uint32_t i = 0; i < renderModuleCount; i++) {
		VkSubpassDescription subpass = subpasses[i];
		fsFree(subpass.pColorAttachments);
		fsFree(subpass.pDepthStencilAttachment);
		fsFree(subpass.pInputAttachments);
		fsFree(subpass.pPreserveAttachments);
		fsFree(subpass.pResolveAttachments);
	}
	fsFree(subpasses);

	fsFree(imageViewAttachments);
	fsFree(attachments);

	fsFree(usedImages);
	fsFree(renderModuleUsedImages);
	dataGridDestroy(&renderModuleOrder);
	dataGridDestroy(&imageStates);

#pragma endregion 

	* renderChain = chain;
}

void renderChainDestroy(RenderChain renderChain) {

	vkDestroyRenderPass(DEVICE(renderChain->handle), renderChain->renderPass, nullptr);
	vkDestroyFramebuffer(DEVICE(renderChain->handle), renderChain->framebuffer, nullptr);

	fsFree(renderChain->preRenderComputeModules);
	fsFree(renderChain->renderModules);
	fsFree(renderChain->postRenderComputeModules);

	fsFree(renderChain);

}

#pragma endregion

#pragma region GraphicsChain





#pragma endregion
