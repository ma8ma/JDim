// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misccharcode.h"
#include "miscutil.h"
#include "miscmsg.h"
#include "jdiconv.h"
#include "jdregex.h"
#include "hkana.h"

#include "dbtree/spchar_decoder.h"
#include "dbtree/node.h"

#include "cssmanager.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>


//
// str を "\n" ごとに区切ってlistにして出力
//
std::list< std::string > MISC::get_lines( const std::string& str ){
        
    std::list< std::string > lines;
    size_t i = 0, i2 = 0, r = 0;
    while ( ( i2 = str.find( '\n', i ) ) != std::string::npos ){
        r = 0;
        if( (i2 >= 1) && (str[ i2 - 1 ] == '\r') ) r = 1;
        if( i2 - i > 0 ){
            lines.push_back( str.substr( i, i2 - i - r ) );
        }
        i = i2 + 1;
    }

    // 最後の行
    if( i != str.length() +1 ) lines.push_back( str.substr( i ) );
    
    return lines;
}


//
// emacs lisp のリスト型を要素ごとにlistにして出力
//
std::list< std::string > MISC::get_elisp_lists( const std::string& str )
{
#ifdef _DEBUG
    std::cout << "MISC::get_elisp_lists\n";
#endif 
   
    std::list< std::string > lists;
    size_t pos = 0, length = 0;
    std::string str2 = remove_space( str );
    const char* data = str2.c_str();
    if( data[ 0 ] != '(' ) return lists;

    pos = 1;

    while( data[ pos ] != '\0' ){

        // 空白削除
        while( data[ pos ] == ' ' && data[ pos ] != '\0' ) ++pos;
        if( data[ pos ] == '\0' ) break;

        length = 1;

        // (が現れた
        if( data[ pos ] == '(' ){

            int count = 1;
            while( data[ pos + length ] != '\0' ){
                if( data[ pos + length ] == ')' ) --count;
                else if( data[ pos + length ] == '(' ) ++count;
                ++length;
                if( ! count ) break;
            }
        }

        // 改行 or データが壊れてる
        else if( data[ pos ] == '\n' || data[ pos ] == ')' ){
            ++pos;
            continue;
        }

        // 通常データ
        else{
            //空白 or ) を探す
            while( data[ pos + length ] != ' ' && data[ pos + length ] != ')' && data[ pos + length ] != '\0' ) ++length;
        }

#ifdef _DEBUG
        std::cout << "pos = " << pos << " length = " << length << std::endl;
#endif

        lists.push_back( str2.substr( pos, length ) );
        pos += length;
    }

#ifdef _DEBUG
    std::list < std::string >::iterator it;
    for( it = lists.begin(); it != lists.end(); ++it ) std::cout << "[" << *it << "]" << std::endl;
#endif
    
    return lists;
}

//
// strを空白または "" 単位で区切って list で出力
//
std::list< std::string > MISC::split_line( const std::string& str )
{
    constexpr const char* str_space = u8"\u3000"; // "\xE3\x80\x80" 全角スペース
    constexpr size_t lng_space = 3;
    bool dquote;

    std::list< std::string > list_str;

    size_t i = 0, i2 = 0, lng = str.length();
    for(;;){

        // 空白を取る
        while( 1 ){

            // 半角
            if( str[ i ] == ' ' ) ++i;

            // 全角
            else if( str[ i ] == str_space[ 0 ] &&
                     str[ i +1 ] == str_space[ 1 ] &&
                     str[ i +2 ] == str_space[ 2 ] ) i += lng_space;

            else break;
        }

        // " から始まる ( \"は除く )
        dquote = false;
        if( str[ i ] == '\"' && (i < 1 || str[ i -1 ] != '\\') ){
            dquote = true;
            ++i;
        }

        // 空白か " を探す
        i2 = i;
        size_t lng_tmp = 1;
        while( i2 < lng ){

            // " 発見( \"は除く )
            if( dquote ){
                if( str[ i2 ] == '\"' && str[ i2-1 ] != '\\' ) break;
            }
            else{
                // 半角
                if( str[ i2 ] == ' ' ) break;

                // 全角
                else if( str[ i2 ] == str_space[ 0 ] &&
                         str[ i2 +1 ] == str_space[ 1 ] &&
                         str[ i2 +2 ] == str_space[ 2 ] ){
                    lng_tmp = lng_space;
                    break;
                }
            }

            ++i2;
        }

        if( i2 - i ) list_str.push_back( str.substr( i, i2 - i ) );
        if( i2 >= lng ) break;
        i = i2 + lng_tmp;
    }
    
    return list_str;
}


// strを delimで区切って list で出力
std::list< std::string > MISC::StringTokenizer( const std::string& str, const char delim )
{
    std::list< std::string > list_str;

    size_t i = 0, i2 = 0, lng = str.length();
    for(;;){

        while( i2 < lng && str[ i2++ ] != delim );
        int tmp = ( i2 >= 1 && ( str[ i2-1 ] == delim || str[ i2 -1 ] == '\n' ) ) ? 1 : 0;
        if( i2 - i ) list_str.push_back( str.substr( i, i2 - i - tmp ) );
        if( i2 >= lng ) break;
        i = i2;
    }
    
    return list_str;
}



//
// list_inから空白行を除いてリストを返す
//
std::list< std::string > MISC::remove_nullline_from_list( const std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( ! tmp_str.empty() ) list_ret.push_back( *it );
    }

    return list_ret;
}


//
// list_inの各行から前後の空白を除いてリストを返す
//
std::list< std::string > MISC::remove_space_from_list( const std::list< std::string >& list_in )
{
    std::list< std::string > list_ret;
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        list_ret.push_back( tmp_str );
    }

    return list_ret;
}


//
// list_inからコメント行(#)を除いてリストを返す
//
std::list< std::string > MISC::remove_commentline_from_list( const std::list< std::string >& list_in )
{
    const char commentchr = '#';

    std::list< std::string > list_ret;
    std::list< std::string >::const_iterator it;    
    for( it = list_in.begin(); it != list_in.end(); ++it ){
        std::string tmp_str = MISC::remove_space( (*it) );
        if( tmp_str.c_str()[ 0 ] != commentchr ) list_ret.push_back( *it );
    }

    return list_ret;
}



//
// 空白と""で区切られた str_in の文字列をリストにして出力
//
// \"は " に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""  → aaa と bbb と "ccc"
//
std::list< std::string > MISC::strtolist( const std::string& str_in )
{
    std::list< std::string > list_tmp;
    std::list< std::string > list_ret;

    list_tmp = MISC::split_line( str_in );
    std::list< std::string >::iterator it = list_tmp.begin();
    for( ; it != list_tmp.end(); ++it ){
        if( !( *it ).empty() ) list_ret.push_back( MISC::recover_quot( ( *it ) ) );
    }

    return list_ret;
}



