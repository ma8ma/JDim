// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "jdsocket.h"
#include "miscmsg.h"
#include "miscutil.h"

#include "config/globalconf.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef _DEBUG
#  include <iomanip>
#endif


inline bool SOC_ISVALID( int _soc ){ return _soc >= 0; }
#define INVALID_SOCKET -1


constexpr int SOCKET_OK = 0;
constexpr int SOCKET_ERR = -1;


inline bool is_ipaddress( const char* addr )
{
    unsigned char buf[ sizeof( struct in6_addr) ];

    if( inet_aton( addr, (struct in_addr *)&buf )
            || inet_pton( AF_INET6, addr, &buf ) ){
        return true;
    }
    return false;
}


inline std::string cafile()
{
    return CONFIG::get_root_cafile();
}


#if defined(USE_GNUTLS)
#  include "jdsocketgnutls.h"
#elif defined(USE_OPENSSL)
#  include "jdsocketopenssl.h"
#endif


using namespace JDLIB;


Socket::Socket( bool *const stop, const bool async )
    : m_stop( stop )
    , m_async( async )
    , m_soc( INVALID_SOCKET )
{
    // nothing to do
}


Socket::~Socket()
{
    if( SOC_ISVALID( m_soc ) ){
        close();
    }
}


bool Socket::connect( const std::string& hostname, const std::string& port, const bool use_ipv6 )
{
#ifdef _DEBUG
    std::cout << "Socket::connect" << std::endl;
#endif

    bool ret = false;
    int optval = 0;
    socklen_t optlen = sizeof( int );

    // addrinfo 取得
    struct addrinfo hints{}, *ainf = nullptr;
    if( use_ipv6 ){
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
    std::cout << "host=" << hostname
              << " ip=" << inet_ntoa( ( ( sockaddr_in* )( ainf->ai_addr ) )->sin_addr ) << std::endl;
#endif

    // ソケット作成
    m_soc = socket( ainf->ai_family, ainf->ai_socktype, ainf->ai_protocol );
    if( ! SOC_ISVALID( m_soc ) ){
        m_errmsg = "socket failed";
        goto EXIT_CONNECT;
    }

    // ソケットを非同期に設定
    if( m_async ){
        int flags;
        flags = fcntl( m_soc, F_GETFL, 0);
        if( flags == -1 || fcntl( m_soc, F_SETFL, flags | O_NONBLOCK ) < 0 )
        {
            m_errmsg = "fcntl failed";
            goto EXIT_CONNECT;
        }
    }

    // connect peer
    err = ::connect( m_soc, ainf->ai_addr, ainf->ai_addrlen );
    if( err ){

        // ノンブロックでまだ接続中
        if ( ! m_async || errno != EINPROGRESS )
        {
            m_errmsg = "connect failed: " + hostname + ":" + port;
            goto EXIT_CONNECT;
        }
    }

    // connect待ち
    if( ! wait_fds( false ) ){
        m_errmsg = "connect timeout";
        goto EXIT_CONNECT;
    }

    // connectが成功したかチェック
    if( getsockopt( m_soc, SOL_SOCKET, SO_ERROR, (void *)&optval, &optlen ) < 0
        || optval ){
        m_errmsg = "connect(getsockopt) failed: " + hostname + ":" + port;
        goto EXIT_CONNECT;
    }

    ret = true;

EXIT_CONNECT:
    // addrinfo開放
    freeaddrinfo( ainf );

    return ret;
}



bool Socket::socks_handshake( const std::string& hostname, const std::string& port, const int protocol )
{
#ifdef _DEBUG
    std::cout << "Socket::socks_handshake" << std::endl;
#endif

    bool ret = false;

    // addrinfo 取得
    uint32_t addr = 0;
    if( protocol == PROXY_SOCKS4 /*|| protocol == PROXY_SOCKS5*/ ){
        struct addrinfo hints{}, *ainf = nullptr;
        // XXX SOCKS5対応の時にIPv6もサポートが必要
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int err = getaddrinfo( hostname.c_str(), port.c_str(), &hints, &ainf );
        if( err ) {
            m_errmsg = "getaddrinfo failed: " + hostname + ":" + port;
            return ret;
        }

        if( ainf ) {
#ifdef _DEBUG
            std::cout << "host=" << hostname
                      << " ip=" << inet_ntoa( ( ( sockaddr_in* )( ainf->ai_addr ) )->sin_addr ) << std::endl;
#endif

            addr = *(uint32_t*)&( ( sockaddr_in* )( ainf->ai_addr ) )->sin_addr;
            freeaddrinfo( ainf );
        }
    }

    // ポート番号変換
    const int num_port = std::stoi( port );
    if( num_port < 0 || num_port >= 0x10000 ){
        m_errmsg = "invlid port number: " + hostname + ":" + port;
        return ret;
    }

    char msgbuf[ 100 ];
    char *p = msgbuf;
    switch( protocol ){

    case PROXY_SOCKS4:
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

    case PROXY_SOCKS4A:
        *p++ = 4;   // SOCKS Version
        *p++ = 1;   // TCP/IP Stream
        *p++ = ( num_port >> 8 ) & 0xff;
        *p++ = num_port & 0xff;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        *p++ = 0;
        for( size_t i = 0; i <= hostname.length(); ++i )
            *p++ = *( hostname.c_str() + i );
        break;
    default:
        // XXX SOCK5未対応
        return ret;
    }

    if( write( msgbuf, p - msgbuf ) <= 0 ){
        return ret;
    }

    if( read( msgbuf, 8 ) <= 0 || msgbuf[0] != 0 || msgbuf[1] != 0x5a ){

        m_errmsg = "socks handshake failed";

#ifdef _DEBUG
        std::cout << "socks recieve: " << std::hex << std::setfill('0')
            << std::setw(2) << (unsigned char)msgbuf[0] << " "
            << std::setw(2) << (unsigned char)msgbuf[1] << " "
            << std::setw(2) << (unsigned char)msgbuf[2] << " "
            << std::setw(2) << (unsigned char)msgbuf[3] << " ip="
            << std::setw(2) << (unsigned char)msgbuf[4] << " "
            << std::setw(2) << (unsigned char)msgbuf[5] << " "
            << std::setw(2) << (unsigned char)msgbuf[6] << " "
            << std::setw(2) << (unsigned char)msgbuf[7] << std::endl;
#endif
        return ret;
    }

#ifdef _DEBUG
    std::cout << "socks handshake done: port=" << std::hex << std::setfill('0')
            << std::setw(2) << msgbuf[2] << " " << std::setw(2) << msgbuf[3]
            << " ip=" << std::setw(2) << msgbuf[4] << " " << std::setw(2) << msgbuf[5]
            << " " << std::setw(2) << msgbuf[6] << " " << std::setw(2) << msgbuf[7]
            << std::endl;
#endif
    ret = true;

    return ret;
}




void Socket::close()
{
#ifdef _DEBUG
    std::cout << "Socket::close" << std::endl;
#endif

    m_errmsg = std::string();

    if( m_tls ){

        tls_close();
    }

    if( SOC_ISVALID( m_soc ) ){

        // writefds待ち
        // 待たないとclose()したときにfinパケットが消える？
        wait_fds( false );

        // 送信禁止
        shutdown( m_soc, SHUT_WR );

        wait_fds( false );

        // ソケットクローズ
        ::close( m_soc );

        m_soc = INVALID_SOCKET;
    }
}



int Socket::write( const char* buf, const size_t bufsize )
{
#ifdef _DEBUG
    std::cout << "Socket::write  size=" << bufsize << std::endl;
#endif

    int ret = SOCKET_OK;

    size_t send_size = bufsize;
    while( send_size > 0 ){

        ssize_t tmpsize;

        if( m_tls ){
            // TLS使用
            tmpsize = tls_write( buf + bufsize - send_size, send_size );
            if( !m_errmsg.empty() ){
                ret = SOCKET_ERR;
                goto EXIT_WRITE;
            }
        }
        else if( SOC_ISVALID( m_soc ) ){

            // writefds 待ち
            if( ! wait_fds( false ) ){
                m_errmsg = "send timeout";
                ret = SOCKET_ERR;
                goto EXIT_WRITE;
            }

#ifdef MSG_NOSIGNAL
            tmpsize = send( m_soc, buf + bufsize - send_size, send_size, MSG_NOSIGNAL );
#else
            // SolarisにはMSG_NOSIGNALが無いのでSIGPIPEをIGNOREする (FreeBSD4.11Rにもなかった)
            signal( SIGPIPE , SIG_IGN ); /* シグナルを無視する */
            tmpsize = send( m_soc, buf + bufsize - send_size, send_size, 0 );
            signal(SIGPIPE,SIG_DFL); /* 念のため戻す */
#endif // MSG_NOSIGNAL

            if( tmpsize == 0
                || ( tmpsize < 0 && !( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ) ) )
            {
                if( errno == ECONNRESET ) m_errmsg = "connection reset by peer";
                else m_errmsg = "send failed: errno=" + std::to_string( errno );
                ret = SOCKET_ERR;
                goto EXIT_WRITE;
            }
        }
        else{
            m_errmsg = "send failed: internal error";
            ret = SOCKET_ERR;
            goto EXIT_WRITE;
        }

        if( tmpsize > 0 ) send_size -= tmpsize;

        if( *m_stop ) goto EXIT_WRITE;
    }

    ret = bufsize - send_size;

EXIT_WRITE:
    return ret;
}



int Socket::read( char* buf, const size_t bufsize )
{
    ssize_t ret = SOCKET_OK;

    do {
        if( m_tls ){
            // SSL
            ret = tls_read( buf, bufsize );
            if( !m_errmsg.empty() ){
                ret = SOCKET_ERR;
                goto EXIT_READ;
            }
        }
        else if( SOC_ISVALID( m_soc ) ){

            // readfds 待ち
            if( !wait_fds( true ) ){
                m_errmsg = "receive timeout";
                ret = SOCKET_ERR;
                goto EXIT_READ;
            }

            ret = recv( m_soc, buf, bufsize, 0 );
            if( ret < 0 && !( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ) )
            {
                if( errno == ECONNRESET ) m_errmsg = "connection reset by peer";
                else m_errmsg = "receive failed: errno=" + std::to_string( errno );
                ret = SOCKET_ERR;
                goto EXIT_READ;
            }
        }
        else{
            m_errmsg = "receive failed: internal error";
            ret = SOCKET_ERR;
            goto EXIT_READ;
        }

        if( *m_stop ) goto EXIT_READ;

    } while( ret < 0 );

#ifdef _DEBUG
    std::cout << "Socket::recv  size=" << ret << std::endl;
#endif

EXIT_READ:
    return ret;
}


//
// sent, recv待ち
//
bool Socket::wait_fds( const bool fdrecv )
{
    if( ! m_soc || ! m_async ) return true;

    int count = 0;
    for(;;){

        errno = 0;

        int ret;
        fd_set fdset;
        FD_ZERO( &fdset );
        FD_SET( m_soc, &fdset );
        timeval tv = { 1, 0 };

        if( fdrecv ) ret = select( m_soc + 1, &fdset, nullptr, nullptr, &tv );
        else ret = select( m_soc + 1, nullptr, &fdset, nullptr, &tv );

#ifdef _DEBUG
//        if( errno == EINTR && ret < 0 ) std::cout << "Socket::wait_fds : errno = EINTR" << std::endl;
#endif
        if( errno != EINTR && ret < 0 ){
#ifdef _DEBUG
            std::cout << "Socket::wait_fds: errno = " << errno << std::endl;
#endif
            MISC::ERRMSG( "select failed" );
            break;
        }

        if( errno != EINTR && FD_ISSET( m_soc, &fdset ) ) return true;
        if( *m_stop ) break;

        if( ++count >= m_tout ) break;
#ifdef _DEBUG
        std::cout << "Socket::wait_fds" << ( fdrecv ? "(recv)" : "(send)" ) << " timeout = " << count << std::endl;
#endif
    }

    return false;
}
