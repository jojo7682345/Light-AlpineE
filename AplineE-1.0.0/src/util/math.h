#pragma once

double clampD(double d, double min, double max);

float clampF(float d, float min, float max);

__int32 clampI32(__int32 d, __int32 min, __int32 max);

unsigned __int32 clampUI32(unsigned __int32 d, unsigned __int32 min, unsigned __int32 max);

__int64 clampI64(__int64 d, __int64 min, __int64 max);

unsigned __int64 clampUI64(unsigned __int64 d, unsigned __int64 min, unsigned __int64 max);

__int16 clampI16(__int16 d, __int16 min, __int16 max);

unsigned __int16 clampUI16(unsigned __int16 d, unsigned __int16 min, unsigned __int16 max);