//
// list_in の文字列リストを空白と""で区切ってストリングにして出力
//
// "は \" に置換される
//
// (例)  "aaa" "bbb" "\"ccc\""
//
std::string MISC::listtostr( const std::list< std::string >& list_in )
{
    std::string str_out;
    std::list< std::string >::const_iterator it = list_in.begin();
    for( ; it != list_in.end(); ++it ){
        if( ! ( *it ).empty() ) str_out += " \"" + MISC::replace_quot( ( *it ) )  + "\"";
    }

    return str_out;
}



//
// strの前後の空白削除
//
std::string MISC::remove_space( const std::string& str )
{
    constexpr const char* str_space = u8"\u3000"; // "\xE3\x80\x80" 全角スペース
    constexpr size_t lng_space = 3;

    size_t lng = str.length();
    
    if( lng == 0 ) return str;
    if( str.find( ' ' ) == std::string::npos ) return str;

    // 前
    size_t i = 0;
    while( 1 ){

        // 半角
        if( str[ i ] == ' ' ) ++i;

        // 全角
        else if( str[ i ] == str_space[ 0 ] &&
                 str[ i +1 ] == str_space[ 1 ] &&
                 str[ i +2 ] == str_space[ 2 ] ) i += lng_space;
        else break;
    }

    // 後
    size_t i2 = lng -1;
    while( 1 ){

        // 半角
        if( str[ i2 ] == ' ' ) --i2;

        // 全角
        else if( i2 +1 >= lng_space &&
                 str[ i2 - lng_space +1 ] == str_space[ 0 ] &&
                 str[ i2 - lng_space +2 ] == str_space[ 1 ] &&
                 str[ i2 - lng_space +3 ] == str_space[ 2 ] ) i2 -= lng_space;
        else break;
    }

    return str.substr( i, i2 - i + 1 );
}


//
// str前後の改行、タブ、スペースを削除
//
std::string MISC::remove_spaces( const std::string& str )
{
    if( str.empty() ) return std::string();

    size_t l = 0, r = str.length();

    while( l < r
         && ( str[l] == '\n'
           || str[l] == '\r'
           || str[l] == '\t'
           || str[l] == ' ' ) ) ++l;

    // 最後の文字の位置は文字数より1少ない
    size_t p = r - 1;
    while( p > l
         && ( str[p] == '\n'
           || str[p] == '\r'
           || str[p] == '\t'
           || str[p] == ' ' ) ) --p;

    if( l == 0 && p == r - 1 ) return str;

    return str.substr( l, p + 1 - l );
}



//
// str1からstr2で示された文字列を除く
//
std::string MISC::remove_str( const std::string& str1, const std::string& str2 )
{
    return MISC::replace_str( str1, str2, "" );
}


//
// start 〜 end の範囲をstrから取り除く ( /* コメント */ など )
//
std::string MISC::remove_str( const std::string& str, const std::string& start, const std::string& end )
{
    std::string str_out = str;

    size_t l_pos = 0, r_pos = 0;
    const size_t start_length = start.length();
    const size_t end_length = end.length();

    while( ( l_pos = str_out.find( start, l_pos ) ) != std::string::npos &&
            ( r_pos = str_out.find( end, l_pos + start_length ) ) != std::string::npos )
    {
        str_out.erase( l_pos, r_pos - l_pos + end_length );
    }

    return str_out;
}



//
// 正規表現を使ってstr1からqueryで示された文字列を除く
//
std::string MISC::remove_str_regex( const std::string& str1, const std::string& query )
{
    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;
    if( ! regex.exec( query, str1, offset, icase, newline, usemigemo, wchar ) ) return std::string();
    return MISC::remove_str( str1, regex.str( 0 ) );
}


//
// str1, str2 に囲まれた文字列を切り出す
//
std::string MISC::cut_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    size_t i = str.find( str1 );
    if( i == std::string::npos ) return std::string();
    i += str1.length();
    size_t i2 = str.find( str2, i );
    if( i2 == std::string::npos ) return std::string();
    
    return str.substr( i, i2 - i );
}


//
// str1 を str2 に置き換え
//
std::string MISC::replace_str( const std::string& str, const std::string& str1, const std::string& str2 )
{
    size_t i, pos = 0;
    if( ( i = str.find( str1 , pos ) ) == std::string::npos ) return str;

    std::string str_out;
    str_out.reserve( str.length() );

    do {
        str_out.append( str, pos, ( i - pos ) );
        str_out.append( str2 );
        pos = i + str1.length();
    }
    while( ( i = str.find( str1 , pos ) ) != std::string::npos );

    str_out.append( str, pos, str.length() );
    return str_out;
}

//
// str1 を str2 に置き換え(ignore case)
//
std::string MISC::replace_casestr( const std::string& str, const std::string& str1, const std::string& str2 )
{
    std::string str_out;

//#ifndef _GNU_SOURCE
#if 0
    // 日本語ロケールだとラテン文字[a-zA-Z]以外はcase判定しないもよう
    // コードは簡潔になるが遅くなるし旨味がない

    const char *p0 = str.c_ctr(), *p1;

    while( ( p1 = strcasestr( p0, str1.c_str() ) ) != NULL ){
        str_out.append( p0, p1 - p0 );
        str_out.append( str2 );
        p0 = p1 + str1.length();
    }
#else
    char accept[2] = { 0, 0 };

    if( ( str1[ 0 ] >= 'A' && str1[ 0 ] <= 'Z' ) || ( str1[ 0 ] >= 'a' && str1[ 0 ] <= 'z' ) ){
        accept[ 0 ] = str1[ 0 ] & ~0x20;
        accept[ 1 ] = str1[ 0 ] | 0x20;
    }
    else accept[ 0 ] = str1[ 0 ];

    const char *p0, *p1, *p2;
    p0 = p1 = str.c_str();

    struct searcheither{
        inline static const char* fn( const char *p, const char* a ){
            while( *p != a[ 0 ] && *p != a[ 1 ] && *p != '\0' ) ++p;
            return ( *p == '\0' ) ? nullptr : p;
        }
    };

    while( ( p2 = searcheither::fn( p1, accept ) ) != nullptr ){

        if( strncasecmp( p2, str1.c_str(), str1.length() ) == 0 ){
            str_out.append( p0, p2 - p0 );
            str_out.append( str2 );
            p0 = p1 = p2 + str1.length();
            continue;
        }
        p1 = p2 + 1;
    }
#endif

    if( p0 == str.c_str() ) return str;

    str_out.append( str, p0 - str.c_str(), std::string::npos );

    return str_out;
}


