// ライセンス: GPL2

//
// ファイルローダに渡すデータクラス
//

#ifndef _LOADERDATA_H
#define _LOADERDATA_H

#include <string>
#include <list>

namespace JDLIB
{
    class LOADERDATA
    {
    public:

        bool head;
        
        size_t length; // Content-Length の値
        size_t length_current; // 解凍前の現在までに読み込んでいるサイズ。よって length_current <= length
        size_t size_data; // 解凍したあとの純粋なデータサイズ。よって length < size_data となる場合もある
        
        std::string url;
        std::string protocol;
        std::string host;
        std::string path;
        int port;
        bool use_ssl; // https

        std::string str_post;

        int protocol_proxy;
        std::string host_proxy;
        int port_proxy;
        std::string basicauth_proxy; // proxy 認証

        std::string agent;
        std::string origin;
        std::string referer;
        std::string accept;
        std::string ex_field;  // 送信時にヘッダに追加するフィールド
        std::string str_header;
        int code;
        std::string str_code;
        std::string date;
        std::string modified;
        size_t byte_readfrom;
        std::string cookie_for_request;
        std::list< std::string > list_cookies;
        std::string location;
        std::string contenttype;
        std::string basicauth;
        size_t size_buf;
        int timeout;

        LOADERDATA();

        void init();

        // 一般データのダウンロード用初期化
        void init_for_data();
    };
}

#endif
