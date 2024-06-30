#pragma once
// #include "image.hpp"

#include <cassert>
#include <type_traits>

namespace splat {

struct square_range {
	using mssize_t = std::make_signed_t<size_t>;

	square_range(size_t width, size_t height, size_t x, size_t y)
			: _width(width)
			, _height(height)
			, _x(x)
			, _y(y) {
		grow();
	}

	// Grow the boundary.
	void grow() noexcept {
		++_offset;

		// We loop inclusively.
		_ytop = _ytop == 0 ? 0 : _ytop - 1;
		_ybottom = _ybottom >= _height - 1 ? _height - 1 : _ybottom + 1;
		_xleft = _xleft == 0 ? 0 : _xleft - 1;
		_xright = _xright >= _width - 1 ? _width - 1 : _xright + 1;
	}

	// Loop around the current square boundary.
	// Return true to exit early.
	// Ignore return value.
	template <class Func>
	bool loop_once(Func&& func) const {
		bool skip_ytop = _ytop != _y - _offset;
		bool skip_ybottom = _ybottom != _y + _offset;
		bool skip_xleft = _xleft != _x - _offset;
		bool skip_xright = _xright != _x + _offset;

		// Top row.
		if (!skip_ytop) {
			for (mssize_t x = _xleft; x <= _xright; ++x) {
				// if (x == _xleft && skip_xleft) {
				//	continue;
				// }
				// if (x == _xright && skip_xright) {
				//	continue;
				// }

				assert(_ytop >= 0 && x >= 0);
				if (func(size_t(x), size_t(_ytop))) {
					return true;
				}
			}
		}

		// Middle.
		{
			mssize_t mtop = skip_ytop ? _ytop : _ytop + 1;
			mssize_t mbottom = skip_ybottom ? _ybottom : _ybottom - 1;
			for (mssize_t y = mtop; y <= mbottom; ++y) {
				if (!skip_xleft) {
					assert(y >= 0 && _xleft >= 0);
					if (func(size_t(_xleft), size_t(y))) {
						return true;
					}
				}

				if (!skip_xright) {
					assert(y >= 0 && _xright >= 0);
					if (func(size_t(_xright), size_t(y))) {
						return true;
					}
				}
			}
		}

		// Bottom row.
		if (!skip_ybottom) {
			for (mssize_t x = _xleft; x <= _xright; ++x) {
				// if (x == _xleft && skip_xleft) {
				//	continue;
				// }
				// if (x == _xright && skip_xright) {
				//	continue;
				// }

				assert(_ybottom >= 0 && x >= 0);
				if (func(size_t(x), size_t(_ybottom))) {
					return true;
				}
			}
		}
		return false;
	}

	// Loop and iterate as long as we can grow our boundary.
	// Return true to stop iterating.
	template <class Func>
	void loop(Func&& func) {
		while (_ytop != 0 && _ybottom != _height && _xleft != 0
				&& _xright != _width) {
			if (loop_once(func)) {
				return;
			}
		}
	}

private:
	// inputs
	mssize_t _width = 0;
	mssize_t _height = 0;
	mssize_t _x = 0;
	mssize_t _y = 0;

	// Current +- offset from center.
	mssize_t _offset = 0;

	// Square boundary.
	mssize_t _ytop = _y;
	mssize_t _ybottom = _y;
	mssize_t _xleft = _x;
	mssize_t _xright = _x;
};
} // namespace splat