//
// list_inから str1 を str2 に置き換えてリストを返す
//
std::list< std::string > MISC::replace_str_list( const std::list< std::string >& list_in,
                                                 const std::string& str1, const std::string& str2 )
{
    std::list< std::string > list_out;
    for( const std::string& s : list_in ) {
        list_out.push_back( MISC::replace_str( s, str1, str2 ) );
    }
    return list_out;
}


//
// str_in に含まれる改行文字を replace に置き換え
//
std::string MISC::replace_newlines_to_str( const std::string& str_in, const std::string& replace )
{
    if( str_in.empty() || replace.empty() ) return str_in;

    std::string str_out;
    str_out.reserve( str_in.length() );

    size_t pos = 0, found = 0;
    while( ( found = str_in.find_first_of( "\r\n", pos ) ) != std::string::npos )
    {
        str_out.append( str_in, pos, ( found - pos ) );
        str_out.append( replace );

        pos = found + 1;

        if( str_in[ found ] == '\r' && str_in[ found + 1 ] == '\n' ) ++pos;
    }

    str_out.append( str_in, pos, str_in.length() );

    return str_out;
}


//
// str 中に含まれている str2 の 数を返す
//
int MISC::count_str( const std::string& str, const std::string& str2  )
{
    int count = 0;
    size_t found, pos = 0;

    while( ( found = str.find( str2, pos ) ) != std::string::npos )
    {
        ++count;
        pos = found + 1;
    }

    return count;
}


//
// 文字列(utf-8も) -> 整数変換
//
// (例) "12３" -> 123
//
// 入力:
// str
//
// 出力:
// dig: 桁数、0なら失敗
// n : str から何バイト読み取ったか
//
// 戻り値: 数値
//
int MISC::str_to_uint( const char* str, size_t& dig, size_t& n )
{
    int out = 0;
    dig = 0;
    n = 0;
    while( *str != '\0' ){

        const unsigned char in = (*str);
        
        if( '0' <=  in && in <= '9' ){

            out = out*10 + ( in - '0' );
            ++dig;
            ++str;
            ++n;
        }

        else{

            const unsigned char in2 = (* ( str +1 ));
            const unsigned char in3 = (* ( str +2 ));

            // utf-8
            if( in == 0xef && in2 == 0xbc && ( 0x90 <= in3 && in3 <= 0x99 ) ){
                out = out*10 + ( in3 - 0x90 );
                ++dig;
                str += 3;
                n += 3;
            }

            else break;
        }
    }

    return out;
}


//
// listで指定した数字を文字に変換
//
std::string MISC::intlisttostr( const std::list< int >& list_num )
{
    std::ostringstream comment;

    std::list < int >::const_iterator it = list_num.begin();

    bool comma = false;
    int num_from = *it;
    int num_to = -1;
    int i = 0;
    for(;;){

        ++i;
        ++it;
        const int num = *it;
        if( num_from + i != num || it == list_num.end() ){

            if( comma ) comment << ",";
            comment << num_from;
            if( num_to != -1 ) comment << "-" << num_to;
            num_from = num;
            num_to = -1;
            i = 0;
            comma = true;

            if( it == list_num.end() ) break;
        }
        else num_to = num;
    }

    return comment.str();
}



//
// 16進数表記文字列をバイト列に変換する( 例 "E38182" -> "\xE3\x81\x82" )
//
// 出力 : char_out 
// 戻り値: 変換に成功した chr_in のバイト数
//
size_t MISC::chrtobin( const char* chr_in, char* chr_out )
{
    if( ! chr_in ) return 0;

    const size_t chr_in_length = strlen( chr_in );

    size_t a;
    for( a = 0; a < chr_in_length; ++a )
    {
        const unsigned int chr = static_cast< unsigned char >( chr_in[a] );

        *chr_out <<= 4;

        // 0(0x30)〜9(0x39)
        if( ( chr - 0x30 ) < 10 ) *chr_out |= chr - 0x30;
        // A(0x41)〜F(0x46)
        else if( ( chr - 0x41 ) < 6 ) *chr_out |= chr - 0x37;
        // a(0x61)〜f(0x66)
        else if( ( chr - 0x61 ) < 6 ) *chr_out |= chr - 0x57;
        // その他
        else break;

        if( a % 2 != 0 ) ++chr_out;
    }

    return a;
}


//
// strが半角でmaxsize文字を超えたらカットして後ろに...を付ける
//
std::string MISC::cut_str( const std::string& str, const unsigned int maxsize )
{
    std::string outstr = str;
    unsigned int pos, lng_str;
    int byte = 0;
    const size_t outstr_length = outstr.length();

    for( pos = 0, lng_str = 0; pos < outstr_length; pos += byte ){
        byte = MISC::utf8bytes( outstr.c_str() + pos );
        if( byte > 1 ) lng_str += 2;
        else ++lng_str;
        if( lng_str >= maxsize ) break;
    }

    // カットしたら"..."をつける
    if( pos != outstr_length ) outstr = outstr.substr( 0, pos ) + "...";

    return outstr;
}


//
// 正規表現のメタ文字が含まれているか
//
// escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
//

#define REGEX_METACHARS ".+*?^$|{}[]()\\"

bool MISC::has_regex_metachar( const std::string& str, const bool escape )
{
    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( escape && str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ]  ) break;
                ++i;
            }

            if( metachars[ i ] == '\0' ) return true;

            ++pos;
        }
        else{

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos ] == metachars[ i ] ) return true;
                ++i;
            }
        }
    }

    return false;
}


//
// 正規表現のメタ文字をエスケープ
//
// escape == true ならエスケープを考慮 (例)  escape == true なら \+ → \+ のまま、falseなら \+ → \\\+
//
std::string MISC::regex_escape( const std::string& str, const bool escape )
{
    if( ! has_regex_metachar( str, escape ) ) return str;

#ifdef _DEBUG
    std::cout << "MISC::regex_escape" << std::endl;
#endif

    std::string str_out;

    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( escape && str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ]  ) break;
                ++i;
            }

            if( metachars[ i ] == '\0' ) str_out += '\\';
            else{
                str_out += str[ pos ];
                ++pos;
            }
        }
        else{

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos ] == metachars[ i ] ){
                    str_out += '\\';
                    break;
                }                    
                ++i;
            }
        }

        str_out += str[ pos ];
    }

#ifdef _DEBUG
    std::cout << str << " -> " << str_out << std::endl;
#endif

    return str_out;
}


