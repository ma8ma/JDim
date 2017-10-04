// ライセンス: GPL2


// NSPR include files
#include <nspr/prerror.h>
#include <nspr/prinit.h>
// Private API, to turn a POSIX file descriptor into an NSPR handle.
#include <nspr/private/pprio.h>

// NSS include files
#include <nss/nss.h>
#include <nss/secmod.h>
#include <nss/ssl.h>
#include <nss/sslproto.h>
#include <nss/sslerr.h>


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

    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 32 );
    NSS_NoDB_Init( 0 );
    SSL_OptionSetDefault( SSL_NO_CACHE, PR_TRUE );
    //SSL_OptionSetDefault( SSL_ENABLE_SESSION_TICKETS, PR_TRUE );
    NSS_SetDomesticPolicy();

    // Initialize the trusted certificate store.
    constexpr const char* module_name = "library=libnssckbi.so name=\"Root Certs\"";
    SECMODModule *module = SECMOD_LoadUserModule( const_cast< char* >( module_name ), nullptr, PR_FALSE );
    if( module == nullptr || !module->loaded ) {
        std::string errmsg = "NSPR: ";
        errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
                //PR_ErrorToName( PR_GetError() );
        MISC::ERRMSG( errmsg );
        if( module ) SECMOD_DestroyModule( module );
    }
}


void JDLIB::tlslib_deinit()
{
#ifdef _DEBUG
    std::cout << "tlslib_deinit" << std::endl;
#endif

    SSL_ClearSessionCache();
    NSS_Shutdown();
    PR_Cleanup();

#ifdef _WIN32
    WSACleanup();
#endif
}


static SECStatus PR_CALLBACK cert_hook( void *arg, PRFileDesc *fd )
{
    static_cast< void >( fd );
    static_cast< void >( arg );
    return SECSuccess;
}



