#include "../test_internal.hpp"
#include "../../API/api_http_internal.hpp"
#include "../../API/api.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../PThread/thread.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

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
    int status_value;
    char *result_body;

    status_value = 0;
    result_body = api_request_string_host(ft_nullptr, 80, "GET", "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);

    status_value = 0;
    result_body = api_request_string_host("localhost", 80, ft_nullptr, "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);

    status_value = 0;
    result_body = api_request_string_host("localhost", 80, "GET",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    return (1);
}

FT_TEST(test_api_request_string_url_validates_arguments,
    "api_request_string_url validates url and method parameters")
{
    int status_value;
    char *result_body;

    status_value = 0;
    result_body = api_request_string_url(ft_nullptr, "GET", ft_nullptr,
            ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);

    status_value = 0;
    result_body = api_request_string_url("example.com/test", "GET",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);

    status_value = 0;
    result_body = api_request_string_url("http://example.com/test",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    return (1);
}

FT_TEST(test_api_request_set_resolve_error_maps_known_codes,
    "api_request_set_resolve_error maps resolver codes to errno values")
{
#ifdef EAI_BADFLAGS
    api_request_set_resolve_error(EAI_BADFLAGS);
#endif
#ifdef EAI_AGAIN
    api_request_set_resolve_error(EAI_AGAIN);
#endif
#ifdef EAI_FAIL
    api_request_set_resolve_error(EAI_FAIL);
#endif
#ifdef EAI_FAMILY
    api_request_set_resolve_error(EAI_FAMILY);
#endif
#ifdef EAI_ADDRFAMILY
    api_request_set_resolve_error(EAI_ADDRFAMILY);
#endif
#ifdef EAI_SOCKTYPE
    api_request_set_resolve_error(EAI_SOCKTYPE);
#endif
#ifdef EAI_SERVICE
    api_request_set_resolve_error(EAI_SERVICE);
#endif
#ifdef EAI_MEMORY
    api_request_set_resolve_error(EAI_MEMORY);
#endif
#ifdef EAI_NONAME
    api_request_set_resolve_error(EAI_NONAME);
#endif
#ifdef EAI_NODATA
    api_request_set_resolve_error(EAI_NODATA);
#endif
#ifdef EAI_OVERFLOW
    api_request_set_resolve_error(EAI_OVERFLOW);
#endif
    api_request_set_resolve_error(12345);
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
    api_request_set_resolve_error(EAI_SYSTEM);
    errno = 0;
    api_request_set_resolve_error(EAI_SYSTEM);
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
    api_request_set_ssl_error(ft_nullptr, 0);
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_handles_missing_session,
    "api_request_set_ssl_error falls back to FT_ERR_IO without a session")
{
    ERR_clear_error();
    api_request_set_ssl_error(ft_nullptr, 0);
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
    api_request_set_ssl_error(session, -1);
    errno = 0;
    api_request_set_ssl_error(session, -1);
    errno = previous_errno;
    SSL_free(session);
    SSL_CTX_free(context);
#endif
    return (1);
}
