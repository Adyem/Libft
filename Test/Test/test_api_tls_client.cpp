#define private public
#include "../../API/tls_client.hpp"
#undef private
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include <openssl/ssl.h>
#include <climits>

static bool g_mock_tls_io_enabled = false;

static const char *g_mock_read_buffers[8];
static size_t g_mock_read_sizes[8];
static size_t g_mock_read_count = 0;
static size_t g_mock_read_index = 0;
static size_t g_mock_read_offset = 0;

static void mock_tls_reset_reads()
{
    g_mock_read_count = 0;
    g_mock_read_index = 0;
    g_mock_read_offset = 0;
}

static void mock_tls_add_read(const char *data)
{
    size_t length;

    if (g_mock_read_count >= 8)
        return ;
    length = ft_strlen(data);
    g_mock_read_buffers[g_mock_read_count] = data;
    g_mock_read_sizes[g_mock_read_count] = length;
    g_mock_read_count += 1;
}

extern "C"
{
    ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len)
    {
        int write_length;
        int write_result;
        size_t copy_index;
        const char *source;

        if (len > static_cast<size_t>(INT_MAX))
        {
            ft_errno = FT_EINVAL;
            return (-1);
        }
        if (!g_mock_tls_io_enabled)
        {
            write_length = static_cast<int>(len);
            write_result = ::SSL_write(ssl, buf, write_length);
            if (write_result <= 0)
            {
                ft_errno = SSL_ERROR_SYSCALL + ERRNO_OFFSET;
                return (-1);
            }
            ft_errno = ER_SUCCESS;
            return (write_result);
        }
        source = static_cast<const char *>(buf);
        copy_index = 0;
        while (copy_index < len)
        {
            (void)source[copy_index];
            copy_index += 1;
        }
        ft_errno = ER_SUCCESS;
        return (static_cast<ssize_t>(len));
    }

    ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len)
    {
        int read_length;
        int read_result;
        char *destination;
        size_t remaining;
        size_t copy_length;
        size_t copy_index;
        const char *source;

        if (len > static_cast<size_t>(INT_MAX))
        {
            ft_errno = FT_EINVAL;
            return (-1);
        }
        if (!g_mock_tls_io_enabled)
        {
            read_length = static_cast<int>(len);
            read_result = ::SSL_read(ssl, buf, read_length);
            if (read_result <= 0)
            {
                ft_errno = SSL_ERROR_SYSCALL + ERRNO_OFFSET;
                return (-1);
            }
            ft_errno = ER_SUCCESS;
            return (read_result);
        }
        (void)ssl;
        while (g_mock_read_index < g_mock_read_count)
        {
            remaining = g_mock_read_sizes[g_mock_read_index] - g_mock_read_offset;
            if (remaining != 0)
                break;
            g_mock_read_index += 1;
            g_mock_read_offset = 0;
        }
        if (g_mock_read_index >= g_mock_read_count)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        remaining = g_mock_read_sizes[g_mock_read_index] - g_mock_read_offset;
        copy_length = len;
        if (copy_length > remaining)
            copy_length = remaining;
        destination = static_cast<char *>(buf);
        source = g_mock_read_buffers[g_mock_read_index];
        copy_index = 0;
        while (copy_index < copy_length)
        {
            destination[copy_index] = source[g_mock_read_offset + copy_index];
            copy_index += 1;
        }
        g_mock_read_offset += copy_length;
        if (g_mock_read_offset >= g_mock_read_sizes[g_mock_read_index])
        {
            g_mock_read_index += 1;
            g_mock_read_offset = 0;
        }
        ft_errno = ER_SUCCESS;
        return (static_cast<ssize_t>(copy_length));
    }
}

static void prepare_mock_client(api_tls_client &client)
{
    client._ssl = reinterpret_cast<SSL *>(0x1);
    client._sock = -1;
    client._error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    client._host = "mock.local";
}

FT_TEST(test_api_tls_client_chunked_response, "api_tls_client handles chunked bodies")
{
    api_tls_client client(ft_nullptr, 0, 0);

    prepare_mock_client(client);
    mock_tls_reset_reads();
    mock_tls_add_read("HTTP/1.1 200 OK\r\nTrAnSfEr-EnCoDiNg: gzip, Chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n");
    g_mock_tls_io_enabled = true;
    int status_value;
    char *body;
    int result;

    status_value = -1;
    body = client.request("GET", "/chunked", ft_nullptr, ft_nullptr, &status_value);
    g_mock_tls_io_enabled = false;
    if (!body)
        return (0);
    result = 1;
    if (ft_strcmp(body, "Wikipedia") != 0)
        result = 0;
    if (status_value != 200)
        result = 0;
    if (client.get_error() != ER_SUCCESS)
        result = 0;
    cma_free(body);
    client._ssl = ft_nullptr;
    return (result);
}

FT_TEST(test_api_tls_client_header_case_insensitive, "api_tls_client handles case insensitive headers")
{
    api_tls_client client(ft_nullptr, 0, 0);

    prepare_mock_client(client);
    mock_tls_reset_reads();
    mock_tls_add_read("HTTP/1.1 200 OK\r\ncontent-length: 5\r\n\r\nHello");
    g_mock_tls_io_enabled = true;
    int status_value;
    char *body;
    int result;

    status_value = -1;
    body = client.request("GET", "/case", ft_nullptr, ft_nullptr, &status_value);
    g_mock_tls_io_enabled = false;
    result = 1;
    if (!body)
        result = 0;
    if (body && ft_strcmp(body, "Hello") != 0)
        result = 0;
    if (status_value != 200)
        result = 0;
    if (client.get_error() != ER_SUCCESS)
        result = 0;
    if (body)
        cma_free(body);
    client._ssl = ft_nullptr;
    return (result);
}

FT_TEST(test_api_tls_client_connection_close_response, "api_tls_client reads connection close bodies")
{
    api_tls_client client(ft_nullptr, 0, 0);

    prepare_mock_client(client);
    mock_tls_reset_reads();
    mock_tls_add_read("HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n");
    g_mock_tls_io_enabled = true;
    int status_value;
    char *body;
    int result;

    status_value = -1;
    body = client.request("GET", "/close", ft_nullptr, ft_nullptr, &status_value);
    g_mock_tls_io_enabled = false;
    result = 1;
    if (!body)
        result = 0;
    if (body && body[0] != '\0')
        result = 0;
    if (status_value != 204)
        result = 0;
    if (client.get_error() != ER_SUCCESS)
        result = 0;
    if (body)
        cma_free(body);
    client._ssl = ft_nullptr;
    return (result);
}