//
// 正規表現のメタ文字をアンエスケープ
//
std::string MISC::regex_unescape( const std::string& str )
{
#ifdef _DEBUG
    std::cout << "MISC::regex_unescape" << std::endl;
#endif

    std::string str_out;

    const char metachars[] = REGEX_METACHARS;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos ){

        if( str[ pos ] == '\\' ){

            int i = 0;
            while( metachars[ i ] != '\0' ){

                if( str[ pos + 1 ] == metachars[ i ] ){
                    ++pos;
                    break;
                }
                ++i;
            }
        }

        str_out += str[ pos ];
    }

#ifdef _DEBUG
    std::cout << str << " -> " << str_out << std::endl;
#endif

    return str_out;
}


//
// HTMLエスケープ
//
// completely : URL中でもエスケープする( デフォルト = true )
//
std::string MISC::html_escape( const std::string& str, const bool completely )
{
    if( str.empty() ) return str;

    bool is_url = false;
    int scheme = SCHEME_NONE;
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t pos = 0; pos < str_length; ++pos )
    {
        char tmpchar = str.c_str()[ pos ];

        // URL中はエスケープしない場合
        if( ! completely )
        {
            // URLとして扱うかどうか
            // エスケープには影響がないので loose_url としておく
            if( scheme != SCHEME_NONE ) is_url = MISC::is_url_char( str.c_str() + pos, true );

            // URLスキームが含まれているか判別
            int len = 0;
            if( ! is_url ) scheme = MISC::is_url_scheme( str.c_str() + pos, &len );

            // URLスキームが含まれていた場合は文字数分進めてループに戻る
            if( len > 0 )
            {
                str_out += str.substr( pos, len );
                pos += len - 1; // あとで ++pos される分を引く
                continue;
            }
        }

        // completely = false でURL中ならエスケープしない
        if( is_url ) str_out += tmpchar;
        else if( tmpchar == '&' ) str_out += "&amp;";
        else if( tmpchar == '\"' ) str_out += "&quot;";
        else if( tmpchar == '<' ) str_out += "&lt;";
        else if( tmpchar == '>' ) str_out += "&gt;";
        else str_out += tmpchar;
    }

#ifdef _DEBUG
    if( str != str_out ){
        std::cout << "MISC::html_escape" << std::endl
                  << "str = " << str << std::endl
                  << "out = " << str_out << std::endl;
    }
#endif

    return str_out;
}


//
// HTMLアンエスケープ
//
std::string MISC::html_unescape( const std::string& str )
{
    if( str.empty() ) return str;
    if( str.find( '&' ) == std::string::npos ) return str;

    std::string str_out;
    const char* pos = str.c_str();
    const char* pos_end = pos + str.length();

    while( pos < pos_end ){

        // '&' までコピーする
        while( *pos != '&' && *pos != '\0' ) str_out.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // エスケープ用の文字参照をデコード
        if( memcmp( pos , "&quot;", 6 ) == 0 ){ str_out.push_back( '"' ); pos += 6; }
        else if( memcmp( pos, "&amp;", 5 ) == 0 ){ str_out.push_back( '&' ); pos += 5; }
        else if( memcmp( pos, "&lt;", 4 ) == 0 ){ str_out.push_back( '<' ); pos += 4; }
        else if( memcmp( pos, "&gt;", 4 ) == 0 ){ str_out.push_back( '>' ); pos += 4; }
        else str_out.push_back( *pos++ );
    }

#ifdef _DEBUG
    if( str != str_out ){
        std::cout << "MISC::html_unescape\nstr = " << str << std::endl
                  << "out = " << str_out << std::endl;
    }
#endif

    return str_out;
}



//
// 特殊文字のデコード内部処理
//
// strは'&'で始まる文字列を指定すること
// completely = true の時は'"' '&' '<' '>'も含めて変換する
//
static std::string spchar_impl( const char* str, int& n_in, const char pre_char, const bool completely )
{
    constexpr int bufsize = 32;
    char out_char[ bufsize ];
    int n_out;
    const int type = DBTREE::decode_char( str, n_in, out_char, n_out, false );

    // 改行、タブ、スペースの処理
    if( type == DBTREE::NODE_SP && pre_char != ' ' ){
        out_char[ 0 ] = ' ';
        n_out = 1;
    }
    // 変換できない文字
    else if( type == DBTREE::NODE_NONE ){
        out_char[ 0 ] = *str;
        n_out = 1;
        n_in = 1;
    }
    // エスケープする文字の場合は元に戻す
    else if( ! completely && n_out == 1 ){
        switch( *out_char ){
            case '"':
                out_char[ 0 ] = '&'; out_char[ 1 ] = 'q'; out_char[ 2 ] = 'u';
                out_char[ 3 ] = 'o'; out_char[ 4 ] = 't'; out_char[ 5 ] = ';';
                n_out = 6;
                break;
            case '&':
                out_char[ 0 ] = '&'; out_char[ 1 ] = 'a'; out_char[ 2 ] = 'm';
                out_char[ 3 ] = 'p'; out_char[ 4 ] = ';';
                n_out = 5;
                break;
            case '<':
                out_char[ 0 ] = '&'; out_char[ 1 ] = 'l'; out_char[ 2 ] = 't';
                out_char[ 3 ] = ';';
                n_out = 4;
                break;
            case '>':
                out_char[ 0 ] = '&'; out_char[ 1 ] = 'g'; out_char[ 2 ] = 't';
                out_char[ 3 ] = ';';
                n_out = 4;
                break;
            default:
                break;
        }
    }

    return std::string( out_char, n_out );
}


//
// HTMLをプレーンテキストに変換する
//
std::string MISC::to_plain( const std::string& html )
{
    if( html.empty() ) return html;
    if( html.find_first_of( '<' ) == std::string::npos
            && html.find_first_of( '&' ) == std::string::npos ) return html;

    std::string str_out;
    const char* pos = html.c_str();
    const char* pos_end = pos + html.length();

    while( pos < pos_end ){

        // '<' か '&' までコピーする
        while( *pos != '<' && *pos != '&' && *pos != '\0' ) str_out.push_back( *pos++ );
        if( pos >= pos_end ) break;

        // タグを取り除く
        if( *pos == '<' ){
            while( *pos != '>' && *pos != '\0' ) pos++;
            if( *pos == '>' ) ++pos;
            continue;
        }

        // 文字参照を処理する
        if( *pos == '&' ){
            int n_in;
            char pre = str_out.length() ? *( str_out.end() - 1 ) : 0;
            str_out += spchar_impl( pos, n_in, pre, true );
            pos += n_in;
        }
    }

#ifdef _DEBUG
    if( html != str_out )
        std::cout << "MISC::to_plain" << std::endl
                  << "html = " << html << std::endl
                  << "plain = " << str_out << std::endl;
#endif

    return str_out;
}


