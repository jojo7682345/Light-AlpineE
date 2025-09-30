#pragma once
#pragma region header_guard
#ifndef _ALPINE_ENGINE_
#define _ALPINE_ENGINE_
#ifdef __cplusplus
extern "C" {
	int a;
	
#endif
#pragma endregion

#include <dataTypes.h>

#pragma region definitions
#define DEFINE_HANDLE(object) typedef struct object##_T* object;
#define DECL_HANDLE(object, members) typedef struct object##_T { members } object##_T
#define MAKE_VERSION(major,minor,patch) (((major)<<22)|((minor)|12)|(patch))
#define AE_API_CALL _stdcall
#define EMPTY_STRUCT struct { void* reservedForFurtherUse; }
	
#ifdef __cplusplus
#define ANONIMOUS_STRUCT(type,name) type name 
#else
#define ANONIMOUS_STRUCT(type,name) type
#endif
	
#pragma endregion 


DEFINE_HANDLE(EngineHandle);

typedef uint32_t bool_t;
#define True 1
#define False 0

typedef enum AeResult {
	AE_SUCCESS = 0,
	AE_ERROR = 1
}AeResult;



typedef enum GpuTypePreference {
	GPU_TYPE_SELECT_PREFER_DEDICATED = 0,
	GPU_TYPE_SELECT_PREFER_CPU = 1,
	GPU_TYPE_SELECT_REQUIRE_DEDICATED = 2,
	GPU_TYPE_SELECT_REQUIRE_CPU = 3
}GpuTypePreference;

typedef struct GpuPropertyScoreScale {
	int32_t isDedicatedScore;
	int32_t isCpuScore;
	float maxImageDimensions1Dscaler;
	float maxImageDimensions2Dscaler;
	float maxImageDimensions3Dscaler;
	float maxImageDimensionsCubescaler;
	
}GpuPropertyScoreScale;

GpuPropertyScoreScale gpuSelectPreferencesScalersGetDefault();

typedef struct GpuSelectPreferences {
	GpuTypePreference type;
	GpuPropertyScoreScale scalers;
} GpuSelectPreferences;

typedef enum VsyncSettings {
	VSYNC_SETTINGS_DISABLED = 0,
	VSYNC_SETTINGS_REQUIRE_DUAL_BUFFERED = 1,
	VSYNC_SETTINGS_REQUIRE_TRIPLE_BUFFERED = 2,
	VSYNC_SETTINGS_PREFER_TRIPLE_BUFFERED_BUT_REQUIRE_DOUBLE_BUFFERED = 3,
	VSYNC_SETTINGS_PREFER_TRIPLE_BUFFERED_IF_NOT_AVAILABLE_PREFER_DOUBLE_BUFFERED = 4
}VsyncSettings;

typedef enum VsyncEnabled {
	VSYNC_DISABLED = 0,
	VSYNC_DOUBLE_BUFFERED = 1,
	VSYNC_TRIPPLE_BUFFERED = 2,
	VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DOUBLE_BUFFERED = 3,
	VSYNC_PREFER_DOUBLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE = 4,
	VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_PREFER_DOUBLE_BUFFERED_IF_BOTH_NOT_AVAILABLE_DISABLE = 5,
	VSYNC_PREFER_TRIPPLE_BUFFERED_BUT_IF_NOT_AVAILABLE_DISABLE = 6
}VsyncEnabled;

#include "AlpineImageFormats.h"
#include "AlpineEngine.h"
#include "AlpineGraphics.h"
#include "AlpineComponents.h"

typedef struct EngineSettings {
	//metadata
	uint32_t gameVersion;
	const char* gameTitle;

	//window data
	uint32_t windowWidth;
	uint32_t windowHeight;
	const char* windowTitle;
	bool_t windowFullscreen;
	bool_t windowResizable;

	//graphics settings
	VsyncSettings vsyncPreferences;
	VsyncEnabled vsyncEnabled;
	bool_t fpsLimitEnabled;
	uint32_t fpsLimit;

	//gpu settings
	GpuSelectPreferences gpuSelectPreference;

} EngineSettings;

typedef enum ShaderType {
	SHADER_TYPE_VERTEX_SHADER,
	SHADER_TYPE_FRAGMENT_SHADER,
	SHADER_TYPE_GEOMETRY_SHADER,
	SHADER_TYPE_COMPUTE_SHADER
}ShaderType;

typedef struct ShaderInfo {
	ShaderType type;
}ShaderInfo;

EngineSettings AE_API_CALL getEngineSettings();
uint32_t AE_API_CALL getEngineVersion();

int main();


#pragma region end_of_header
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_ALPINE_ENGINE_*/
#pragma endregion