bool Socket::tls_handshake( const std::string& hostname, const bool verify )
{
#ifdef _DEBUG
    std::cout << "Socket::tls_handshake" << std::endl;
#endif

    bool ret = false;

    if( !SOC_ISVALID( m_soc ) ){
        m_errmsg = "invalid socket";
        return ret;
    }

    if( m_tls != nullptr ){
        m_errmsg = "duplicate fuction call";
        return ret;
    }


#ifdef SSL_LIBRARY_VERSION_TLS_1_2
    SSLVersionRange sslver = { SSL_LIBRARY_VERSION_TLS_1_0, SSL_LIBRARY_VERSION_TLS_1_2 };
#elif defined SSL_LIBRARY_VERSION_TLS_1_1
    SSLVersionRange sslver = { SSL_LIBRARY_VERSION_TLS_1_0, SSL_LIBRARY_VERSION_TLS_1_1 };
#else
    SSLVersionRange sslver = { SSL_LIBRARY_VERSION_3_0, SSL_LIBRARY_VERSION_TLS_1_0 };
#endif

    PRFileDesc *prfd;

    // setup sockets
    if( !( prfd = PR_ImportTCPSocket( m_soc ) ) ){
        m_errmsg = "Unable to convert socket: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }

    if( m_async ){
        PRSocketOptionData sock_opt;
        sock_opt.option = PR_SockOpt_Nonblocking;
        sock_opt.value.non_blocking = PR_TRUE;
        if( PR_SetSocketOption( prfd, &sock_opt ) != PR_SUCCESS ){
            m_errmsg = "Unable to non blocking connection :";
            m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
            goto EXIT_TLS_CONNECT;
        }
    }

    if( !( m_tls = SSL_ImportFD( nullptr, prfd ) ) ){
        m_errmsg = "Unable to import SSL FileDesc: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }
    prfd = nullptr;

    // setup SSL Options
    if( SSL_OptionSet( m_tls, SSL_HANDSHAKE_AS_CLIENT, PR_TRUE ) != SECSuccess ){
        m_errmsg = "Unable to setup handshake mode: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }
    if( SSL_OptionSet( m_tls, SSL_ENABLE_FDX, PR_TRUE ) != SECSuccess ){
        m_errmsg = "Unable to setup full duplex mode: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }
    if( !verify && SSL_BadCertHook( m_tls, cert_hook, nullptr ) != SECSuccess ){
        m_errmsg = "Unable to register certificate check hook: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }
    if( SSL_VersionRangeSet( m_tls, &sslver) != SECSuccess ){
        m_errmsg = "Unable to register protocol version range: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }

    // start SSL/TLS negotiation
    if( SSL_ResetHandshake( m_tls, PR_FALSE ) != SECSuccess ){
        m_errmsg = "Unable to reset secure handshake: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }

    if( !is_ipaddress( hostname.c_str() ) &&
            SSL_SetURL( m_tls, hostname.c_str() ) != SECSuccess ){
        m_errmsg = "Unable to register target host: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }

    // Make the handshake happen now
    while( SSL_ForceHandshakeWithTimeout( m_tls, PR_SecondsToInterval( m_tout ) ) != SECSuccess ){
        // PR_WOULD_BLOCK_ERROR in non-blocking mode is not an error
        if( PR_GetError() != PR_WOULD_BLOCK_ERROR ){
            m_errmsg = "Unable to secure negotiation: ";
            m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
            goto EXIT_TLS_CONNECT;
        }

        // writefds待ち
        if( ! wait_fds( true ) ){
            m_errmsg = "handshake timeout";
            goto EXIT_TLS_CONNECT;
        }
    }

#ifdef _DEBUG
    SSLChannelInfo channel;
    if( SSL_GetChannelInfo( m_tls, &channel, sizeof channel ) != SECSuccess ||
        channel.length != sizeof channel || ! channel.cipherSuite ){
        m_errmsg = "Unable to get ChannelInfo: ";
        m_errmsg += PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT );
        goto EXIT_TLS_CONNECT;
    }

    const char *tlsver;
    switch( channel.protocolVersion ){
        case SSL_LIBRARY_VERSION_2: tlsver = "SSLv2"; break;
        case SSL_LIBRARY_VERSION_3_0: tlsver = "SSLv3"; break;
        case SSL_LIBRARY_VERSION_TLS_1_0: tlsver = "TLS1.0"; break;
#ifdef SSL_LIBRARY_VERSION_TLS_1_1
        case SSL_LIBRARY_VERSION_TLS_1_1: tlsver = "TLS1.1"; break;
#endif
#ifdef SSL_LIBRARY_VERSION_TLS_1_2
        case SSL_LIBRARY_VERSION_TLS_1_2: tlsver = "TLS1.2"; break;
#endif
#ifdef SSL_LIBRARY_VERSION_TLS_1_3
        case SSL_LIBRARY_VERSION_TLS_1_3: tlsver = "TLS1.3"; break;
#endif
        default: tlsver = "SSL/TLS";
    }

    SSLCipherSuiteInfo suite;
    if( SSL_GetCipherSuiteInfo( channel.cipherSuite, &suite, sizeof suite ) == SECSuccess )
        std::cout << tlsver << " " << suite.cipherSuiteName << " connected." << std::endl;
    else std::cout << tlsver << " connected." << std::endl;
#endif

    ret = true;

EXIT_TLS_CONNECT:

    if( prfd ) PR_Close( prfd );

    return ret;
}


void Socket::tls_close()
{
    if( PR_Shutdown( m_tls, PR_SHUTDOWN_BOTH ) != PR_SUCCESS ){
        MISC::ERRMSG( std::string( "TLS shutdown failed: " )
                + PR_ErrorToString( PR_GetError(), PR_LANGUAGE_I_DEFAULT ) );
    }
    PR_Close( m_tls );
    m_soc = INVALID_SOCKET;
    m_tls = nullptr;
}


int Socket::tls_write( const char* buf, const size_t length )
{
    const ssize_t tmpsize = PR_Write( m_tls, buf, length );

    if( tmpsize < 0 ){
        PRErrorCode err = PR_GetError();
        if( err != PR_WOULD_BLOCK_ERROR ){
            m_errmsg = "PR_Write failed: ";
            m_errmsg += PR_ErrorToString( err, PR_LANGUAGE_I_DEFAULT );
        }

        // writefds 待ち
        else if( ! wait_fds( true ) ){
            m_errmsg = "send timeout";
        }
    }

    return tmpsize;
}


int Socket::tls_read( char* buf, const size_t bufsize )
{
    const ssize_t ret = PR_Read( m_tls, buf, bufsize );

    if( ret < 0 ){
        PRErrorCode err = PR_GetError();
        if( err != PR_WOULD_BLOCK_ERROR ){
            m_errmsg = "PR_Read failed: ";
            m_errmsg += PR_ErrorToString( err, PR_LANGUAGE_I_DEFAULT );
        }

        // writefds 待ち
        else if( ! wait_fds( true ) ){
            m_errmsg = "receive timeout";
        }
    }

    return ret;
}

