// From https://github.com/mapbox/earcut.hpp
#include "earcut.hpp"

std::vector<int> lastResult;

struct Vector2 {
	float x;
	float y;
	Vector2(float x, float y) : x(x), y(y) {}
};

namespace mapbox {
	namespace util {
		template <>
		struct nth<0, Vector2> {
			inline static auto get(const Vector2& t) {
				return t.x;
			};
		};

		template <>
		struct nth<1, Vector2> {
			inline static auto get(const Vector2& t) {
				return t.y;
			};
		};
	};
};

extern "C" {
	__declspec(dllexport) int* earcut(float* coordinates2D, int numPoints) {
		std::vector<std::vector<Vector2>> polygons;
		polygons.emplace_back();
		auto& polygon = polygons[polygons.size() - 1];
		for (int i = 0; i < numPoints; ++i) {
			polygon.emplace_back(coordinates2D[i * 2], coordinates2D[i * 2 + 1]);
		}
		lastResult = mapbox::earcut<int>(polygons);
		return lastResult.data();
	}
}
