// Lisence: GPLv2

//
// string_viewのラッパー
// std::string_view が無い環境では std::experimental::string_view があれば利用する
// コンパイラ要件が gcc-7 以上になればラッパーは不要になる
//

#ifndef JDLIB_STRING_VIEW_H
#define JDLIB_STRING_VIEW_H

#ifndef __has_include
#error "require __has_include() support."
#endif

#if __has_include(<string_view>)
#include <string_view>
namespace cpp17 {
using string_view = std::string_view;
}
#elif __has_include(<experimental/string_view>)
#include <experimental/string_view>
namespace cpp17 {
using string_view = std::experimental::string_view;
}
#else
#error "require string_view support (std or std::experimental)."
#endif

#endif