//
// URL中のスキームを判別する
//
// 戻り値 : スキームタイプ
// length    : "http://"等の文字数
//
int MISC::is_url_scheme_impl( const char* str_in, int* length )
{
    int scheme = SCHEME_NONE;
    int len = 0;

    // http https
    if( *str_in == 'h' && *( str_in + 1 ) == 't'
        && *( str_in + 2 ) == 't' && *( str_in + 3 ) == 'p' )
    {
        scheme = SCHEME_HTTP;
        len = 4;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // ftp
    else if( *str_in == 'f' && *( str_in + 1 ) == 't' && *( str_in + 2 ) == 'p' )
    {
        scheme = SCHEME_FTP;
        len = 3;
    }
    // ttp ttps
    else if( *str_in == 't' && *( str_in + 1 ) == 't' && *( str_in + 2 ) == 'p' )
    {
        scheme = SCHEME_TTP;
        len = 3;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // tp tps
    else if( *str_in == 't' && *( str_in + 1 ) == 'p' )
    {
        scheme = SCHEME_TP;
        len = 2;
        if( *( str_in + len ) == 's' ) ++len;
    }
    // sssp
    else if( *str_in == 's' && *( str_in + 1 ) == 's' && *( str_in + 2 ) == 's' && *( str_in + 3 ) == 'p' ){
        if( *( str_in + 7 ) == 'i' && *( str_in + 8 ) == 'm' && *( str_in + 9 ) == 'g' && *( str_in + 10 ) == '.'
             && *( str_in + 11 ) == '2' && *( str_in + 12 ) == 'c' && *( str_in + 13 ) == 'h'){
            scheme = SCHEME_SSSP;
        }
        else{
            // XXX img.2ch以外のアドレスはHTTPスキームにする
            scheme = SCHEME_HTTP;
        }
        len = 4;
    }

    // 各スキーム後に続く共通の"://"
    if( *( str_in + len ) == ':' && *( str_in + len + 1 ) == '/'
        && *( str_in + len + 2 ) == '/' )
    {
        len += 3;
        if( length ) *length = len;
    }
    else scheme = SCHEME_NONE;

    return scheme;
}


//
// URLとして扱う文字かどうか判別する
//
// 基本 : 「!#$%&'()*+,-./0-9:;=?@A-Z_a-z~」
// 拡張 : 「[]^|」
//
// "RFC 3986" : http://www.ietf.org/rfc/rfc3986.txt
// "RFC 2396" : http://www.ietf.org/rfc/rfc2396.txt
//
static const char s_url_char[ 128 ] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//         !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
//      @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 2, 1,
//      `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//      p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 1, 0,
};
bool MISC::is_url_char( const char* str_in, const bool loose_url )
{
    unsigned char c = (unsigned char)(*str_in);

    // 128以上のテーブルはないので先に判定
    if( c & 0x80 ) return false;

    // 基本
    if( s_url_char[ c ] == 1 ) return true;

    // 拡張
    // RFC 3986(2.2.)では"[]"が予約文字として定義されているが
    // RFC 2396(2.4.3.)では除外されていて、普通にURLとして扱う
    // と問題がありそうなので"loose_url"の扱いにしておく。
    if( loose_url && s_url_char[ c ] == 2 ) return true;

    return false;
}



//
// URLデコード
//
std::string MISC::url_decode( const char* url, const size_t n )
{
    std::string decoded;
    if( n == 0 ) return decoded;

    unsigned int a;
    for( a = 0; a < n; ++a )
    {
        if( url[a] == '%' && ( a + 2 ) < n )
        {
            char src[3] = { url[ a + 1 ], url[ a + 2 ], '\0' };
            char tmp[3] = { '\0', '\0', '\0' };

            if( chrtobin( src, tmp ) == 2 )
            {
                // 改行はLFにする
                if( *tmp == '\n' && !decoded.empty() && decoded.back() == '\r' ){
                    decoded.pop_back();
                }
                // '%4A' など、2文字が変換できていること
                decoded.push_back( *tmp );
                a += 2;
            }
            else
            {
                // 変換失敗は、単なる '%' 文字として扱う
                decoded.push_back( url[a] );
            }
        }
        else if( url[a] == '+' )
        {
            decoded.push_back( ' ' );
        }
        else
        {
            decoded.push_back( url[a] );
        }
    }

    return decoded;
}


//
// url エンコード
//
std::string MISC::url_encode( const char* str, const size_t n, const CharCode coding )
{
    if( str[ n ] != '\0' ){
        ERRMSG( "url_encode : invalid input." );
        return std::string();
    }

    std::string str_encoded;
    std::unique_ptr< JDLIB::Iconv > icv;
    if( coding != CHARCODE_UTF8 ) icv.reset( new JDLIB::Iconv( CHARCODE_UTF8, coding ) );
    
    size_t pos = 0;
    while( pos < n ){
        
        const int tmplng = 16;
        char str_tmp[ tmplng ];
        
        if( icv ){
            size_t pos_start = pos;
            while( str[ pos ] & 0x80 ) ++pos;
            if( pos != pos_start ){
                int byte_out;
                const std::string str_enc = icv->convert( str + pos_start, pos - pos_start, byte_out );

                // マルチバイト文字は全てパーセントエンコードする
                for( int i = 0; i < byte_out; ++i ){
                    unsigned char c = str_enc[ i ];
                    snprintf( str_tmp, tmplng , "%%%02X", c );
                    str_encoded += str_tmp;
                }

                if( pos >= n ) break;
            }
        }

        const unsigned char c = str[ pos ];

        // 非予約文字はそのまま
        if( ( 'a' <= c && c <= 'z' ) || ( 'A' <= c && c <= 'Z' )
            || ( '0' <= c && c <= '9' )
            || ( c == '-' ) || ( c == '.' ) || ( c == '_' ) || ( c == '~' ) ){
            str_tmp[ 0 ] = c;
            str_tmp[ 1 ] = '\0';
        }
        // スペースは'+'に置換
        else if( c == ' ' ){
            str_tmp[ 0 ] = '+';
            str_tmp[ 1 ] = '\0';
        }
        // 改行を正規化
        else if( c == '\n' ){
            str_tmp[ 0 ] = '%'; str_tmp[ 1 ] = '0'; str_tmp[ 2 ] = 'D'; // CR ( %0d )
            str_tmp[ 3 ] = '%'; str_tmp[ 4 ] = '0'; str_tmp[ 5 ] = 'A'; // LF ( %0a )
            str_tmp[ 6 ] = '\0';
        }
        // CR は無視
        else if( c == '\r' ){
            str_tmp[ 0 ] = '\0';
        }
        // その他はパーセントエンコード
        else{
            snprintf( str_tmp, tmplng , "%%%02X", c );
        }

        str_encoded += str_tmp;
        ++pos;
    }

    return str_encoded;
}


//
// 半角スペースまたは "" 単位で区切って url エンコード
//
std::string MISC::url_encode_split( const std::string& str, const CharCode charcode )
{
    std::list< std::string > list_str = MISC::split_line( str );
    std::list< std::string >::iterator it = list_str.begin();
    std::string str_out;
    for( ; it != list_str.end(); ++it ){

        if( it != list_str.begin() ) str_out += "+";
        str_out += MISC::url_encode( it->c_str(), it->length(), charcode );
    }

    return str_out;
}


//
// BASE64
//
std::string MISC::base64( const std::string& str )
{
    char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int lng = str.length();

    std::string out;
    out.reserve( lng * 2 );

    std::string data = str + "\0\0\0\0";

    for( int i = 0; i < lng; i += 3 ){

        unsigned char* cstr = (unsigned char*)( data.c_str() + i );
        unsigned char key[ 4 ];

        key[ 0 ] = (*cstr) >> 2;
        key[ 1 ] = ( ( (*cstr) << 4 ) + ( (*(cstr+1)) >> 4 ) );
        key[ 2 ] = ( ( (*(cstr+1)) << 2 ) + ( (*(cstr+2)) >> 6 ) );
        key[ 3 ] = *(cstr+2);

        for( int j = 0; j < 4; ++j ){
            key[ j ] &= 0x3f;
            out += table[ key[ j ] ];
        }
    }

    if( lng % 3 == 1 ){
        out[ out.length()-2 ] = '=';
        out[ out.length()-1 ] = '=';
    }
    else if( lng % 3 == 2 ){
        out[ out.length()-1 ] = '=';
    }

#ifdef _DEBUG
    std::cout << "MISC::base64 " << str << " -> " << out << std::endl;
#endif 

    return out;
}



//
// 「&#数字;」形式の数字参照文字列の中の「数字」部分の文字列長
//
// in_char: 入力文字列、in_char[0] == "&" && in_char[1] == "#" であること
// offset : 開始位置が返る
//
// 戻り値 : 「&#数字;」の中の数字の文字列の長さ、変換出来ないときは -1
//
// 例 : &#9999; なら 戻り値 = 4、 offset = 2
//
int MISC::spchar_number_ln( const char* in_char, int& offset )
{
    int lng = 0;
    offset = 2;

    // offset == 2 なら 10 進数、3 なら16進数
    if( in_char[ offset ] == 'x' || in_char[ offset ] == 'X' ) ++offset;

    // UCS-4の範囲でデコードするので最大1114111
    // デコードするとき「;」で終端されていなくてもよい

    // デコード可能かチェック
    // 10 進数
    if( offset == 2 ){

        // 最大7桁 (&#1114111;)
        for( lng = 0; lng <= 7; lng++ ){
            if( in_char[ offset + lng ] < '0' || in_char[ offset + lng ] > '9' ) break;
        }
        
        // 桁数チェック
        if( lng == 0 || lng == 8 ) return -1;
    }

    // 16 進数
    else{

        // 最大6桁 (&#x10FFFF;)
        for( lng = 0; lng <= 6; lng++ ){
            if(
                ! (
                    ( in_char[ offset + lng ] >= '0' && in_char[ offset + lng ] <= '9' )
                    || ( in_char[ offset + lng ] >= 'a' && in_char[ offset + lng ] <= 'f' )
                    || ( in_char[ offset + lng ] >= 'A' && in_char[ offset + lng ] <= 'F' )
                    )
                ) break;
        }
        
        // 桁数チェック
        if( lng == 0 || lng == 7 ) return -1;
    }

    return lng;
}


// 特定の変換が必要なコードポイントをチェックする
static int transform_7f_9f( int raw_point )
{
    switch( raw_point ) {
        case 0x80: return 0x20AC; // EURO SIGN (€)
        case 0x82: return 0x201A; // SINGLE LOW-9 QUOTATION MARK (‚)
        case 0x83: return 0x0192; // LATIN SMALL LETTER F WITH HOOK (ƒ)
        case 0x84: return 0x201E; // DOUBLE LOW-9 QUOTATION MARK („)
        case 0x85: return 0x2026; // HORIZONTAL ELLIPSIS (…)
        case 0x86: return 0x2020; // DAGGER (†)
        case 0x87: return 0x2021; // DOUBLE DAGGER (‡)
        case 0x88: return 0x02C6; // MODIFIER LETTER CIRCUMFLEX ACCENT (ˆ)
        case 0x89: return 0x2030; // PER MILLE SIGN (‰)
        case 0x8A: return 0x0160; // LATIN CAPITAL LETTER S WITH CARON (Š)
        case 0x8B: return 0x2039; // SINGLE LEFT-POINTING ANGLE QUOTATION MARK (‹)
        case 0x8C: return 0x0152; // LATIN CAPITAL LIGATURE OE (Œ)
        case 0x8E: return 0x017D; // LATIN CAPITAL LETTER Z WITH CARON (Ž)
        case 0x91: return 0x2018; // LEFT SINGLE QUOTATION MARK (‘)
        case 0x92: return 0x2019; // RIGHT SINGLE QUOTATION MARK (’)
        case 0x93: return 0x201C; // LEFT DOUBLE QUOTATION MARK (“)
        case 0x94: return 0x201D; // RIGHT DOUBLE QUOTATION MARK (”)
        case 0x95: return 0x2022; // BULLET (•)
        case 0x96: return 0x2013; // EN DASH (–)
        case 0x97: return 0x2014; // EM DASH (—)
        case 0x98: return 0x02DC; // SMALL TILDE (˜)
        case 0x99: return 0x2122; // TRADE MARK SIGN (™)
        case 0x9A: return 0x0161; // LATIN SMALL LETTER S WITH CARON (š)
        case 0x9B: return 0x203A; // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (›)
        case 0x9C: return 0x0153; // LATIN SMALL LIGATURE OE (œ)
        case 0x9E: return 0x017E; // LATIN SMALL LETTER Z WITH CARON (ž)
        case 0x9F: return 0x0178; // LATIN CAPITAL LETTER Y WITH DIAERESIS (Ÿ)
        default:
            return 0xFFFD; // REPLACEMENT CHARACTER
    }
}

//
// コードポイントが数値文字参照としてはエラーなら規定の値へ変換する
// 例えばサロゲートペアは'REPLACEMENT CHARACTER' (U+FFFD)を返す
//
// 参考文献 : Numeric character reference end state (HTML 5.3)
//            https://www.w3.org/TR/html53/syntax.html#numeric-character-reference-end-state
//
static int sanitize_numeric_character_reference( int raw_point )
{
    // NOTE: 記号や絵文字を速やかに処理できるよう順番が組まれている

    bool parse_error = false;

    // 基本多言語面(BMP)をおおまかにチェック
    if( 0x009F < raw_point && raw_point < 0xD800 ) return raw_point;
    // 特定のbitパターンの非文字と符号空間の範囲をチェック
    else if( ( raw_point & 0xFFFE ) == 0xFFFE || raw_point > 0x10FFFF ) parse_error = true;
    // bitパターンを除いたらBMPの一部と追加多言語面(SMP)以降をチェック
    else if( 0xFDEF < raw_point ) return raw_point;
    // サロゲートペアはエラー
    else if( 0xD800 <= raw_point && raw_point <= 0xDFFF ) parse_error = true;

    // 基本ラテン文字をチェック
    else if( 0x001F < raw_point && raw_point < 0x007F ) return raw_point;
    // 特定の変換が必要なコードポイントをチェック
    else if( 0x007F <= raw_point && raw_point <= 0x009F ) return transform_7f_9f( raw_point );
    // 最後に制御文字と非文字をチェック
    // サロゲートペアは他の値より入力される可能性が高いので処理を優先している
    else if( raw_point <= 0x0008 // Control character
            || raw_point == 0x000B // Control character (Vertical tab)
            || ( 0x000D <= raw_point && raw_point <= 0x001F ) // Control character
            || ( 0xFDD0 <= raw_point && raw_point <= 0xFDEF ) // Noncharacters
            ) {
        parse_error = true;
    }

    if( parse_error ) {
#ifdef _DEBUG
        std::cout << "Parse error for numeric character reference... " << raw_point << std::endl;
#endif
        return 0xFFFD; // REPLACEMENT CHARACTER
    }
    return raw_point;
}

//
// 「&#数字;」形式の数字参照文字列を数字(int)に変換する
//
// 最初に MISC::spchar_number_ln() を呼び出して offset と lng を取得すること
//
// in_char: 入力文字列、in_char[0] == "&" && in_char[1] == "#" であること
// offset : spchar_number_ln() の戻り値
// lng : spchar_number_ln() の戻り値
//
// 戻り値 : 「&#数字;」の中の数字(int型)
//
int MISC::decode_spchar_number( const char* in_char, const int offset, const int lng )
{
    char str_num[ 16 ];

    memcpy( str_num, in_char + offset, lng );
    str_num[ lng ] = '\0';

#ifdef _DEBUG
    std::cout << "MISC::decode_spchar_number offset = " << offset << " lng = " << lng << " str = " << str_num << std::endl;
#endif

    int num = 0;
    if( offset == 2 ) num = atoi( str_num );
    else num = strtol( str_num, nullptr, 16 );

    return sanitize_numeric_character_reference( num );
}


//
// str に含まれる「&#数字;」形式の数字参照文字列を全てユニーコード文字に変換する
//
std::string MISC::decode_spchar_number( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length ; ++i ){

        if( str[ i ] == '&' && str[ i + 1 ] == '#' ){

            int offset;
            const int lng = MISC::spchar_number_ln( str.c_str()+i, offset );
            if( lng == -1 ){
                str_out += str[ i ];
                continue;
            }

            const int num = MISC::decode_spchar_number( str.c_str()+i, offset, lng );

            char out_char[ 64 ];
            const int n_out = MISC::cptoutf8( num, out_char );
            if( ! n_out ){
                str_out += str[ i ];
                continue;
            }

            for( int j = 0; j < n_out; ++j ) str_out += out_char[ j ];
            i += offset + lng;
        }
        else str_out += str[ i ];
    }

    return str_out;
}


