#include "util.h"

namespace utils {
	namespace math {
		float sigmoid(int x, int offset, float stretch) {
			return 1.0f / (1.0f + expf(-stretch * (x - offset)));
		}
	}
}