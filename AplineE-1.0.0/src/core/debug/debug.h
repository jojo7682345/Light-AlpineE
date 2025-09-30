#pragma once
#include <dataTypes.h>
#include "../core.h"

const char* const* debugGetRequiredExtensions(uint32_t* extensionCount);
const char* const* debugGetRequiredValidationLayers(uint32_t* layerCount);

int debugCheckValidationLayerSupport();
void debugCheckSupport();

void debugMessengerCreate(EngineHandle handle, EngineSettings settings);

void debugMessengerDestroy(EngineHandle handle);

