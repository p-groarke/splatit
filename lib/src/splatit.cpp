#include "splatit/splatit.hpp"
#include "private_include/image.hpp"
#include "splatit/algorithm.hpp"

#include <algorithm>
#include <cmath>
#include <fea/containers/id_slotmap.hpp>
#include <fea/image/bmp.hpp>
#include <fea/utils/file.hpp>
#include <fea/utils/throw.hpp>
#include <format>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <spng.h>
#include <vector>

namespace fea {
template <>
struct id_hash<splat::splat_id> {
	constexpr unsigned operator()(const splat::splat_id& k) const noexcept {
		return k.id;
	}
};
} // namespace fea

namespace splat {
namespace {
unsigned next_splat_id = 0u;

fea::id_slotmap<splat_id, image> splat_db;
} // namespace

splat_id load(const std::filesystem::path& fpath) {
	if (!std::filesystem::exists(fpath)) {
		fea::maybe_throw<std::invalid_argument>(__FUNCTION__, __LINE__,
				std::format("Image file doesn't exist '{}'.",
						fpath.string().c_str()));
	}

	// Load png.
	std::vector<uint8_t> file_data;
	{
		std::ifstream ifs{ fpath, std::ios::binary | std::ios::ate };
		if (!ifs.is_open()) {
			fea::maybe_throw<std::runtime_error>(__FUNCTION__, __LINE__,
					std::format("Couldn't open image file '{}'.",
							fpath.string().c_str()));
		}

		size_t bytes = fea::file_size(ifs);
		if (bytes == 0) {
			fea::maybe_throw<std::runtime_error>(__FUNCTION__, __LINE__,
					std::format("No data to read in image file '{}'.",
							fpath.string().c_str()));
		}

		file_data.resize(bytes);
		ifs.read(reinterpret_cast<char*>(file_data.data()), file_data.size());
	}

	splat_id ret{ next_splat_id++ };
	splat_db.insert({ ret, {} });
	image& img = splat_db.at(ret);

	// Decode png.
	{
		spng_ctx* ctx = spng_ctx_new(0);
		spng_set_png_buffer(ctx, file_data.data(), file_data.size());

		size_t out_size = 0;
		spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);

		spng_ihdr ihdr{};
		spng_get_ihdr(ctx, &ihdr);

		img.width = ihdr.width;
		img.height = ihdr.height;
		img.alpha = ihdr.color_type == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA
				 || ihdr.color_type == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;
		img.data.resize(out_size);

		spng_decode_image(ctx, img.data.data(), out_size, SPNG_FMT_RGBA8, 0);

		spng_ctx_free(ctx);
	}

	// Debug decoding.
	{
		fea::bmp test{
			int(img.width),
			int(img.height),
			4,
			img.data,
		};
		test.write("1-test_out.bmp");
	}

	struct rgb {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	auto is_background = [&](const rgb& col) {
		if (img.alpha) {
			return col.a == 0;
		}
		return col.r == col.g && col.g == col.b && col.b == col.a
			&& col.a == uint8_t(255u);
	};

	// Generate mask.
	{
		assert(img.data.size() % 4 == 0);
		size_t size = img.data.size() / 4;

		// 0 outside, 1 inside, 2 edge.
		std::vector<point>& mask = img.mask;
		mask.resize(size, point::outside);

		const rgb* buf = reinterpret_cast<const rgb*>(img.data.data());

		for (size_t y = 0; y < img.height; ++y) {
			for (size_t x = 0; x < img.width; ++x) {
				size_t idx = (y * img.width) + x;
				const rgb& p = buf[idx];

				if (is_background(p)) {
					// 0
					continue;
				}

				point val = point::inside;
				square_range rng{ img.width, img.height, x, y };
				rng.loop_once([&](size_t nx, size_t ny) {
					assert(!(ny == y && nx == x));
					size_t nidx = (ny * img.width) + nx;
					const rgb& np = buf[nidx];
					if (is_background(np)) {
						val = point::boundary;
						return true;
					}
					return false;
				});

				mask[idx] = val;
			}
		}
	}

	// Debug mask.
	{
		std::vector<uint8_t> dbg;
		dbg.reserve(img.mask.size() * 3);
		for (point v : img.mask) {
			if (v == point::outside) {
				dbg.push_back(0);
				dbg.push_back(0);
				dbg.push_back(0);
			} else if (v == point::inside) {
				dbg.push_back(255u);
				dbg.push_back(255u);
				dbg.push_back(255u);
			} else {
				dbg.push_back(255u);
				dbg.push_back(0);
				dbg.push_back(0);
			}
		}

		fea::bmp test{ int(img.width), int(img.height), 3, dbg };
		test.write("2-test_out.bmp");
	}