//
// str を大文字化
//
std::string MISC::toupper_str( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length ; ++i ) str_out += toupper( str[ i ] );

    return str_out;
}


//
// list 内のアイテムを全部大文字化
//
std::list< std::string > MISC::toupper_list( const std::list< std::string >& list_str )
{
    std::list< std::string > list_out;
    std::list< std::string >::const_iterator it = list_str.begin();
    for( ; it != list_str.end() ; ++it ) list_out.push_back( MISC::toupper_str( *it ) );

    return list_out;
}



//
// str を小文字化
//
std::string MISC::tolower_str( const std::string& str )
{
    std::string str_out;
    const size_t str_length = str.length();

    for( size_t i = 0; i < str_length; ++i ) str_out += tolower( str[ i ] );

    return str_out;
}



//
// path からホスト名だけ取り出す
//
// protocol = false のときはプロトコルを除く
//
std::string MISC::get_hostname( const std::string& path, bool protocol )
{
    int lng = 0;
    if( path.find( "http://" ) == 0 ) lng = strlen( "http://" );
    else if( path.find( "https://" ) == 0 ) lng = strlen( "https://" );
    else if( path.find( "ftp://" ) == 0 ) lng = strlen( "ftp://" );
    if( !lng ) return std::string();

    int pos = 0;
    if( ! protocol ) pos = lng;

    size_t i = path.find( "/", lng ); 

    if( i == std::string::npos ) path.substr( pos );

    return path.substr( pos, i - pos );
}



