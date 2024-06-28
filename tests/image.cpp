#include <gtest/gtest.h>
#include <splatit/splatit.hpp>

#include <fea/utils/file.hpp>

extern const char* argv0;

namespace {
TEST(image, load) {
	std::filesystem::path exe_path = fea::executable_dir(argv0);
	std::filesystem::path testfiles_dir = exe_path / "tests_data/";
	std::filesystem::path bw_img_path = testfiles_dir / "tattoo_bw.png";

	splat::img_id imgid = splat::load(bw_img_path);
	EXPECT_NE(imgid, splat::img_id{});
	EXPECT_EQ(imgid.id, 0u);

	splat::splat_id sid = splat::convert(imgid, {});
}
} // namespace