	// Make sdf.
	{
		const std::vector<point>& mask = img.mask;
		std::vector<double>& sdf = img.sdf;
		sdf.resize(mask.size());

		for (size_t y = 0; y < img.height; ++y) {
			for (size_t x = 0; x < img.width; ++x) {
				size_t idx = (y * img.width) + x;

				point p = mask[idx];
				if (p == point::boundary) {
					sdf[idx] = 0.0;
					continue;
				}

				double dist = (std::numeric_limits<double>::max)();
				glm::dvec2 from{ double(x) + 0.5, double(y) + 0.5 };

				// Find closest boundary.
				square_range rng{ img.width, img.height, x, y };
				while (true) {
					// We'll exit the loop once every sample is considered
					// too far, aka, we've found the closest point.
					bool all_far = true;

					// We use loop_once so we check all boundary candidates.
					rng.loop_once([&](size_t nx, size_t ny) {
						size_t nidx = (ny * img.width) + nx;
						if (mask[nidx] != point::boundary) {
							return false;
						}

						glm::dvec2 to{ double(nx) + 0.5, double(ny) + 0.5 };
						double ndist = glm::distance(from, to);
						if (ndist < dist) {
							dist = ndist;
							all_far = false;
						} else {
							all_far &= true;
						}
						return false;
					});

					if (dist != (std::numeric_limits<double>::max)()) {
						if (all_far) {
							break;
						}
					}

					rng.grow();
				}

				assert(dist != (std::numeric_limits<double>::max)());
				if (p == point::inside) {
					dist *= -1.0;
				}
				sdf[idx] = dist;
			}
		}
	}

	// Debug sdf
	{
		std::vector<uint8_t> dbg;
		dbg.reserve(img.sdf.size() * 3);
		for (double d : img.sdf) {
			d = std::clamp(d, -8.0, 8.0);
			d /= 8.0;
			// d = std::clamp(d, -1.0, 1.0);
			assert(d <= 1.0);
			assert(d >= -1.0);

			uint8_t v = uint8_t(127.0 * d + 127.0);
			dbg.push_back(v);
			dbg.push_back(v);
			dbg.push_back(v);
		}

		fea::bmp test{ int(img.width), int(img.height), 3, dbg };
		test.write("3-test_out.bmp");
	}

	return ret;
}

void save(splat_id sid, opts mopts, const std::filesystem::path& fpath) {
	if (!splat_db.contains(sid)) {
		fea::maybe_throw(__FUNCTION__, __LINE__, "Invalid splat id.");
	}

	const image& img = splat_db.at(sid);
	const std::vector<double>& sdf = img.sdf;
	if (sdf.empty()) {
		return;
	}

	// Convert to png binary.
	// RGB. TODO : consider output_format
	std::vector<uint8_t> pixels;

	// Output color type.
	uint8_t png_color_type = 0;

	switch (mopts.format) {
	case output_format::rgb_uint8: {
		// Q : Allow customizing clamp value?
		png_color_type = SPNG_COLOR_TYPE_TRUECOLOR;

		constexpr double clamp_max = 8.0;
		for (double d : sdf) {
			d = std::clamp(d, -clamp_max, clamp_max);
			d /= clamp_max;

			uint8_t v = uint8_t(127.0 * d + 127.0);
			pixels.push_back(v);
			pixels.push_back(v);
			pixels.push_back(v);
		}
	} break;
	case output_format::rgba_f32: {
		png_color_type = SPNG_COLOR_TYPE_TRUECOLOR_ALPHA;

		for (double d : sdf) {
			float f = float(d);
			std::array<uint8_t, 4> arr{};
			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&f);
			std::copy(ptr, ptr + 4, arr.begin());
			pixels.insert(pixels.end(), arr.begin(), arr.end());
		}
	} break;
	default: {
		assert(false);
	} break;
	}

	{
		//// Normalize / clamp?
		//{
		//	// auto it = std::max_element(
		//	//		sdf.begin(), sdf.end(), [](double lhs, double rhs) {
		//	//			return std::abs(lhs) < std::abs(rhs);
		//	//		});
		//	// assert(it != sdf.end());

		//	// double max_dist = std::abs(*it);
		//	for (double& d : sdf) {
		//		d /= clamp_max;
		//	}
		//}
	}

	size_t png_size = 0;
	void* png = nullptr;
	{
		/* Creating an encoder context requires a flag */
		spng_ctx* enc = spng_ctx_new(SPNG_CTX_ENCODER);

		/* Encode to internal buffer managed by the library */
		spng_set_option(enc, SPNG_ENCODE_TO_BUFFER, 1);

		/* Specify image dimensions, PNG format */
		spng_ihdr ihdr = {
			.width = uint32_t(img.width),
			.height = uint32_t(img.height),
			.bit_depth = 8,
			.color_type = png_color_type,
		};

		/* Image will be encoded according to ihdr.color_type, .bit_depth */
		spng_set_ihdr(enc, &ihdr);

		/* SPNG_FMT_PNG is a special value that matches the format in ihdr,
		   SPNG_ENCODE_FINALIZE will finalize the PNG with the end-of-file
		   marker */
		spng_encode_image(enc, pixels.data(), pixels.size(), SPNG_FMT_PNG,
				SPNG_ENCODE_FINALIZE);

		/* PNG is written to an internal buffer by default */
		int error = 0;
		png = spng_get_png_buffer(enc, &png_size, &error);

		/* Free context memory */
		spng_ctx_free(enc);
	}

	// Serialize
	{
		std::ofstream ofs{ fpath, std::ios::binary };
		if (!ofs.is_open()) {
			return;
		}
		ofs.write(reinterpret_cast<const char*>(png), png_size);

		/* User owns the buffer after a successful call */
		free(png);
	}
}

} // namespace splat