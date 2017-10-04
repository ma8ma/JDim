// ライセンス: GPL2

#include <openssl/err.h>
#include <openssl/conf.h>

using namespace JDLIB;

void JDLIB::tlslib_init()
{
#ifdef _DEBUG
    std::cout << "tlslib_init" << std::endl;
#endif

#ifdef _WIN32
    WSADATA wsaData;
    if ( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 ){
        MISC::ERRMSG( "could not startup winsock2" );
    }
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100003L
    SSL_library_init();
#else
    OPENSSL_init_ssl( 0, nullptr );
    OPENSSL_no_config();
    OpenSSL_add_ssl_algorithms();
#endif
    SSL_load_error_strings();
}


void JDLIB::tlslib_deinit()
{
#ifdef _DEBUG
    std::cout << "tlslib_deinit" << std::endl;
#endif

    EVP_cleanup();
    ERR_free_strings();

#ifdef _WIN32
    WSACleanup();
#endif
}


inline const char* ssllib_errstr()
{
    return ERR_reason_error_string( ERR_get_error() );
}

static std::string tls_get_errstr( const int error ){
    std::string str_err;

    switch( error ){
    case SSL_ERROR_NONE: str_err = "no error"; break;
    case SSL_ERROR_SSL: str_err = ssllib_errstr(); break;
    case SSL_ERROR_SYSCALL: str_err = "syscall errno=" + std::to_string( errno ); break;
    case SSL_ERROR_ZERO_RETURN: str_err = "zero return"; break;
    case SSL_ERROR_WANT_CONNECT: str_err = "want connect"; break;
    case SSL_ERROR_WANT_ACCEPT: str_err = "want accept"; break;
    case SSL_ERROR_WANT_X509_LOOKUP: str_err = "want x509 lookup"; break;
    }

    return str_err;
}


bool Socket::tls_handshake( const std::string& hostname, const bool verify )
{
#ifdef _DEBUG
    std::cout << "Socket::tls_handshake" << std::endl;
#endif

    bool ret = false;
    int err = 0;

    if( !SOC_ISVALID( m_soc ) ){
        m_errmsg = "invalid socket";
        goto EXIT_TLS_CONNECT;
    }

    if( m_tls != nullptr ){
        m_errmsg = "duplicate fuction call";
        goto EXIT_TLS_CONNECT;
    }


    if( !( m_ctx = SSL_CTX_new( SSLv23_client_method() ) ) ){
        m_errmsg = "SSL_CTX_new failed: ";
        m_errmsg += ssllib_errstr();
        goto EXIT_TLS_CONNECT;
    }

    //SSL_CTX_set_options( m_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 );

    if( verify ){
        // load the trusted client CA certificate into context
        if( SSL_CTX_load_verify_locations( m_ctx, cafile().c_str(), nullptr ) != 1 ){
            m_errmsg = "load_verify_location failed: ";
            m_errmsg += ssllib_errstr();
            goto EXIT_TLS_CONNECT;
        }

        SSL_CTX_set_verify( m_ctx, SSL_VERIFY_PEER, nullptr );
        SSL_CTX_set_verify_depth( m_ctx, 4 );
    }

    if( !m_tout ) SSL_CTX_set_timeout( m_ctx, m_tout );

    if( !( m_tls = SSL_new( m_ctx ) ) ){
        m_errmsg = "SSL_new failed: ";
        m_errmsg += ssllib_errstr();
        goto EXIT_TLS_CONNECT;
    }

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
    if( !is_ipaddress( hostname.c_str() ) &&
            ( err = SSL_set_tlsext_host_name( m_tls, hostname.c_str() ) ) != 1 ){
        m_errmsg = "set_tlsext_host_name failed: ";
        m_errmsg += ssllib_errstr();
        goto EXIT_TLS_CONNECT;
    }
#endif

    if( !( err = SSL_set_fd( m_tls, m_soc ) ) ){
        m_errmsg = "set_fd failed: ";
        m_errmsg += ssllib_errstr();
        goto EXIT_TLS_CONNECT;
    }

    SSL_set_connect_state( m_tls );

    while( ( err = SSL_do_handshake( m_tls ) ) != 1 ){
        bool want_read;

        err = SSL_get_error( m_tls, err );
        if( err == SSL_ERROR_WANT_READ ) want_read = true;
        else if( err == SSL_ERROR_WANT_WRITE ) want_read = false;
        else{
            m_errmsg = "SSL_do_handshake failed: ";
            m_errmsg += tls_get_errstr( err );
            goto EXIT_TLS_CONNECT;
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ){
            m_errmsg = "SSL_do_handshake timeout";
            goto EXIT_TLS_CONNECT;
        }
    }

#ifdef _DEBUG
    std::cout << SSL_get_version( m_tls ) << " (OpenSSL)" << std::endl;;
#endif

    ret = true;

EXIT_TLS_CONNECT:

    return ret;
}


void Socket::tls_close()
{
    int ret;

    while( ( ret = SSL_shutdown( m_tls ) ) != 1 ){
        bool want_read = true;

        if( ret != 0 ){
            ret = SSL_get_error( m_tls, ret );
            if( ret == 0 || ret == SSL_ERROR_WANT_READ ) want_read = true;
            else if( ret == SSL_ERROR_WANT_WRITE ) want_read = false;
            else{
                if( ret != SSL_ERROR_SYSCALL || errno != 0 ){
                    MISC::ERRMSG( "SSL_shutdown failed: " + tls_get_errstr( ret ) );
                }
                break;
            }
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ){
            MISC::ERRMSG( "SSL_shutdown timeout" );
            break;
        }
    }
    SSL_free( m_tls );
    SSL_CTX_free( m_ctx );

    m_tls = nullptr;
}


int Socket::tls_write( const char* buf, const size_t length )
{
    const ssize_t tmpsize = SSL_write( m_tls, buf, length );

    if( tmpsize < 0 ){
        bool want_read;

        const int ret = SSL_get_error( m_tls, tmpsize );
        if( ret == SSL_ERROR_WANT_READ ) want_read = true;
        else if( ret == SSL_ERROR_WANT_WRITE ) want_read = false;
        else{
            m_errmsg = "SSL_write failed: " + tls_get_errstr( ret );
            return tmpsize;
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ){
            m_errmsg = "send timeout";
        }
    }

    return tmpsize;
}


int Socket::tls_read( char* buf, const size_t bufsize )
{
    const ssize_t tmpsize = SSL_read( m_tls, buf, bufsize );

    if( tmpsize < 0 ){
        bool want_read;

        const int ret = SSL_get_error( m_tls, tmpsize );
        if( ret == SSL_ERROR_WANT_READ ) want_read = true;
        else if( ret == SSL_ERROR_WANT_WRITE ) want_read = false;
        else{
            m_errmsg = "SSL_read failed: " + tls_get_errstr( ret );
            return tmpsize;
        }

        // writefds 待ち
        if( ! wait_fds( want_read ) ){
            m_errmsg = "receive timeout";
        }
    }

    return tmpsize;
}

