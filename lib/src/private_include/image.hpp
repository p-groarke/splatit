#pragma once
#include <cstdint>
#include <vector>

namespace splat {
enum class point : uint8_t {
	outside,
	inside,
	boundary,
	count,
};

struct image {
	size_t width = 0;
	size_t height = 0;
	bool alpha = false;

	// Maybe unused?
	std::vector<uint8_t> data;

	// A mask of 0, 1, 2.
	std::vector<point> mask;

	// The raw sdf (non-normalized / non-clamped).
	std::vector<double> sdf;
};
} // namespace splat