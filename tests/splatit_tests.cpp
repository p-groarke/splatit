#include <gtest/gtest.h>
#include <splatit/splatit.hpp>

#include <fea/utils/file.hpp>

extern const char* argv0;

namespace {

splat::splat_id sid_rgb;
splat::splat_id sid_rgba;

TEST(splatit, load) {
	std::filesystem::path exe_path = fea::executable_dir(argv0);
	std::filesystem::path testfiles_dir = exe_path / "tests_data/";

	// rgb
	{
		std::filesystem::path img_path = testfiles_dir / "splatit_rgb.png";
		sid_rgb = splat::load(img_path);
		EXPECT_NE(sid_rgb, splat::splat_id{});
		EXPECT_EQ(sid_rgb.id, 0u);
	}

	// rgba
	{
		std::filesystem::path img_path = testfiles_dir / "splatit_rgba.png";
		sid_rgba = splat::load(img_path);
		EXPECT_NE(sid_rgba, splat::splat_id{});
		EXPECT_EQ(sid_rgba.id, 1u);
	}
}

TEST(splatit, save) {
	std::filesystem::path exe_path = fea::executable_dir(argv0);
	// std::filesystem::path testfiles_dir = exe_path / "tests_data/";
	std::filesystem::path rgb_out_path = exe_path / "splatit_out_rgb.png";
	std::filesystem::path rgba_out_path = exe_path / "splatit_out_rgba.png";

	splat::save(sid_rgb, { .format = splat::output_format::rgb_uint8 },
			rgb_out_path);
	splat::save(sid_rgba, { .format = splat::output_format::rgba_f32 },
			rgba_out_path);
}
} // namespace