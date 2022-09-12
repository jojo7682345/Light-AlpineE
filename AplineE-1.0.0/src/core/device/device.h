#pragma once
#include "../core.h"

void getRequiredDeviceExtensions(uint32_t* extensionCount, char*** extensions, EngineSettings settings);


void deviceCreate(EngineHandle handle, EngineSettings settings);
void deviceDestroy(EngineHandle handle);