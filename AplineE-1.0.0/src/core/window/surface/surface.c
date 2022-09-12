#include "surface.h"

void surfaceCreate(EngineHandle handle, EngineSettings settings) {
	VkCheck(glfwCreateWindowSurface(INSTANCE(handle), WINDOW(handle), nullptr, &SURFACE(handle)));
}

void surfaceDestroy(EngineHandle handle) {
	vkDestroySurfaceKHR(INSTANCE(handle), SURFACE(handle), nullptr);
}
