// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdsocket.h"

#include "miscmsg.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#ifndef MSG_NOSIGNAL
#include <signal.h>
#endif

#include <cerrno>
#include <cstdint>
#include <cstring> // std::strerror
#ifdef _DEBUG
#include <iomanip>
#endif


namespace {

inline bool soc_is_valid( int _soc ) { return _soc >= 0; }
constexpr int kInvlidSocket = -1;

constexpr int kSocket_OK = 0;
constexpr int kSocket_ERR = -1;


inline bool is_ipaddress( const char* addr )
{
    unsigned char buf[ sizeof(struct in6_addr) ];

    if( inet_aton( addr, reinterpret_cast<struct in_addr*>( &buf ) )
            || inet_pton( AF_INET6, addr, &buf ) ) {
        return true;
    }
    return false;
}

} // namespace


#if defined(USE_GNUTLS)
#include "jdsocketgnutls.h"
#elif defined(USE_OPENSSL)
#include "jdsocketopenssl.h"
#endif


using namespace JDLIB;


/** @brief コンストラクター
 *
 * @details 渡された stop は参照で保持するためSocketでは寿命を管理しない
 * @param[in] stop  通信を行うスレッドが中断されたかチェックするための参照 (読み取り専用)
 * @param[in] async true なら非同期通信を行う
 */
Socket::Socket( const std::atomic<bool>& stop, const bool async )
    : m_soc( kInvlidSocket )
    , m_stop( stop )
    , m_async( async )
{
    // nothing to do
}


Socket::~Socket()
{
    if( soc_is_valid( m_soc ) ) {
        close();
    }
}


/** @brief ネットワークホストに接続する
 *
 * @param[in] hostname 接続するホスト
 * @param[in] port     ポート番号の文字列
 * @param[in] use_ipv6 true なら IPv6 を使う
 * @retval true  接続に成功した
 * @retval false 接続に失敗した
 */
bool Socket::connect( const std::string& hostname, const std::string& port, const bool use_ipv6 )
{
#ifdef _DEBUG
    std::cout << "Socket::connect" << std::endl;
#endif

    bool ret = false;
    int optval = 0;
    socklen_t optlen = sizeof(int);

    // addrinfo 取得
    struct addrinfo hints{}, *ainf = nullptr;
    if( use_ipv6 ) {
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
    }
    else hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo( hostname.c_str(), port.c_str(), &hints, &ainf );
    if( err ) {
        m_errmsg = "getaddrinfo failed: " + hostname + ":" + port;
        if( ainf ) freeaddrinfo( ainf );
        return ret;
    }
    if( ! ainf ) return ret;

#ifdef _DEBUG
    const auto family = ainf->ai_addr->sa_family;
    const void* src;
    if( family == AF_INET ) {
        src = &reinterpret_cast<sockaddr_in*>( ainf->ai_addr )->sin_addr;
    }
    else {
        src = &reinterpret_cast<sockaddr_in6*>( ainf->ai_addr )->sin6_addr;
    }
    char buf[ INET6_ADDRSTRLEN ]{};
    if( inet_ntop( family, src, buf, sizeof(buf) ) ) {
        std::cout << "Socket::connect: host=" << std::quoted( hostname ) << ", ip=" << std::quoted( buf ) << std::endl;
    }
#endif

    // ソケット作成
    m_soc = socket( ainf->ai_family, ainf->ai_socktype, ainf->ai_protocol );
    if( ! soc_is_valid( m_soc ) ) {
        m_errmsg = "socket failed";
        goto EXIT_CONNECT;
    }

    // ソケットを非同期に設定
    if( m_async ) {
        const int flags = fcntl( m_soc, F_GETFL, 0);
        if( flags == -1 || fcntl( m_soc, F_SETFL, flags | O_NONBLOCK ) < 0 )
        {
            m_errmsg = "fcntl failed";
            goto EXIT_CONNECT;
        }
    }

    // connect peer
    err = ::connect( m_soc, ainf->ai_addr, ainf->ai_addrlen );
    if( err ) {
        // ノンブロックでまだ接続中
        if( ! m_async || errno != EINPROGRESS ) {
            m_errmsg = "connect failed: " + hostname + ":" + port;
            goto EXIT_CONNECT;
        }
    }

    // connect待ち
    if( ! wait_fds( WaitFor::send ) ) {
        m_errmsg = "connect timeout";
        goto EXIT_CONNECT;
    }

    // connectが成功したかチェック
    if( getsockopt( m_soc, SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>( &optval ), &optlen ) < 0
            || optval ) {
        m_errmsg = "connect(getsockopt) failed: " + hostname + ":" + port;
        goto EXIT_CONNECT;
    }

    ret = true;

EXIT_CONNECT:
    // addrinfo開放
    freeaddrinfo( ainf );

    return ret;
}


