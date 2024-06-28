#pragma once
namespace splat {
struct image {
	size_t width = 0;
	size_t height = 0;
	std::vector<uint8_t> data;
};
} // namespace splat