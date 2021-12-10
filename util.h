#pragma once
#include <corecrt_math.h>

namespace utils {
	namespace math {
		float sigmoid(int x, int offset = 0, float stretch = 1.0f);
	}
}