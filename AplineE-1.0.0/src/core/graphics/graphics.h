#pragma once
#include "../core.h"
#include "shaders/shaders.h"


void graphicsPipelineCreate(EngineHandle handle, EngineSettings settings);
void frameBuffersAllocate(EngineHandle handle, EngineSettings settings);

void graphicsPipelineDestroy(EngineHandle handle);
void frameBuffersFree(EngineHandle handle);
