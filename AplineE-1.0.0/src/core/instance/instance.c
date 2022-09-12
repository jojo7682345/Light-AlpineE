#include "instance.h"
#include "../debug/debug.h"

void getRequiredInstanceExtensions(uint32_t* extensionCount, char*** extensions) {
	//get glfw extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//get debug extensions
	uint32_t debugExtensionCount = 0;
	const char* const* debugExtensions = debugGetRequiredExtensions(&debugExtensionCount);

	//
	*extensionCount = glfwExtensionCount + debugExtensionCount;
	*extensions = (char**)fsAllocate(sizeof(char*) * *extensionCount);

	//populate extensions
	memcpy(*extensions, glfwExtensions, sizeof(char*) * glfwExtensionCount);
	memcpy(*extensions + glfwExtensionCount, debugExtensions, sizeof(char*) * debugExtensionCount);

}

void instanceCreate(EngineHandle handle, EngineSettings settings) {

	uint32_t validationLayerCount = 0;
	const char* const* validationLayers = debugGetRequiredValidationLayers(&validationLayerCount);

	//create the vulkan instance
	VkApplicationInfo applicationInfo = { 0 };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.engineVersion = getEngineVersion();
	applicationInfo.pEngineName = "AlpineE";
	applicationInfo.apiVersion = VK_API_VERSION_1_3;
	applicationInfo.applicationVersion = settings.gameVersion;
	applicationInfo.pApplicationName = settings.gameTitle;
	applicationInfo.pNext = 0;

	VkInstanceCreateInfo instanceCreateInfo = { 0 };
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pNext = 0;
	instanceCreateInfo.enabledLayerCount = validationLayerCount;
	instanceCreateInfo.ppEnabledLayerNames = validationLayers;

	uint32_t extensionCount = 0;
	char** extensions = nullptr;
	getRequiredInstanceExtensions(&extensionCount, &extensions);
	instanceCreateInfo.enabledExtensionCount = extensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = extensions;

	VkCheck(vkCreateInstance(&instanceCreateInfo, NULL, &INSTANCE(handle)));
	fsFree(extensions);
}

void instanceDestroy(EngineHandle handle) {
	vkDestroyInstance(INSTANCE(handle), nullptr);
}


