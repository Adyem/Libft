#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_strerror_covers_documented_codes, "ft_strerror returns descriptions for documented error codes")
{
    static const int error_code_table[] = {
        ER_SUCCESS,
        FT_ERR_NO_MEMORY,
        FT_ERR_INVALID_ARGUMENT,
        FT_ERR_INVALID_POINTER,
        FT_ERR_INVALID_HANDLE,
        FT_ERR_INVALID_OPERATION,
        FT_ERR_INVALID_STATE,
        FT_ERR_NOT_FOUND,
        FT_ERR_ALREADY_EXISTS,
        FT_ERR_OUT_OF_RANGE,
        FT_ERR_EMPTY,
        FT_ERR_FULL,
        FT_ERR_IO,
        FT_ERR_OVERLAP,
        FT_ERR_TERMINATED,
        FT_ERR_INTERNAL,
        FT_ERR_CONFIGURATION,
        FT_ERR_UNSUPPORTED_TYPE,
        FT_ERR_ALREADY_INITIALIZED,
        FT_ERR_INITIALIZATION_FAILED,
        FT_ERR_END_OF_FILE,
        FT_ERR_DIVIDE_BY_ZERO,
        FT_ERR_BROKEN_PROMISE,
        FT_ERR_SOCKET_CREATION_FAILED,
        FT_ERR_SOCKET_BIND_FAILED,
        FT_ERR_SOCKET_LISTEN_FAILED,
        FT_ERR_SOCKET_CONNECT_FAILED,
        FT_ERR_INVALID_IP_FORMAT,
        FT_ERR_SOCKET_ACCEPT_FAILED,
        FT_ERR_SOCKET_SEND_FAILED,
        FT_ERR_SOCKET_RECEIVE_FAILED,
        FT_ERR_SOCKET_CLOSE_FAILED,
        FT_ERR_SOCKET_JOIN_GROUP_FAILED,
        FT_ERR_SOCKET_RESOLVE_FAILED,
        FT_ERR_SOCKET_RESOLVE_BAD_FLAGS,
        FT_ERR_SOCKET_RESOLVE_AGAIN,
        FT_ERR_SOCKET_RESOLVE_FAIL,
        FT_ERR_SOCKET_RESOLVE_FAMILY,
        FT_ERR_SOCKET_RESOLVE_SOCKTYPE,
        FT_ERR_SOCKET_RESOLVE_SERVICE,
        FT_ERR_SOCKET_RESOLVE_MEMORY,
        FT_ERR_SOCKET_RESOLVE_NO_NAME,
        FT_ERR_SOCKET_RESOLVE_OVERFLOW,
        FT_ERR_GAME_GENERAL_ERROR,
        FT_ERR_GAME_INVALID_MOVE,
        FT_ERR_MUTEX_NOT_OWNER,
        FT_ERR_MUTEX_ALREADY_LOCKED,
        FT_ERR_SSL_WANT_READ,
        FT_ERR_SSL_WANT_WRITE,
        FT_ERR_SSL_ZERO_RETURN,
        FT_ERR_BITSET_NO_MEMORY,
        FT_ERR_PRIORITY_QUEUE_EMPTY,
        FT_ERR_PRIORITY_QUEUE_NO_MEMORY,
        FT_ERR_SSL_SYSCALL_ERROR
    };
    size_t code_index;
    size_t code_count;

    code_index = 0;
    code_count = sizeof(error_code_table) / sizeof(error_code_table[0]);
    while (code_index < code_count)
    {
        const char *error_message;

        ft_errno = ER_SUCCESS;
        error_message = ft_strerror(error_code_table[code_index]);
        FT_ASSERT(error_message != ft_nullptr);
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
        code_index++;
    }
    return (1);
}
