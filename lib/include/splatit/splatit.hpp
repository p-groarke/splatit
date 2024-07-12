#pragma once
#include <filesystem>
#include <limits>

namespace splat {
struct splat_id {
	auto operator<=>(const splat_id&) const noexcept = default;
	unsigned id = (std::numeric_limits<unsigned>::max)();
};

enum class output_format : unsigned {
	// r == g == b, uint8
	// Bad precision, bad overall, for debugging and compatibility.
	rgb_uint8,

	// r != g != b != a, float
	// 32bit reinterpret casted float into rgba channels, best.
	rgba_f32,

	count,
};

struct opts {
	output_format format = output_format::rgb_uint8;
};

// Load and decode a png image.
// Pre-processes data and convert to raw sdf.
extern splat_id load(const std::filesystem::path& fpath);

// Save and encode the sdf image to png.
extern void save(splat_id, opts, const std::filesystem::path&);

} // namespace splat