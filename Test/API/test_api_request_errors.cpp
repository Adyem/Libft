#include "../../API/api_http_internal.hpp"
#include "../../API/api.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../PThread/thread.hpp"

#ifndef _WIN32
# include <errno.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <atomic>
#include <cstdint>
#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

FT_TEST(test_api_request_string_host_validates_arguments,
    "api_request_string_host returns null and sets errno on invalid arguments")
{
    ft_errno = FT_ERR_SUCCESSS;
    int status_value;
    char *result_body;

    status_value = 0;
    result_body = api_request_string_host(ft_nullptr, 80, "GET", "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_SUCCESSS;
    status_value = 0;
    result_body = api_request_string_host("localhost", 80, ft_nullptr, "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_SUCCESSS;
    status_value = 0;
    result_body = api_request_string_host("localhost", 80, "GET",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_api_request_string_url_validates_arguments,
    "api_request_string_url validates url and method parameters")
{
    int status_value;
    char *result_body;

    ft_errno = FT_ERR_SUCCESSS;
    status_value = 0;
    result_body = api_request_string_url(ft_nullptr, "GET", ft_nullptr,
            ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_SUCCESSS;
    status_value = 0;
    result_body = api_request_string_url("example.com/test", "GET",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_SUCCESSS;
    status_value = 0;
    result_body = api_request_string_url("http://example.com/test",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_resolve_error_maps_known_codes,
    "api_request_set_resolve_error maps resolver codes to errno values")
{
#ifdef EAI_BADFLAGS
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_BADFLAGS);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_BAD_FLAGS, ft_errno);
#endif
#ifdef EAI_AGAIN
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_AGAIN);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_AGAIN, ft_errno);
#endif
#ifdef EAI_FAIL
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_FAIL);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_FAIL, ft_errno);
#endif
#ifdef EAI_FAMILY
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_FAMILY);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_FAMILY, ft_errno);
#endif
#ifdef EAI_ADDRFAMILY
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_ADDRFAMILY);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_FAMILY, ft_errno);
#endif
#ifdef EAI_SOCKTYPE
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_SOCKTYPE);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_SOCKTYPE, ft_errno);
#endif
#ifdef EAI_SERVICE
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_SERVICE);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_SERVICE, ft_errno);
#endif
#ifdef EAI_MEMORY
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_MEMORY);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_MEMORY, ft_errno);
#endif
#ifdef EAI_NONAME
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_NONAME);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_NO_NAME, ft_errno);
#endif
#ifdef EAI_NODATA
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_NODATA);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_NO_NAME, ft_errno);
#endif
#ifdef EAI_OVERFLOW
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_OVERFLOW);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_OVERFLOW, ft_errno);
#endif
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(12345);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_FAILED, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_resolve_error_handles_system_errno,
    "api_request_set_resolve_error handles system errno reporting")
{
#ifdef EAI_SYSTEM
#ifdef _WIN32
    FT_ASSERT_EQ(1, 1);
#else
    int previous_errno;

    previous_errno = errno;
    errno = EDOM;
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_SYSTEM);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    errno = 0;
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_resolve_error(EAI_SYSTEM);
    FT_ASSERT_EQ(FT_ERR_SOCKET_RESOLVE_FAIL, ft_errno);
    errno = previous_errno;
#endif
#endif
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_prefers_openssl_queue,
    "api_request_set_ssl_error prioritizes library errors over session state")
{
    unsigned long expected_error;

    ERR_clear_error();
    ERR_put_error(ERR_LIB_SSL, 0, SSL_R_UNSUPPORTED_PROTOCOL, __FILE__, __LINE__);
    expected_error = ERR_peek_error();
    FT_ASSERT(expected_error != 0);
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_ssl_error(ft_nullptr, 0);
    FT_ASSERT_EQ(static_cast<int>(expected_error), ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_handles_missing_session,
    "api_request_set_ssl_error falls back to FT_ERR_IO without a session")
{
    ERR_clear_error();
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_ssl_error(ft_nullptr, 0);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_reports_errno_for_syscall,
    "api_request_set_ssl_error propagates errno when SSL_ERROR_SYSCALL occurs")
{
#ifdef _WIN32
    FT_ASSERT_EQ(1, 1);
#else
    SSL_CTX *context;
    SSL *session;
    int previous_errno;

    if (!OPENSSL_init_ssl(0, ft_nullptr))
        return (0);
    context = SSL_CTX_new(TLS_method());
    if (!context)
        return (0);
    session = SSL_new(context);
    if (!session)
    {
        SSL_CTX_free(context);
        return (0);
    }
    ERR_clear_error();
    previous_errno = errno;
    errno = EDOM;
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_ssl_error(session, -1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    errno = 0;
    ft_errno = FT_ERR_SUCCESSS;
    api_request_set_ssl_error(session, -1);
    FT_ASSERT_EQ(FT_ERR_SSL_SYSCALL_ERROR, ft_errno);
    errno = previous_errno;
    SSL_free(session);
    SSL_CTX_free(context);
#endif
    return (1);
}
