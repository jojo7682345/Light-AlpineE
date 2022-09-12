#include "shaders.h"
#include "stdio.h"

char* readFile(const char* filename, size_t* fileSize) {
	FILE* file;
	fopen_s(&file, filename, "rb");
	if (file == NULL) {
		//error
		abort();
		return nullptr;
	}
	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	char* buffer = (char*)fsAllocate(size + 1);
	fread(buffer, 1, size, file);
	fclose(file);
	*fileSize = size;
	return buffer;
}

VkShaderModule shaderModuleCreate(const char* shaderFile, EngineHandle handle) {
	size_t codeSize = 0;
	char* code = readFile(shaderFile, &codeSize);

	VkShaderModuleCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codeSize;
	createInfo.pCode = (uint32_t*) code;
	VkShaderModule shaderModule;
	VkCheck(vkCreateShaderModule(DEVICE(handle), &createInfo, nullptr, &shaderModule));

	fsFree(code);

	return shaderModule;
}

void shaderModuleDestroy(VkShaderModule shader, EngineHandle handle) {
	vkDestroyShaderModule(DEVICE(handle), shader, nullptr);
}