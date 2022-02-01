#pragma once
#include <corecrt_math.h>

typedef unsigned __int64 bitboard;
#define C64(constantU64) constantU64##ULL
#define Bitloop(X) for(;X;X &= (X-1))
#ifdef __GNUC__
#define getSquare(X) _tzcnt_u64(X)
#else
unsigned long __inline getSquare(bitboard value) {
	unsigned long trailing_zero = 0;

	if (_BitScanForward64(&trailing_zero, value)) {
		return trailing_zero;
	}
	else {
		// This is undefined, I better choose 32 than 0
		return 32;
	}
}
#endif

namespace utils {
	namespace math {
		float sigmoid(int x, int offset = 0, float stretch = 1.0f);
		float invSigmoid(float x, int offset = 0, float stretch = 1.0f);
	}
}