#pragma once

#define WIL_DELETE_COPY_AND_REASSIGNMENT(c) c(const c&)=delete; c&operator=(const c&)=delete

namespace wil {

using VendorPtr = void*;

}
