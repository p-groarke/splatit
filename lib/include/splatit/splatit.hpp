#pragma once
#include <filesystem>
#include <limits>

namespace splat {
struct img_id {
	auto operator<=>(const img_id&) const noexcept = default;
	unsigned id = (std::numeric_limits<unsigned>::max)();
};
struct splat_id {
	auto operator<=>(const splat_id&) const noexcept = default;
	unsigned id = (std::numeric_limits<unsigned>::max)();
};

// Load and decode a png image. Pre-processes data for sdf conversion.
extern img_id load(const std::filesystem::path& fpath);


enum class output_format : unsigned {
	// r == g == b, uint8
	// Bad precision, bad overall, for debugging and compatibility.
	rgb_uint8,

	// r != g != b != a, float
	// 32bit reinterpret casted float into rgba channels, best.
	rgba_f32,

	count,
};

struct convert_opts {
	output_format format = output_format::rgb_uint8;
};

// Convert image to sdf using options.
extern splat_id convert(img_id, convert_opts);

// Save and encode the sdf image to png.
extern void save(splat_id, const std::filesystem::path&);

} // namespace splat