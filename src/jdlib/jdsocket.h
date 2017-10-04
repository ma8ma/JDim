// ライセンス: GPL2


#ifndef _JDSOCKET_H
#define _JDSOCKET_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(USE_GNUTLS)
#  include <gnutls/gnutls.h>
#elif defined(USE_OPENSSL)
#  include <openssl/ssl.h>
// gdkmm/device.h で定義される set_key マクロと衝突する
#  ifdef set_key
#    undef set_key
#  endif
#else
#error "No TLS library specified."
#endif

#include <string>

namespace JDLIB
{
    enum{
        PROXY_NONE = 0,
        PROXY_HTTP,
        PROXY_SOCKS4,
        PROXY_SOCKS4A,
        PROXY_SOCKS5,
        PROXY_SOCKS5H
    };

    class Socket
    {
        std::string m_errmsg;
        std::string m_tls_proto;
        volatile bool *m_stop;
        bool m_async;
        long m_tout{};
        int m_soc;

#if defined(USE_GNUTLS)
        gnutls_session_t m_tls{};
        gnutls_certificate_credentials_t m_cred{};
        bool m_terminated{};
#elif defined(USE_OPENSSL)
        SSL_CTX* m_ctx{};
        SSL* m_tls{};
#endif

    public:

        explicit Socket( bool *const stop, const bool async = true );
        ~Socket();

        std::string& get_errmsg(){ return m_errmsg; }
        void set_timeout( const long timeout ){ m_tout = timeout; }

        bool connect( const std::string& host, const std::string& port, const bool use_ipv6 );
        void close();

        int write( const char* buf, const size_t bufsize );
        int read( char* buf, const size_t bufsize );

        bool tls_handshake( const std::string& hostname, const bool verify );
        bool socks_handshake( const std::string& hostname, const std::string& port, const int protocol );

    private:
        int tls_write( const char* buf, const size_t length );
        int tls_read( char* buf, const size_t bufsize );
        void tls_close();

        bool wait_fds( const bool fdrecv );
    };


    void tlslib_init();
    void tlslib_deinit();
}

#endif

