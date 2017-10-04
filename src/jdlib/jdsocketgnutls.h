// ライセンス: GPL2


using namespace JDLIB;


void JDLIB::tlslib_init()
{
#ifdef _DEBUG
    std::cout << "tlslib_init" << std::endl;
#endif

    gnutls_global_init();
}


void JDLIB::tlslib_deinit()
{
#ifdef _DEBUG
    std::cout << "tlslib_deinit" << std::endl;
#endif

    gnutls_global_deinit();
}



bool Socket::tls_handshake( const std::string& hostname, const bool verify )
{
#ifdef _DEBUG
    std::cout << "Socket::tls_handshake" << std::endl;
#endif

    bool ret = false;
    int err = 0;

    m_errmsg.clear();

    if( !SOC_ISVALID( m_soc ) ){
        m_errmsg = "invalid socket";
        return ret;
    }

    if( m_tls != nullptr ){
        m_errmsg = "duplicate fuction call";
        return ret;
    }

    m_terminated = false;

    gnutls_init( &m_tls, GNUTLS_CLIENT );

    if( ( err = gnutls_certificate_allocate_credentials( &m_cred ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "allocate cred failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }
    if( verify && ( err = gnutls_certificate_set_x509_system_trust( m_cred ) ) < 0 ){
        m_errmsg = "set trust file failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    // allow the use of private ciphersuites. Server Name Indication (SNI)
    if( !is_ipaddress( hostname.c_str() ) ){
         gnutls_server_name_set( m_tls, GNUTLS_NAME_DNS, hostname.c_str(), hostname.length() );
    }

    if( ( err = gnutls_set_default_priority( m_tls ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "set default priority failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }
    if( ( err = gnutls_credentials_set( m_tls, GNUTLS_CRD_CERTIFICATE, m_cred ) ) != GNUTLS_E_SUCCESS ){
        m_errmsg = "cred set failed: ";
        m_errmsg += gnutls_strerror( err );
        goto EXIT_TLS_CONNECT;
    }

    if( verify ) {
        gnutls_session_set_verify_cert( m_tls, hostname.c_str(), 0 );
    }

    gnutls_transport_set_int( m_tls, m_soc );
    gnutls_handshake_set_timeout( m_tls, m_tout * 1000 );

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

#ifdef _DEBUG
    std::cout << gnutls_protocol_get_name( gnutls_protocol_get_version( m_tls ) )
        << " " << gnutls_cipher_suite_get_name( gnutls_kx_get( m_tls ),
                        gnutls_cipher_get( m_tls ), gnutls_mac_get( m_tls ) )
        << std::endl;
#endif

    ret = true;

EXIT_TLS_CONNECT:

    if( ! m_errmsg.empty() && m_cred ) {
        gnutls_certificate_free_credentials( m_cred );
        m_cred = nullptr;
    }

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

    if( m_cred ){
        gnutls_certificate_free_credentials( m_cred );
        m_cred = nullptr;
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

