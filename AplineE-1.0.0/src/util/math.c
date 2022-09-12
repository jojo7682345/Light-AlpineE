#include "math.h"

double clampD(double d, double min, double max) {
	const double t = d < min ? min : d;
	return t > max ? max : t;
}

float clampF(float d, float min, float max) {
	const float t = d < min ? min : d;
	return t > max ? max : t;
}

__int32 clampI32(__int32 d, __int32 min, __int32 max) {
	const __int32 t = d < min ? min : d;
	return t > max ? max : t;
}

unsigned __int32 clampUI32(unsigned __int32 d, unsigned __int32 min, unsigned __int32 max) {
	const unsigned __int32 t = d < min ? min : d;
	return t > max ? max : t;
}

__int64 clampI64(__int64 d, __int64 min, __int64 max) {
	const __int64 t = d < min ? min : d;
	return t > max ? max : t;
}

unsigned __int64 clampUI64(unsigned __int64 d, unsigned __int64 min, unsigned __int64 max) {
	const unsigned __int64 t = d < min ? min : d;
	return t > max ? max : t;
}

__int16 clampI16(__int16 d, __int16 min, __int16 max) {
	const __int16 t = d < min ? min : d;
	return t > max ? max : t;
}

unsigned __int16 clampUI16(unsigned __int16 d, unsigned __int16 min, unsigned __int16 max) {
	const unsigned __int16 t = d < min ? min : d;
	return t > max ? max : t;
}
