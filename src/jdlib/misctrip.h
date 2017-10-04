// ライセンス: GPL2

// トリップ関係の関数

#ifndef _MISCTRIP_H
#define _MISCTRIP_H

#include "charcode.h"

#include <string>

namespace MISC
{
    // トリップを取得 (SHA1等の新方式対応)
    std::string get_trip( const std::string& str, const CharCode charcode );
    // FNV Hash
    uint32_t fnv_hash( const char *key, size_t length );
}

#endif
