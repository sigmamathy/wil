#pragma once

#define WIL_DELETE_COPY_AND_REASSIGNMENT(c) c(const c&)=delete; c&operator=(const c&)=delete

static_assert(
		sizeof(int)			== 4
	&&	sizeof(unsigned)	== 4
	&&	sizeof(float)		== 4
);

namespace wil {

using VendorPtr = void*;

}
