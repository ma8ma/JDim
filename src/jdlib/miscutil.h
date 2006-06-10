// ライセンス: 最新のGPL

// 文字列関係の関数

#ifndef _MISCUTIL_H
#define _MISCUTIL_H

#include <string>
#include <list>

namespace MISC
{
    // str を "\n" ごとに区切ってlistにして出力
    // rm_space == true なら各行の前後の空白を削る
    std::list< std::string > get_lines( const std::string& str, bool rm_space = false );

    // strを空白または "" 単位で区切って list で出力
    std::list< std::string > split_line( const std::string& str );

    // strを delimで区切って list で出力
    std::list< std::string > StringTokenizer( const std::string& str, char delim );

    // emacs lisp のリスト型を要素ごとにlistにして出力
    std::list< std::string > get_elisp_lists( const std::string& str );
    
    // strの前後の空白削除
    std::string remove_space( const std::string& str );

    // str1, str2 に囲まれた文字列を切り出す
    std::string cut_str( const std::string& str, const std::string& str1, const std::string& str2 );

    // str1 を str2 に置き換え
    std::string replace_str( const std::string& str, const std::string& str1, const std::string& str2 );

    // " を \" に置き換え
    std::string replace_quot( const std::string& str );

    // \" を " に置き換え
    std::string recover_quot( const std::string& str );
    
    //文字 -> 整数変換
    int str_to_uint( const char* str, unsigned int& dig, unsigned int& n );

    // 数字　-> 文字変換
    std::string itostr( int n );

    // urlエンコード
    std::string url_encode( const char* str, size_t n );

    //文字コード変換して url エンコード
    std::string charset_url_encode( const std::string& str, const std::string& charset );

    // UTF8の文字列に変換
    std::string strtoutf8( const std::string& str, const std::string& charset );

    // utf-8 -> ucs2 変換
    int utf8toucs2( const char* utfstr, int& byte );

    // ucs2 -> utf8 変換
    int ucs2utf8( int ucs2, char* utfstr );

    // str を大文字化
    std::string toupper_str( const std::string& str );

    // list 内のアイテムを全部大文字化
    std::list< std::string > toupper_list( std::list< std::string >& list_str );
    
    //str を小文字化
    std::string tolower_str( const std::string& str );

    // path からホスト名だけ取り出す
    std::string get_hostname( const std::string& path );

    // path からファイル名だけ取り出す
    std::string get_filename( const std::string& path );

    // path からファイル名を除いてディレクトリだけ取り出す
    std::string get_dir( const std::string& path );
}


#endif
