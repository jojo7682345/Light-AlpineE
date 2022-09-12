#pragma once
#include "../core.h"

void commandBufferPoolCreate(EngineHandle handle, EngineSettings settings);

void commandBufferCreate(EngineHandle handle, EngineSettings settings);
void commandBufferRecord(VkCommandBuffer buffer,EngineHandle handle, uint32_t imageIndex);


void commandBufferPoolDestroy(EngineHandle handle);
void commandBufferDestroy(EngineHandle handle);