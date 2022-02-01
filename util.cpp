#include "util.h"

namespace utils {
	namespace math {
		float sigmoid(int x, int offset, float stretch) {
			return 1.0f / (1.0f + expf(-stretch * (x - offset)));
		}

		float invSigmoid(float x, int offset, float stretch) {
			return -(1.0 / stretch) * log(1.0 / x - 1) + offset;
		}
	}
}