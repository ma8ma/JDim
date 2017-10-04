// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#ifdef _DEBUG
#include <iomanip>
#endif

#include "misctrip.h"
#include "miscutil.h"

#include <array>
#include <sstream>
#include <cstring>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(USE_GNUTLS)
#  include <gnutls/gnutls.h>
#  include <gnutls/crypto.h>
#elif defined(USE_OPENSSL)
#  include <openssl/sha.h>
#  include <openssl/hmac.h>
#endif

// for crypt
#if defined (USE_OPENSSL)
#  include <openssl/des.h>
#elif defined (HAVE_CRYPT_H)
#  include <crypt.h>
#else
#  include <unistd.h>
#endif


/*--------------------------------------------------------------------*/
// ローカル関数の宣言

// SHA1を計算
std::string create_sha1( const std::string& key );

// トリップを計算(新方式)
std::string create_trip_newtype( const std::string& key );

// トリップを計算(従来方式)
std::string create_trip_conventional( const std::string& key );
/*--------------------------------------------------------------------*/



/*--------------------------------------------------------------------*/
// SHA1を計算
//
// param1: 元となる文字列
// return: SHA1文字列
/*--------------------------------------------------------------------*/
std::string create_sha1( const std::string& key )
{
    if( key.empty() ) return std::string();

    std::array<unsigned char, 20> digest;  // SHA_DIGEST_LENGTH

#if defined(USE_GNUTLS)
    if( ::gnutls_hash_fast( GNUTLS_DIG_SHA1, key.c_str(), key.size(), digest.data() ) < 0 ) {
        return std::string{};
    }

#elif defined(USE_OPENSSL)
    SHA1( reinterpret_cast< const unsigned char * >( key.c_str() ), key.length(), digest.data() );

#endif

#ifdef _DEBUG
    std::cout << "create_sha1 : SHA1 = " << std::hex << std::setfill( '0' );
    for( unsigned char u : digest ) {
        std::cout << std::setw( 2 ) << static_cast<unsigned int>( u );
    }
    std::cout << std::setfill( ' ' ) << std::dec << std::endl;
#endif

    return std::string( digest.begin(), digest.end() );
}


/*--------------------------------------------------------------------*/
// トリップを計算(新方式) 2009/06の仕様
// param1: 元となる文字列
// return: トリップ文字列
/*--------------------------------------------------------------------*/
std::string create_trip_newtype( const std::string& key )
{
    if( key.empty() ) return std::string();

    const size_t key_length = key.length();

    // キーは1024文字以内に限定される
    if( key_length > 1024 ) return std::string();

    std::string trip = "???";

    // "#"で始まる
    if( key[0] == '#' )
    {
        // 全体が17〜19文字 ^#[0-9A-Fa-f]{16}[./0-9A-Za-z]{0,2}$
        if( key_length > 16 && key_length < 20 )
        {
            // 16進数文字列部分
            const std::string hex_part = key.substr( 1, 16 );

            char key_binary[17] = { 0 };

            // 16進数文字列を全てバイナリに変換出来た [0-9A-Za-z]{16}
            if( MISC::chrtobin( hex_part.c_str(), key_binary ) == 16 )
            {
                std::string salt;

                bool is_salt_suitable = true;

                size_t n;

                // salt 候補の17〜18文字目を検証
                for( n = 17; n < 19 && key[n]; ++n )
                {
                    // [./0-9A-Za-z]
                    if( isalnum( key[n] ) != 0 || (unsigned char)( key[n] - 0x2e ) < 2 )
                    {
                        salt.push_back( key[n] );
                    }
                    else
                    {
                        is_salt_suitable = false;
                        break;
                    }
                }

                // salt が適切(空も含む)
                if( is_salt_suitable == true )
                {
                    // salt に".."を足す
                    salt.append( ".." );

                    // crypt (key は先頭8文字しか使われない)
#ifdef USE_OPENSSL
                    char ret[14];
                    const char *crypted = DES_fcrypt( key_binary, salt.c_str(), ret );
#elif defined( _GNU_SOURCE )
                    struct crypt_data crydat;
                    crydat.initialized = 0;
                    const char *crypted = crypt_r( key_binary, salt.c_str(), &crydat );
#else
                    const char *crypted = crypt( key_binary, salt.c_str() );
#endif

                    // 末尾から10文字(cryptの戻り値はnullptrでなければ必ず13文字)
                    if( crypted ) trip = std::string( crypted + 3 );
                    else trip.clear();
                }
            }
        }
    }
    // "$"で始まる
    else if( key[0] == '$' )
    {
        // 現在は"???"を返す
    }
    // SHA1パターン
    else
    {
        const std::string sha1 = create_sha1( key );

        if( ! sha1.empty() )
        {
            // BASE64エンコード
            const std::string encoded = MISC::base64( sha1 );

            // 先頭から12文字
            trip = encoded.substr( 0, 12 );
            std::replace( trip.begin(), trip.end(), '+', '.' );
        }
    }

    return trip;
}