//
// path からファイル名だけ取り出す
//
std::string MISC::get_filename( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( "/" );
    if( i == std::string::npos ) return path;

    return path.substr( i+1 );
}



//
// path からファイル名を除いてディレクトリだけ取り出す
//
std::string MISC::get_dir( const std::string& path )
{
    if( path.empty() ) return std::string();

    size_t i = path.rfind( "/" );
    if( i == std::string::npos ) return std::string();

    return path.substr( 0, i+1 );
}



//
// 文字数を限定して環境変数の値を返す
//
std::string MISC::getenv_limited( const char *name, const size_t size )
{
    if( ! name || ! getenv( name ) ) return std::string();

    std::string env( getenv( name ) );
    if( env.length() > size ) env.resize( size );

#ifdef _WIN32
    return recover_path( Glib::locale_to_utf8( env ) );
#else
    return env;
#endif
}


//
// pathセパレータを / に置き換える
//
std::string MISC::recover_path( const std::string& str )
{
#ifdef _WIN32
    // Windowsのpathセパレータ \ を、jdの / に置き換える
    std::string ret( str );
    for (int i=ret.length()-1; i>=0; i--)
        if (ret[ i ] == '\\')
            ret[ i ] = '/';
    return ret;
#else
    return str;
#endif
}

std::vector< std::string > MISC::recover_path( std::vector< std::string >&& list_str )
{
#ifdef _WIN32
    std::vector< std::string > list_ret;
    std::vector< std::string >::const_iterator it = list_str.begin();
    for( ; it != list_str.end() ; ++it )
        list_ret.push_back( MISC::recover_path( *it ) );
    return list_ret;
#else
    return std::move( list_str );
#endif
}