/** @brief プロキシサーバーに対してハンドシェイクする
 *
 * @param[in] hostname 接続するプロキシサーバー
 * @param[in] port     ポート番号の文字列
 * @param[in] protocol プロキシのプロトコルを表す数値 (`JDLIB::Proxy` を参照)
 * @retval true  接続に成功した
 * @retval false 接続に失敗した
 */
bool Socket::socks_handshake( const std::string& hostname, const std::string& port, const Proxy protocol )
{
#ifdef _DEBUG
    std::cout << "Socket::socks_handshake" << std::endl;
#endif

    bool ret = false;

    // addrinfo 取得
    std::uint32_t addr = 0;
    if( protocol == Proxy::socks4 /*|| protocol == Proxy::socks5*/ ) {
        struct addrinfo hints{}, *ainf = nullptr;
        // XXX SOCKS5対応の時にIPv6もサポートが必要
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        const int err = getaddrinfo( hostname.c_str(), port.c_str(), &hints, &ainf );
        if( err ) {
            m_errmsg = "getaddrinfo failed: " + hostname + ":" + port;
            return ret;
        }

        if( ainf ) {
            auto addr_in = reinterpret_cast<sockaddr_in*>( ainf->ai_addr );
#ifdef _DEBUG
            std::cout << "Socket::socks_handshake: host=" << std::quoted( hostname )
                      << ", ip=" << std::quoted( inet_ntoa( addr_in->sin_addr ) ) << std::endl;
#endif

            addr = *reinterpret_cast<std::uint32_t*>( &( addr_in->sin_addr ) );
            freeaddrinfo( ainf );
        }
    }

    // ポート番号変換
    const int num_port = std::stoi( port );
    if( num_port < 0 || num_port >= 0x10000 ) {
        m_errmsg = "invlid port number: " + hostname + ":" + port;
        return ret;
    }

    char msgbuf[ 100 ];
    char *p = msgbuf;
    switch( protocol ) {

    case Proxy::socks4:
        *p++ = 4;   // SOCKS Version
        *p++ = 1;   // TCP/IP Stream
        *p++ = ( num_port >> 8 ) & 0xff;
        *p++ = num_port & 0xff;
        *p++ = addr & 0xff;
        *p++ = ( addr >> 8 ) & 0xff;
        *p++ = ( addr >> 16 ) & 0xff;
        *p++ = ( addr >> 24 ) & 0xff;
        *p++ = 0;
        break;

    case Proxy::socks4a:
        *p++ = 4;   // SOCKS Version
        *p++ = 1;   // TCP/IP Stream
        *p++ = ( num_port >> 8 ) & 0xff;
        *p++ = num_port & 0xff;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1; // set address to 0.0.0.x (x is non zero) for using hostname
        *p++ = 0;
        for( const char c : hostname ) *p++ = c;
        *p++ = 0;
        break;

    default:
        // XXX SOCK5未対応
        return ret;
    }

    if( write( msgbuf, p - msgbuf ) <= 0 ) {
        return ret;
    }

    if( read( msgbuf, 8 ) <= 0 || msgbuf[0] != 0 || msgbuf[1] != 0x5a ) {

        m_errmsg = "socks handshake failed";

#ifdef _DEBUG
        std::cout << "Socket::socks_handshake: socks recieve: ";
#endif
    }
    else {
        ret = true;

#ifdef _DEBUG
        std::cout << "Socket::socks_handshake: socks handshake done: ";
#endif
    }

#ifdef _DEBUG
    std::cout << std::hex << std::setfill('0')
              << std::setw(2) << static_cast<unsigned char>( msgbuf[0] ) << " "
              << std::setw(2) << static_cast<unsigned char>( msgbuf[1] ) << " port="
              << std::setw(2) << static_cast<unsigned char>( msgbuf[2] ) << " "
              << std::setw(2) << static_cast<unsigned char>( msgbuf[3] ) << " ip="
              << std::setw(2) << static_cast<unsigned char>( msgbuf[4] ) << " "
              << std::setw(2) << static_cast<unsigned char>( msgbuf[5] ) << " "
              << std::setw(2) << static_cast<unsigned char>( msgbuf[6] ) << " "
              << std::setw(2) << static_cast<unsigned char>( msgbuf[7] ) << std::dec << std::endl;
#endif
    return ret;
}


/**
 * @brief ソケットを閉じる
 */
void Socket::close()
{
#ifdef _DEBUG
    std::cout << "Socket::close" << std::endl;
#endif

    m_errmsg.clear();

    if( m_tls ) {
        tls_close();
    }

    if( soc_is_valid( m_soc ) ) {
        // writefds待ち
        // 待たないとclose()したときにfinパケットが消える？
        wait_fds( WaitFor::send );

        // 送信禁止
        shutdown( m_soc, SHUT_WR );

        wait_fds( WaitFor::send );

        // ソケットクローズ
        ::close( m_soc );

        m_soc = kInvlidSocket;
    }
}


