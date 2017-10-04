// ライセンス: GPL2


#include <gnutls/x509.h>

#if GNUTLS_VERSION_NUMBER < 0x020000
#define gnutls_certificate_credentials_t gnutls_certificate_credentials
#endif // GNUTLS_VERSION_NUMBER >= 0x020000

#if GNUTLS_VERSION_NUMBER < 0x029900
#define GNUTLS_E_PREMATURE_TERMINATION GNUTLS_E_UNEXPECTED_PACKET_LENGTH
#endif


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

    gnutls_global_init();
}


void JDLIB::tlslib_deinit()
{
#ifdef _DEBUG
    std::cout << "tlslib_deinit" << std::endl;
#endif

    gnutls_global_deinit();

#ifdef _WIN32
    WSACleanup();
#endif
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
        return ret;
    }

    if( m_tls != nullptr ){
        m_errmsg = "duplicate fuction call";
        return ret;
    }

    m_terminated = false;
    gnutls_certificate_credentials_t cred = 0;

    gnutls_init( &m_tls, GNUTLS_CLIENT );

#if GNUTLS_VERSION_NUMBER >= 0x020108
    // gnutls >= 2.1.7 (unreleased)
    const char* const priority ="NORMAL:%COMPAT";
    if( ( err = gnutls_priority_set_direct( m_tls, priority, nullptr ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "priority_set failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }
#else // GNUTLS_VERSION_NUMBER >= 0x020108
    static const int priority_prot[] = { GNUTLS_SSL3, 0 };
    // DEPRECATED (gnutls >= 2.1.4 gnutls =< 2.1.6)
    // UNDEPRECATED (gnutls >= 2.1.7)
    gnutls_set_default_priority( m_tls );
    // _GNUTLS_GCC_ATTR_DEPRECATE (gnutls >= 2.12.0)
    gnutls_protocol_set_priority( m_tls, priority_prot );
#endif // GNUTLS_VERSION_NUMBER >= 0x020108

    // allow the use of private ciphersuites. Server Name Indication (SNI)
    if( !is_ipaddress( hostname.c_str() ) ){
         gnutls_server_name_set( m_tls, GNUTLS_NAME_DNS, hostname.c_str(), hostname.length() );
    }

    if( ( err = gnutls_certificate_allocate_credentials( &cred ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "allocate cred failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    if( verify && ( err = gnutls_certificate_set_x509_trust_file( cred, cafile().c_str(), GNUTLS_X509_FMT_PEM ) ) < 0 ){
        m_errmsg = "set trust file failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    if( ( err = gnutls_credentials_set( m_tls, GNUTLS_CRD_CERTIFICATE, cred ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "cred set failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

#if GNUTLS_VERSION_NUMBER >= 0x030109
    gnutls_transport_set_int( m_tls, m_soc );
#else
    gnutls_transport_set_ptr( m_tls, (gnutls_transport_ptr_t)m_soc );
#endif

#if GNUTLS_VERSION_NUMBER >= 0x030100
    gnutls_handshake_set_timeout( m_tls, m_tout * 1000 );
#endif

    while ( ( err = gnutls_handshake( m_tls ) ) != GNUTLS_E_SUCCESS )
    {
        if ( gnutls_error_is_fatal( err ) != 0 ){
            m_errmsg = "handshake failed: ";
            m_errmsg += gnutls_strerror( err );
            goto EXIT_TLS_CONNECT;
        }

        // writefds 待ち
        if( ! wait_fds( true ) ){
            m_errmsg = "handshake timeout";
            goto EXIT_TLS_CONNECT;
        }
    }

    if( verify ){
        unsigned int status;
        gnutls_x509_crt_t cert;
        const gnutls_datum_t* cert_list;
        unsigned int cert_list_size;

        // try to verify the peer's certificate
        if( ( err = gnutls_certificate_verify_peers2( m_tls, &status ) ) < 0 ){
            m_errmsg = "certificate verify failed: ";
            m_errmsg += gnutls_strerror( err );
            goto EXIT_TLS_CONNECT;
        }
        else if( status & GNUTLS_CERT_INVALID ){
            m_errmsg = "certificate is not trusted";
            goto EXIT_TLS_CONNECT;
        }
        else if( status & GNUTLS_CERT_SIGNER_NOT_FOUND ){
            m_errmsg = "certificate hasn't got a known issuer";
            goto EXIT_TLS_CONNECT;
        }
        else if( status & GNUTLS_CERT_REVOKED ){
            m_errmsg = "certificate has been revoked";
            goto EXIT_TLS_CONNECT;
        }
#if GNUTLS_VERSION_NUMBER >= 0x020800
        else if( status & GNUTLS_CERT_NOT_ACTIVATED ){
            m_errmsg = "certificate not activated yet";
            goto EXIT_TLS_CONNECT;
        }
        else if( status & GNUTLS_CERT_EXPIRED ){
            m_errmsg = "certificate has expired";
            goto EXIT_TLS_CONNECT;
        }
#endif

        // initialize an X.509 certificate structure.
        if( ( err = gnutls_x509_crt_init( &cert ) ) < GNUTLS_E_SUCCESS ){
            m_errmsg = "cert init failed";
            m_errmsg += gnutls_strerror( err );
            goto EXIT_TLS_CONNECT;
        }

        if( ( cert_list = gnutls_certificate_get_peers( m_tls, &cert_list_size ) ) == nullptr ){
            m_errmsg = "No cert was found";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }

        // XXX We only check the first certificate in the given chain.
        if( ( err = gnutls_x509_crt_import( cert, &cert_list[0], GNUTLS_X509_FMT_DER ) ) < GNUTLS_E_SUCCESS ){
            m_errmsg = "error parsing certificate: ";
            m_errmsg += gnutls_strerror( err );
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }

        // check if the given certificate's subject matches the given hostname.
        if( ! gnutls_x509_crt_check_hostname( cert, hostname.c_str() ) ){
            m_errmsg = "the cert's owner does not match target hostname '";
            m_errmsg += hostname + "'";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }

#if GNUTLS_VERSION_NUMBER < 0x020800
        // Check for time-based validity
        const time_t expire = gnutls_x509_crt_get_expiration_time( cert );

        if( expire == static_cast< time_t >( -1 ) ){
            m_errmsg = "cert expiration date verify failed";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }
        else if( expire < time( nullptr ) ){
            m_errmsg = "cert expiration date has passed";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }

        const time_t activate = gnutls_x509_crt_get_activation_time( cert );

        if( activate == static_cast< time_t >( -1 ) ){
            m_errmsg = "cert activation date verify failed";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }
        else if( activate > time( nullptr ) ){
            m_errmsg = "cert not activated yet";
            gnutls_x509_crt_deinit( cert );
            goto EXIT_TLS_CONNECT;
        }
#endif

        gnutls_x509_crt_deinit( cert );
    }


#ifdef _DEBUG
    std::cout << gnutls_protocol_get_name( gnutls_protocol_get_version( m_tls ) )
        << " " << gnutls_cipher_suite_get_name( gnutls_kx_get( m_tls ),
                        gnutls_cipher_get( m_tls ), gnutls_mac_get( m_tls ) )
        << std::endl;
#endif

    ret = true;

EXIT_TLS_CONNECT:

    if( cred ) gnutls_certificate_free_credentials( cred );

    return ret;
}



void Socket::tls_close()
{
    int ret;

    while( ( ret = gnutls_bye( m_tls, GNUTLS_SHUT_RDWR ) ) != GNUTLS_E_SUCCESS ){

        if( ret != GNUTLS_E_AGAIN && ret != GNUTLS_E_INTERRUPTED ){
            if( !m_terminated )
                MISC::ERRMSG( std::string("TLS shutdown failed: ") + gnutls_strerror( ret ) );
            break;
        }

        // readfds 待ち
        if( !wait_fds( false ) ){
            MISC::ERRMSG( "TLS shutdown timeout" );
            break;
        }
    }

    gnutls_deinit( m_tls );
    m_terminated = false;
    m_tls = nullptr;
}

int Socket::tls_write( const char* buf, const size_t length )
{
    const int tmpsize = gnutls_record_send( m_tls, buf, length );

    if( tmpsize < 0 ){
        if( tmpsize != GNUTLS_E_AGAIN ){
            m_errmsg = "tls_write failed: ";
            m_errmsg += gnutls_strerror( tmpsize );
        }

        // readfds 待ち
        else if( !wait_fds( false ) ){
            m_errmsg = "send timeout";
        }
    }

    return tmpsize;
}


int Socket::tls_read( char* buf, const size_t bufsize )
{
    int ret = 0;
    if( !m_terminated ){

        ret = gnutls_record_recv( m_tls, buf, bufsize );
        if( ret == GNUTLS_E_PREMATURE_TERMINATION ){
            ret = 0;
            m_terminated = true;
        }
        else if( ret < 0 ){
            if( ret != GNUTLS_E_AGAIN ){
                m_errmsg = "tls_read failed: ";
                m_errmsg += gnutls_strerror( ret );
            }

            // readfds 待ち
            else if( !wait_fds( true ) ){
                m_errmsg = "receive timeout";
            }
        }
    }

    return ret;
}

