#include "splatit/splatit.hpp"
#include "private_include/image.hpp"

#include <fea/containers/id_slotmap.hpp>
#include <fea/image/bmp.hpp>
#include <fea/utils/file.hpp>
#include <fea/utils/throw.hpp>
#include <format>
#include <fstream>
#include <glm/vec2.hpp>
#include <spng.h>

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

	spng_ctx* ctx = spng_ctx_new(0);
	spng_set_png_buffer(ctx, file_data.data(), file_data.size());

	// int fmt = file_data[png_data.size() - 3]
	//		| (file_data[png_data.size() - 2] << 8);
	// fmt;

	size_t out_size = 0;
	spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);

	spng_ihdr ihdr{};
	spng_get_ihdr(ctx, &ihdr);

	img_id ret{ next_img_id++ };
	image_db.insert({ ret, {} });
	image& img = image_db.at(ret);
	img.width = ihdr.width;
	img.height = ihdr.height;
	img.data.resize(out_size);

	spng_decode_image(ctx, img.data.data(), out_size, SPNG_FMT_RGBA8, 0);

	spng_ctx_free(ctx);

	{
		fea::bmp test{
			int(ihdr.width),
			int(ihdr.height),
			4,
			img.data,
		};
		test.write("1-test_out.bmp");
	}

	return ret;
}

splat_id convert(img_id imgid, convert_opts) {
	const image& img = image_db.at(imgid);

	assert(img.data.size() % 4 == 0);
	size_t size = img.data.size() / 4;

	struct rgb {
		bool white() const noexcept {
			return r == g && g == b && b == a && a == uint8_t(255u);
			// constexpr uint8_t cmp = 250u;
			// return r >= cmp && g >= cmp && b >= cmp && a == uint8_t(255u);
		}

		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};
	const rgb* buf = reinterpret_cast<const rgb*>(img.data.data());

	// 0 outside, 1 inside, 2 edge.
	std::vector<uint8_t> mask(size);

	auto for_each_neighbour = [&](size_t y, size_t x, auto&& func) {
		size_t yfirst = y == 0 ? 0 : y - 1;
		size_t ylast = y == img.height - 1 ? y : y + 2;
		size_t xfirst = x == 0 ? 0 : x - 1;
		size_t xlast = x == img.width - 1 ? x : x + 2;

		for (size_t ny = yfirst; ny < ylast; ++ny) {
			for (size_t nx = xfirst; nx < xlast; ++nx) {
				if (ny == y && nx == x) {
					continue;
				}
				func(ny, nx);
			}
		}
	};

	for (size_t y = 0; y < img.height; ++y) {
		for (size_t x = 0; x < img.width; ++x) {
			size_t idx = (y * img.width) + x;
			const rgb& p = buf[idx];

			if (p.white()) {
				// 0
				continue;
			}

			uint8_t val = 1u;
			size_t dbg = 0;
			for_each_neighbour(y, x, [&](size_t ny, size_t nx) {
				assert(!(ny == y && nx == x));

				++dbg;
				size_t nidx = (ny * img.width) + nx;
				const rgb& np = buf[nidx];
				if (np.white()) {
					val = 2u;
				}
			});
			assert(dbg <= 8);
			mask[idx] = val;
		}
	}

	// dbg
	{
		std::vector<uint8_t> dbg;
		dbg.reserve(mask.size() * 3);
		for (uint8_t v : mask) {
			if (v == 0) {
				dbg.push_back(0);
				dbg.push_back(0);
				dbg.push_back(0);
			} else if (v == 1) {
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


	// constexpr bool use_alpha = false;
	for (size_t y = 0; y < img.height; ++y) {
		for (size_t x = 0; x < img.width; ++x) {
			size_t idx = (y * img.width) + x;
			const rgb& p = buf[idx];

			if (p.white()) {
				// outside
			} else {
				// inside
			}
		}
	}

	return {};
}

} // namespace splat