/** @brief ソケットにメッセージを送信する
 *
 * @param[in] buf     送信するメッセージ (not null)
 * @param[in] bufsize メッセージのサイズ
 * @return 送信されたバイト数
 * @retval 0  スレッドが中断した
 * @retval -1 送信に失敗した
 */
int Socket::write( const char* buf, const std::size_t bufsize )
{
    std::size_t send_size = bufsize;

    while( send_size > 0 ) {

        ssize_t tmpsize;

        if( m_tls ) {
            tmpsize = tls_write( buf + bufsize - send_size, send_size );
            if( ! m_errmsg.empty() ) return kSocket_ERR;
        }
        else if( soc_is_valid( m_soc ) ) {
            // writefds 待ち
            if( ! wait_fds( WaitFor::send ) ) {
                m_errmsg = "send timeout";
                return kSocket_ERR;
            }

#ifdef MSG_NOSIGNAL
            tmpsize = ::send( m_soc, buf + bufsize - send_size, send_size, MSG_NOSIGNAL );
#else
            // SolarisにはMSG_NOSIGNALが無いのでSIGPIPEをIGNOREする (FreeBSD4.11Rにもなかった)
            signal( SIGPIPE, SIG_IGN ); /* シグナルを無視する */
            tmpsize = ::send( m_soc, buf + bufsize - send_size, send_size, 0 );
            signal( SIGPIPE, SIG_DFL ); /* 念のため戻す */
#endif // MSG_NOSIGNAL

            if( tmpsize == 0
                || ( tmpsize < 0 && !( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ) ) )
            {
                m_errmsg = std::string{ "send failed: errno=" } + std::strerror( errno );
                return kSocket_ERR;
            }
        }
        else {
            m_errmsg = "send failed: internal error";
            return kSocket_ERR;
        }

        if( tmpsize > 0 ) send_size -= tmpsize;

        if( m_stop.load( std::memory_order_acquire ) ) return 0;
    }

    const int ret = bufsize - send_size; // possibly narrow cast
#ifdef _DEBUG
    std::cout << "Socket::write: size=" << ret << std::endl;
#endif
    return ret;
}


/** @brief ソケットからメッセージを受信する
 *
 * @param[out] buf     メッセージを格納するバッファ (not null)
 * @param[in]  bufsize バッファのサイズ
 * @return 受信したバイト数
 * @retval -1 受信に失敗した
 */
int Socket::read( char* buf, const std::size_t bufsize )
{
    ssize_t ret = kSocket_OK;

    do {
        if( m_tls ) {
            ret = tls_read( buf, bufsize );
            if( ! m_errmsg.empty() ) return kSocket_ERR;
        }
        else if( soc_is_valid( m_soc ) ) {
            // readfds 待ち
            if( ! wait_fds( WaitFor::recv ) ) {
                m_errmsg = "receive timeout";
                return kSocket_ERR;
            }

            ret = ::recv( m_soc, buf, bufsize, 0 );
            if( ret < 0 && !( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ) ) {

                m_errmsg = std::string{ "receive failed: errno=" } + std::strerror( errno );
                return kSocket_ERR;
            }
        }
        else {
            m_errmsg = "receive failed: internal error";
            return kSocket_ERR;
        }

        if( m_stop.load( std::memory_order_acquire ) ) break;

    } while( ret < 0 );

#ifdef _DEBUG
    std::cout << "Socket::read: size=" << ret << std::endl;
#endif
    return ret; // possibly narrow cast
}


/** @brief 通信が可能になるまで待機する
 *
 * @param[in] operation 待機する操作(send or recv)
 * @retval true  operation で指定した操作が可能になった
 * @retval false select() 呼び出しが割り込まれた or エラー or スレッド停止
 */
bool Socket::wait_fds( const WaitFor operation )
{
    if( ! m_soc || ! m_async ) return true;

    int count = 0;
    while( 1 ) {

        errno = 0;

        int ret;
        fd_set fdset;
        FD_ZERO( &fdset );
        FD_SET( m_soc, &fdset );
        struct timeval tv = { 1, 0 }; // タイムアウトを1秒に設定する

        if( operation == WaitFor::recv ) ret = select( m_soc + 1, &fdset, nullptr, nullptr, &tv );
        else ret = select( m_soc + 1, nullptr, &fdset, nullptr, &tv );

        if( errno != EINTR && ret < 0 ) {
            MISC::ERRMSG( std::string{ "Socket::wait_fds: " } + std::strerror( errno ) );
            break;
        }

        if( errno != EINTR && FD_ISSET( m_soc, &fdset ) ) return true;
        if( m_stop.load( std::memory_order_acquire ) ) break;

        if( ++count >= m_tout ) break;
#ifdef _DEBUG
        std::cout << "Socket::wait_fds" << ( operation == WaitFor::recv ? "(recv)" : "(send)" )
                  << ": timeout=" << count << std::endl;
#endif
    }

    return false;
}
