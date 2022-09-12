#pragma once
#include "../core.h"
#include "surface/surface.h"


void windowGetDisplaySize(EngineHandle handle, int* width, int* height);

void windowCreate(EngineHandle handle, EngineSettings settings);
void windowDestroy(EngineHandle handle);

void windowSetFullScreen(EngineHandle handle, bool_t fullscreen);

void windowSetSize(EngineHandle handle, uint32_t width, uint32_t height);