//
// 文字列(utf-8)に全角英数字が含まれるか判定する
//
bool MISC::has_widechar( const char* str )
{
    while( *str != '\0' ){

        const unsigned char in = *str;

        if( ( in & 0xf0 ) == 0xe0 ){

            if( in == 0xef ){

                const unsigned char in2 = * ( str + 1 );
                const unsigned char in3 = * ( str + 2 );

                if( in2 == 0xbc ){

                    // 全角数字
                    if( 0x90 <= in3 && in3 <= 0x99 ) return true;

                    // 全角大文字
                    else if( 0xa1 <= in3 && in3 <= 0xba ) return true;
                }

                //  全角小文字
                else if( in2 == 0xbd && ( 0x81 <= in3 && in3 <= 0x9a ) ) return true;

                // 半角かな
                else if( ( in2 == 0xbd && ( 0xa1 <= in3 && in3 <= 0xbf ) )
                         || ( in2 == 0xbe && ( 0x80 <= in3 && in3 <= 0x9f ) ) ) return true;
            }

            str += 3;
        }

        else if( ( in & 0xe0 ) == 0xc0 ) str += 2;

        else if( ( in & 0xf8 ) == 0xf0 ) str += 4;

        else ++str;
    }

    return false;
}


//
// 全角英数字(str1) -> 半角英数字(str2)
//
// table_pos : 置き換えた文字列の位置
//
void MISC::asc( const char* str1, std::string& str2, std::vector< int >& table_pos )
{
    for( int pos = 0; str1[ pos ] != '\0'; ) {
        assert( pos >= 0 );
        assert( table_pos.max_size() > table_pos.size() );
        const auto in1 = static_cast< unsigned char >( str1[ pos ] );

        if( in1 == 0xef ) {
            const auto in2 = static_cast< unsigned char >( str1[ pos + 1 ] );
            const auto in3 = static_cast< unsigned char >( str1[ pos + 2 ] );

            if( in2 == 0xbc ){
                //  全角数字 (U+FF10 - U+FF19)
                if( 0x90 <= in3 && in3 <= 0x99 ){
                    str2.push_back( '0' + in3 - 0x90 );
                    table_pos.push_back( pos );
                    pos += 3;
                    continue;
                }
                //  全角大文字 (U+FF21 - U+FF3A)
                else if( 0xa1 <= in3 && in3 <= 0xba ){
                    str2.push_back( 'A' + in3 - 0xa1 );
                    table_pos.push_back( pos );
                    pos += 3;
                    continue;
                }
            }

            //  全角小文字 (U+FF41 - U+FF5A)
            else if( in2 == 0xbd && ( 0x81 <= in3 && in3 <= 0x9a ) ){
                str2.push_back( 'a' + in3 - 0x81 );
                table_pos.push_back( pos );
                pos += 3;
                continue;
            }

            // 半角かな (U+FF61 - U+FF9F)
            else if( ( in2 == 0xbd && ( 0xa1 <= in3 && in3 <= 0xbf ) )
                     || ( in2 == 0xbe && ( 0x80 <= in3 && in3 <= 0x9f ) ) ){

                bool flag_hkana = false;
                bool dakuten = false;
                size_t i = 0;

                // 濁点、半濁点
                const auto in4 = static_cast< unsigned char >( str1[ pos + 3 ] );
                const auto in5 = static_cast< unsigned char >( str1[ pos + 4 ] );
                if( in4 == 0xef && in5 == 0xbe ){
                    const auto in6 = static_cast< unsigned char >( str1[ pos + 5 ] );

                    // 濁点
                    if( in6 == 0x9e ){
                        dakuten = true;
                        i = 61;
                    }
                    // 半濁点
                    else if( in6 == 0x9f ){
                        dakuten = true;
                        i = 61 + 21;
                    }
                }

                while( hkana_table1[ i ][ 0 ][ 0 ] != '\0' ){

                    if( in1 == hkana_table1[ i ][ 0 ][ 0 ]
                            && in2 == hkana_table1[ i ][ 0 ][ 1 ]
                            && in3 == hkana_table1[ i ][ 0 ][ 2 ] ) {

                        std::copy_n( hkana_table1[ i ][ 1 ], 3, std::back_inserter( str2 ) );
                        std::generate_n( std::back_inserter( table_pos ), 3, [&pos]{ return pos++; } );

                        if( dakuten ) pos += 3;
                        flag_hkana = true;
                        break;
                    }
                    ++i;
                }
                if( flag_hkana ) continue;
            }
        }

        str2.push_back( str1[ pos ] );
        table_pos.push_back( pos );
        ++pos;
    }
}


//
// UTF8文字列の正規化
//
// str1 : 変換する文字列
// str2 : 出力先
// table_pos : 置き換えた文字列の位置
//
void MISC::norm( const char* str1, std::string& str2, std::vector< int >* table_pos )
{
    size_t pos1 = 0;
    Glib::ustring ustr;
    std::string str_norm;

    while( str1[ pos1 ] != '\0' ){

        int nbytes = MISC::utf8bytes( str1 + pos1 );
        if( nbytes <= 1 ){
            str2.push_back( str1[ pos1 ] );
            if( table_pos ) table_pos->push_back( pos1 );
            pos1++;
            continue;
        }

        // 異字体は纏める
        int lng;
        const char32_t next = MISC::utf8tocp( str1 + pos1 + nbytes, lng );
        size_t nchars = 1;
        if( ( next >= 0x180b && next <= 0x180d ) ||
            ( next >= 0xfe00 && next <= 0xfe0f ) ||
            ( next >= 0xe0100 && next <= 0xe01ef ) ){
            nchars++;
            nbytes += lng;
        }

        ustr.assign( str1 + pos1, nchars );
        str_norm.assign( ustr.normalize( Glib::NORMALIZE_NFKD ) );

        lng = str_norm.length();

        std::copy( str_norm.begin(), str_norm.end(), std::back_inserter( str2 ) );
        if( table_pos ){
            table_pos->push_back( pos1 );
            std::fill_n( std::back_inserter( *table_pos ), lng -1, -1 );
        }

        pos1 += nbytes;
    }
}
