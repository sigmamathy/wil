#pragma once

#define WIL_DELETE_COPY_AND_REASSIGNMENT(c) c(const c&)=delete; c&operator=(const c&)=delete
#define WIL_ENUM_DEFINE_OR_OPERATOR(c) constexpr c operator|(c x, c y)\
		{ return static_cast<c>((int)x | (int)y); }

#ifdef _MSC_VER // msvc
#define WIL_UNREACHABLE __assume(0)
#else // gcc or clang
#define WIL_UNREACHABLE __builtin_unreachable()
#endif

static_assert(
		sizeof(int)			== 4
	&&	sizeof(unsigned)	== 4
	&&	sizeof(float)		== 4
);

namespace wil {

using VendorPtr = void*;

}
