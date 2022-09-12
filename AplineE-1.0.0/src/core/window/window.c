#include "window.h"

void windowGetDisplaySize(EngineHandle handle, int* width, int* height) {
	//get the size of the primary monitor
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	*width = mode->width;
	*height = mode->height;
}

void windowCreate(EngineHandle handle, EngineSettings settings) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, settings.windowResizable ? GLFW_TRUE : GLFW_FALSE);

	GLFWmonitor* monitor = nullptr;
	if (settings.windowFullscreen) {
		monitor = glfwGetPrimaryMonitor();
		settings.windowWidth = glfwGetVideoMode(monitor)->width;
		settings.windowHeight = glfwGetVideoMode(monitor)->height;
	}

	WINDOW(handle) = glfwCreateWindow(settings.windowWidth, settings.windowHeight, settings.windowTitle, monitor, nullptr);

	glfwGetWindowSize(WINDOW(handle), &handle->system.windowWidth, &handle->system.windowHeight);
	windowGetDisplaySize(handle, &handle->system.displayWidth, &handle->system.displayHeight);

}

void windowDestroy(EngineHandle handle) {
	glfwDestroyWindow(WINDOW(handle));
	glfwTerminate();
}



void windowSetFullScreen(EngineHandle handle, bool_t fullscreen) {

	GLFWmonitor* monitor = nullptr;
	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}
	glfwSetWindowMonitor(WINDOW(handle), monitor, 0, 0, engineWindowGetWidth(handle), engineWindowGetHeight(handle), GLFW_DONT_CARE);
	glfwGetWindowSize(WINDOW(handle), &handle->system.windowWidth, &handle->system.windowHeight);
}

void windowSetSize(EngineHandle handle, uint32_t width, uint32_t height) {
	bool_t fullscreen = engineWindowGetFullscreen(handle);
	if (fullscreen) {
		return;
	}
	glfwSetWindowMonitor(WINDOW(handle), nullptr, 0, 0, width, height, GLFW_DONT_CARE);
	glfwGetWindowSize(WINDOW(handle), &handle->system.windowWidth, &handle->system.windowHeight);
}


