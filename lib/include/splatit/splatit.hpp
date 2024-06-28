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

extern img_id load(const std::filesystem::path& fpath);

struct convert_opts {};
extern splat_id convert(img_id, convert_opts);

} // namespace splat