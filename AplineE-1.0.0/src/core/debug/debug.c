#include "debug.h"
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <MemoryUtilities/MemoryUtilities.h>
#undef APIENTRY
#include <windows.h>

#ifdef NDEBUG
const uint16_t enableValidationLayers = 0;
#else
const uint16_t enableValidationLayers = 1;
#endif

const char* const validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};
const char* const validationExtensions[] = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};
const uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(const char*);
const uint32_t validationExtensionCount = sizeof(validationExtensions) / sizeof(const char*);

const char* const* debugGetRequiredExtensions(uint32_t* extensionCount) {
	if (!enableValidationLayers) {
		*extensionCount = 0;
		return nullptr;
	}
	*extensionCount = validationExtensionCount;
	return validationExtensions;
}

const char* const* debugGetRequiredValidationLayers(uint32_t* layerCount) {
	if (!enableValidationLayers) {
		*layerCount = 0;
		return nullptr;
	}
	*layerCount = validationExtensionCount;
	return validationLayers;
}

int debugCheckValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	VkLayerProperties* availableLayers = (VkLayerProperties*)fsAllocate(sizeof(VkLayerProperties) * layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);


	for (uint32_t i = 0; i < validationLayerCount; i++) {
		const char* layerName = validationLayers[i];

		int layerFound = 0;

		for (uint32_t j = 0; j < layerCount; j++) {
			const VkLayerProperties layerProperties = availableLayers[j];
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = 1;
				break;
			}
		}

		if (!layerFound) {
			fsFree(availableLayers);
			return 0;
		}
	}
	fsFree(availableLayers);
	return 1;
}

void debugCheckSupport() {
	if (enableValidationLayers && !debugCheckValidationLayerSupport()) {
		printf("Validation Layers Requested, but not available!\n");
		abort();
	}
}

#define BLACK			0
#define BLUE			1
#define GREEN			2
#define CYAN			3
#define RED				4
#define MAGENTA			5
#define BROWN			6
#define LIGHTGRAY		7
#define DARKGRAY		8
#define LIGHTBLUE		9
#define LIGHTGREEN		10
#define LIGHTCYAN		11
#define LIGHTRED		12
#define LIGHTMAGENTA	13
#define YELLOW			14
#define WHITE			15


#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERRORS 1
#define DEBUG_LEVEL_ERRORS_WARNINGS 2
#define DEBUG_LEVEL_ERRORS_WARNINGS_INFO 3
#define DEBUG_LEVEL_ALL 4

#define DEBUG_LEVEL DEBUG_LEVEL_ERRORS_WARNINGS_INFO

const char* putSentencesOnNewLines(const char* str) {
	size_t len = strlen(str);
	char* period = nullptr;
	while (period = memchr(str, '.', len)) {
		if (*(period + 1) == ' ') {
			*(period + 1) = '\n';
		}
		len -= period - str + 1;
		str = period + 1;
	}
	return str;
}
const char* putColonsOnNewLines(const char* str) {
	size_t len = strlen(str);
	char* period = nullptr;
	while (period = memchr(str, ':', len)) {
		if (*(period + 1) == ' ') {
			*(period + 1) = '\n';
		}
		len -= period - str + 1;
		str = period + 1;
	}
	return str;
}
const char* putCommasOnNewLines(const char* str) {
	size_t len = strlen(str);
	char* period = nullptr;
	while (period = memchr(str, ',', len)) {
		if (*(period + 1) == ' ') {
			*(period + 1) = '\n';
		}
		len -= period - str + 1;
		str = period + 1;
	}
	return str;
}
//0123456789012
//XXX_.XXXX_.XX
//|--|>----|>--