/*--------------------------------------------------------------------*/
// トリップを計算(従来方式) 2003/11/15の仕様らしい
// 新方式導入に伴い、特殊文字の置換が不要になったようだ
//
// param1: 元となる文字列
// return: トリップ文字列
/*--------------------------------------------------------------------*/
std::string create_trip_conventional( const std::string& key )
{
    if( key.empty() ) return std::string();

    // key の2,3バイト目を salt として取り出す
    std::string salt = key.substr( 1, 2 );

    // 仕様に合わせて salt を変換
    const size_t salt_length = salt.length();
    size_t n;
    for( n = 0; n < salt_length; n++ )
    {
        // 0x2e〜0x7aの範囲にないものは '.'(0x2e)
        if( (unsigned char)( salt[n] - 0x2E ) > 0x4C )
        {
            salt[n] = 0x2e;
        }
        // :;<=>?@ (0x3a〜0x40) は A〜G (0x41〜0x47)
        else if( (unsigned char)( salt[n] - 0x3A ) < 0x07 )
        {
            salt[n] += 7;
        }
        // [\]^_` (0x5b〜0x60) は a〜f (0x61〜0x66)
        else if( (unsigned char)( salt[n] - 0x5B ) < 0x06 )
        {
            salt[n] += 6;
        }
    }

    // salt の末尾に"H."を足す
    salt.append( "H." );

    // crypt (key は先頭8文字しか使われない)
#ifdef USE_OPENSSL
    char ret[14];
    const char *crypted = DES_fcrypt( key.c_str(), salt.c_str(), ret );
#elif defined( _GNU_SOURCE )
    struct crypt_data crydat;
    crydat.initialized = 0;
    const char *crypted = crypt_r( key.c_str(), salt.c_str(), &crydat );
#else
    const char *crypted = crypt( key.c_str(), salt.c_str() );
#endif

    std::string trip;

    // 末尾10(cryptの戻り値はnullptrでなければ必ず13文字)
    if( crypted ) trip = std::string( crypted + 3 );

    return trip;
}


/*--------------------------------------------------------------------*/
// トリップを取得
//
// param1: 元となる文字列(最初の"#"を含まない。UTF-8 であること)
// param2: 書き込む掲示板の文字コード
// return: トリップ文字列
/*--------------------------------------------------------------------*/
std::string MISC::get_trip( const std::string& str, const std::string& charset )
{
    if( str.empty() ) return std::string();

    // str の文字コードを UTF-8 から charset に変更して key に代入する
    std::string key = MISC::Iconv( str, charset, "UTF-8" );

    std::string trip;

    // key が12文字以上の場合は新方式
    if( key.length() > 11 )
    {
        trip = create_trip_newtype( key );
    }
    // 従来方式
    else
    {
        trip = create_trip_conventional( key );
    }

#ifdef _DEBUG
    std::cout << "MISC::get_trip : " << str << " -> " << trip << std::endl;
#endif

    return trip;
}
