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
struct id_hash<splat::img_id> {
	constexpr unsigned operator()(const splat::img_id& k) const noexcept {
		return k.id;
	}
};
} // namespace fea

namespace splat {
namespace {
unsigned next_img_id;
unsigned next_splat_id;

fea::id_slotmap<img_id, image> image_db;
// fea::id_slotmap<splat_id, std::vector<uint8_t>> splat_db;
} // namespace

img_id load(const std::filesystem::path& fpath) {
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

	img_id ret{ next_img_id++ };
	image_db.insert({ ret, {} });
	image& img = image_db.at(ret);

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

	// Generate mask.
	{
		assert(img.data.size() % 4 == 0);
		size_t size = img.data.size() / 4;

		// 0 outside, 1 inside, 2 edge.
		std::vector<point>& mask = img.mask;
		mask.resize(size, point::outside);

		struct rgb {
			bool white() const noexcept {
				return r == g && g == b && b == a && a == uint8_t(255u);
				// constexpr uint8_t cmp = 250u;
				// return r >= cmp && g >= cmp && b >= cmp && a ==
				// uint8_t(255u);
			}

			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
		const rgb* buf = reinterpret_cast<const rgb*>(img.data.data());

		for (size_t y = 0; y < img.height; ++y) {
			for (size_t x = 0; x < img.width; ++x) {
				size_t idx = (y * img.width) + x;
				const rgb& p = buf[idx];

				if (p.white()) {
					// 0
					continue;
				}

				point val = point::inside;
				square_range rng{ img.width, img.height, x, y };
				rng.loop_once([&](size_t nx, size_t ny) {
					assert(!(ny == y && nx == x));
					size_t nidx = (ny * img.width) + nx;
					const rgb& np = buf[nidx];
					if (np.white()) {
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

	return ret;
}

splat_id convert(img_id imgid, convert_opts) {
	const image& img = image_db.at(imgid);

	const std::vector<point>& mask = img.mask;
	std::vector<double> sdf(mask.size());

	// Without boundary, we don't have enough precision in certain formats.
	constexpr double clamp_max = 8.0;

	for (size_t y = 0; y < img.height; ++y) {
		for (size_t x = 0; x < img.width; ++x) {
			size_t idx = (y * img.width) + x;

			point p = mask[idx];
			if (p == point::boundary) {
				sdf[idx] = 0.0;
				continue;
			}

			// double dist = p == point::outside ? 1.0 : -1.0;
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
					if (ndist > clamp_max) {
						ndist = clamp_max;
					}

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

	// Normalize?
	{
		// auto it = std::max_element(
		//		sdf.begin(), sdf.end(), [](double lhs, double rhs) {
		//			return std::abs(lhs) < std::abs(rhs);
		//		});
		// assert(it != sdf.end());

		// double max_dist = std::abs(*it);
		for (double& d : sdf) {
			d /= clamp_max;
		}
	}

	// Debug sdf
	{
		std::vector<uint8_t> dbg;
		dbg.reserve(sdf.size() * 3);
		for (double d : sdf) {
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
	return {};
}

} // namespace splat