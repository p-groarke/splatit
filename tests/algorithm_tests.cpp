#include <gtest/gtest.h>
#include <splatit/algorithm.hpp>
#include <unordered_set>
#include <utility>

namespace std {
template <>
struct hash<std::pair<size_t, size_t>> {
	size_t operator()(const std::pair<size_t, size_t>& p) const noexcept {
		size_t h1 = std::hash<size_t>{}(p.first);
		size_t h2 = std::hash<size_t>{}(p.first);
		return h1 ^ (h2 << 1);
	}
};
} // namespace std

namespace {
TEST(algorithm, square_range) {
	// center
	{
		splat::square_range rng{ 200, 100, 100, 50 };

		size_t loop_cnt = 0;
		std::unordered_set<std::pair<size_t, size_t>> visited;
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 8);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 16);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 24);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));
	}

	// top left corner
	{
		splat::square_range rng{ 200, 100, 0, 0 };

		size_t loop_cnt = 0;
		std::unordered_set<std::pair<size_t, size_t>> visited;
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 3);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 5);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 7);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));
	}

	// top right corner
	{
		splat::square_range rng{ 200, 100, 199, 0 };

		size_t loop_cnt = 0;
		std::unordered_set<std::pair<size_t, size_t>> visited;
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 3);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 5);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 7);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));
	}

	// bottom left corner
	{
		splat::square_range rng{ 200, 100, 0, 99 };

		size_t loop_cnt = 0;
		std::unordered_set<std::pair<size_t, size_t>> visited;
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 3);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 5);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 7);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));
	}

	// bottom right corner
	{
		splat::square_range rng{ 200, 100, 199, 99 };

		size_t loop_cnt = 0;
		std::unordered_set<std::pair<size_t, size_t>> visited;
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 3);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 5);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));

		loop_cnt = 0;
		visited.clear();
		rng.grow();
		rng.loop_once([&](size_t nx, size_t ny) {
			++loop_cnt;

			EXPECT_FALSE(visited.contains({ nx, ny }));
			visited.insert({ nx, ny });
			return false;
		});
		EXPECT_EQ(loop_cnt, 7);
		EXPECT_EQ(loop_cnt, visited.size());
		EXPECT_FALSE(visited.contains({ size_t(0), size_t(0) }));
	}
}
} // namespace