char* wrapLines(const char* str) {
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo = { 0 };
	GetConsoleScreenBufferInfo(GetStdHandle(((DWORD)-11)), &consoleInfo);
	uint16_t width = (uint16_t)consoleInfo.dwSize.X;

	typedef struct Section {
		uint32_t start;
		uint32_t end;
	}Section;

	// create an array of words from the string
	// by creating sections of each word
	// and then adding them to the array
	Section* sections = (Section*)fsAllocate(sizeof(Section) * strlen(str));
	uint32_t sectionCount = 0;
	uint32_t currentSectionStart = 0;
	for (uint32_t i = 0; i < strlen(str); i++) {
		if (str[i] == ' ') {
			sections[sectionCount].start = currentSectionStart;
			sections[sectionCount].end = i - 1;
			sectionCount++;
			currentSectionStart = i + 1;
		}

		//if the current character is a newline, add a section for it
		//end the current section
		if (str[i] == '\n') {
			//end the current section
			sections[sectionCount].start = currentSectionStart;
			sections[sectionCount].end = i - 1;
			sectionCount++;
			currentSectionStart = i + 1;
			//add a section for the newline
			sections[sectionCount].start = i;
			sections[sectionCount].end = i;
			sectionCount++;
		}
	}

	//add the last section
	sections[sectionCount].start = currentSectionStart;
	sections[sectionCount].end = (uint32_t) (strlen(str)) - 1;
	sectionCount++;

	//wrap the sections
	//allocate a new string to store the wrapped text
	char* wrapped = (char*)fsAllocate(sizeof(char) * strlen(str) + 1);

	//loop through the sections and print them to wrapped
	uint32_t currentLineWidth = 0;
	uint32_t currentLineIndex = 0;
	for (uint32_t i = 0; i < sectionCount; i++) {
		uint32_t sectionSize = sections[i].end - sections[i].start + 1;
		if (sectionSize <= 0) {
			continue;
		}
		char* section = (char*)fsAllocate(sizeof(char) * sectionSize + 1);
		memcpy(section, str + sections[i].start, sectionSize);
		section[sectionSize] = '\0';

		//calculate the caracter with of the section
		uint32_t sectionWidth = 0;
		for (uint32_t j = 0; j < sectionSize; j++) {
			//add 1 to the section width for each character
			//add 4 to the section width for each tab
			if (section[j] == '\t') {
				sectionWidth += 4;
			} else {
				sectionWidth++;
			}
		}

		//if the section will fit on the current line, print it
		if (currentLineWidth + sectionWidth <= width) {
			memcpy(wrapped + currentLineIndex, section, sectionSize);
			currentLineIndex += sectionSize;
			currentLineWidth += sectionWidth;

			//if the section is a newline reset the line width
			if (section[0] == '\n') {
				currentLineWidth = 0;
			} else {
				wrapped[currentLineIndex] = ' ';
				currentLineIndex++;
				currentLineWidth++;
			}

			fsFree(section);
			continue;
		}

		//if the section will not fit on the current line, print it on a new line
		wrapped[currentLineIndex] = '\n';
		currentLineIndex++;
		memcpy(wrapped + currentLineIndex, section, sectionSize);
		currentLineIndex += sectionSize;
		currentLineWidth = sectionWidth;
		
		//if the section is a newline reset the line width
		if (section[0] == '\n') {
			currentLineWidth = 0;
		} else {
			wrapped[currentLineIndex] = ' ';
			currentLineIndex++;
			currentLineWidth++;
		}

		//free the section
		fsFree(section);
	}

	//add a null terminator to the wrapped string
	wrapped[currentLineIndex] = '\0';

	//return the wrapped string
	return wrapped;

}

VKAPI_ATTR VkBool32 debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	HANDLE console = GetStdHandle(((DWORD)-11));
	WORD color = WHITE;

	switch (messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		color = RED;
		if (DEBUG_LEVEL <= DEBUG_LEVEL_ERRORS) {
			return VK_FALSE;
		}
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		color = YELLOW;
		if (DEBUG_LEVEL <= DEBUG_LEVEL_ERRORS_WARNINGS) {
			return VK_FALSE;
		}
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		color = CYAN;
		if (DEBUG_LEVEL <= DEBUG_LEVEL_ERRORS_WARNINGS_INFO) {
			return VK_FALSE;
		}
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		color = BLUE;
		if (DEBUG_LEVEL <= DEBUG_LEVEL_ALL) {
			return VK_FALSE;
		}
		break;
	default:
		color = WHITE;
		if (DEBUG_LEVEL <= DEBUG_LEVEL_ALL) {
			return VK_FALSE;
		}
		break;
	}

	SetConsoleTextAttribute(console, color);

	putSentencesOnNewLines(pCallbackData->pMessage);
	putColonsOnNewLines(pCallbackData->pMessage);
	putCommasOnNewLines(pCallbackData->pMessage);
	printf("validation layer: \n%s\n\n\n", wrapLines(pCallbackData->pMessage));

	SetConsoleTextAttribute(console, WHITE);

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void debugMessengerCreate(EngineHandle handle, EngineSettings settings) {
	if (enableValidationLayers) {
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessengerCreateInfo.pfnUserCallback = debugCallback;
		debugMessengerCreateInfo.pUserData = nullptr;
		debugMessengerCreateInfo.pNext = nullptr;
		debugMessengerCreateInfo.flags = 0;
		VkCheck(CreateDebugUtilsMessengerEXT(INSTANCE(handle), &debugMessengerCreateInfo, nullptr, &DEBUG_MESSENGER(handle)));
	}
}

void debugMessengerDestroy(EngineHandle handle) {
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(INSTANCE(handle),DEBUG_MESSENGER(handle), nullptr);
	}
}







