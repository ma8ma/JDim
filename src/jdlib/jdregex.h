// ライセンス: GPL2

#ifndef _JDREGEX_H
#define _JDREGEX_H

#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ONIGPOSIX_H
#include <onigposix.h>
#elif defined( HAVE_PCREPOSIX_H )
#include <pcreposix.h>
#elif defined( HAVE_REGEX_H )
#include <regex.h>
#else
#include <glib.h>
#endif	/** USE_ONIG **/

#if defined( HAVE_ONIGPOSIX_H ) || defined( HAVE_PCREPOSIX_H ) || defined( HAVE_REGEX_H )
#define USE_REGEX_COMPAT 1
#else
#define USE_REGEX_COMPAT 0
#endif

namespace JDLIB
{
    class Regex;

    class RegexPattern
    {
        friend class Regex;

#if USE_REGEX_COMPAT
        // onigurumaではregexec()の引数regex_t*がconst修飾されていない
        // そのためRegex::match()のコンパイルエラーを回避するためmutable修飾が必要
        mutable regex_t m_regex;
#else
        GRegex *m_regex;
#endif
        bool m_compiled;
        bool m_newline;
        bool m_wchar;
        bool m_norm;
#if USE_REGEX_COMPAT
        int m_error;
#else
        GError *m_error;
#endif

    public:
        RegexPattern() : m_compiled( false ), m_error( 0 ){};
        RegexPattern( const std::string& reg, const bool icase, const bool newline,
                        const bool usemigemo = false, const bool wchar = false,
                        const bool norm = false );
        ~RegexPattern();

        // m_regexを複製する方法がないためcopy禁止にする
        RegexPattern( const RegexPattern& ) = delete;
        RegexPattern& operator=( const RegexPattern& ) = delete;
        // moveは許可
        RegexPattern( RegexPattern&& ) noexcept = default;
        RegexPattern& operator=( RegexPattern&& ) noexcept = default;

        bool set( const std::string& reg, const bool icase, const bool newline,
                        const bool usemigemo = false, const bool wchar = false,
                        const bool norm = false );
        void clear();
        bool compiled() const { return m_compiled; }
        std::string errstr() const;
    };


    class Regex
    {
        std::vector< int > m_pos;
        std::vector< std::string > m_results;

        // 全角半角を区別しないときに使う変換用バッファ
        // 処理可能なバッファ長は regoff_t (= int) のサイズに制限される
        std::string m_target_asc;
        std::vector< int > m_table_pos;

    public:

        Regex() noexcept;
        ~Regex() noexcept;

        // notbol : 行頭マッチは必ず失敗する
        // noteol : 行末マッチは必ず失敗する
        bool match( const RegexPattern& creg, const std::string& target, const size_t offset,
                    const bool notbol = false, const bool noteol = false );
        
        // icase : 大文字小文字区別しない
        // newline :  . に改行をマッチさせない
        // usemigemo : migemo使用 (コンパイルオプションで指定する必要あり)
        // wchar : 全角半角の区別をしない
        bool exec( const std::string& reg, const std::string& target, const size_t offset,
                    const bool icase, const bool newline, const bool usemigemo = false,
                    const bool wchar = false, const bool norm = false ){
            RegexPattern pattern( reg, icase, newline, usemigemo, wchar, norm );
            return match( pattern, target, offset );
        }

        // マッチした文字列と \0〜\9 を置換する
        std::string replace( const std::string& repstr );

        int pos( const size_t num ) const;
        std::string str( const size_t num ) const;
    };
}

#endif
