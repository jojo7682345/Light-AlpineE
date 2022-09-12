#include "../graphics.h"

VkShaderModule shaderModuleCreate(const char* shaderFile, EngineHandle handle);

void shaderModuleDestroy(VkShaderModule shader, EngineHandle handle);
