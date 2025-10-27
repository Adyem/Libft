#include "errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cstddef>

typedef struct s_ft_error_string
{
    int error_code;
    const char *error_message;
}   t_ft_error_string;

static const t_ft_error_string g_error_strings[] =
{
    {ER_SUCCESS, "Operation successful"},
    {FT_ERR_NO_MEMORY, "Memory allocation failed"},
    {FT_ERR_INVALID_ARGUMENT, "Invalid argument"},
    {FT_ERR_INVALID_POINTER, "Invalid pointer"},
    {FT_ERR_INVALID_HANDLE, "Invalid handle"},
    {FT_ERR_INVALID_OPERATION, "Invalid operation"},
    {FT_ERR_INVALID_STATE, "Invalid state"},
    {FT_ERR_NOT_FOUND, "Object not found"},
    {FT_ERR_ALREADY_EXISTS, "Resource already exists"},
    {FT_ERR_OUT_OF_RANGE, "Index out of range"},
    {FT_ERR_EMPTY, "Container is empty"},
    {FT_ERR_FULL, "Container is full"},
    {FT_ERR_IO, "I/O error"},
    {FT_ERR_OVERLAP, "Memory regions overlap"},
    {FT_ERR_TERMINATED, "Operation terminated"},
    {FT_ERR_INTERNAL, "Internal error"},
    {FT_ERR_CONFIGURATION, "Invalid configuration"},
    {FT_ERR_UNSUPPORTED_TYPE, "Unsupported type"},
    {FT_ERR_ALREADY_INITIALIZED, "Already initialized"},
    {FT_ERR_INITIALIZATION_FAILED, "Initialization failed"},
    {FT_ERR_END_OF_FILE, "End of file reached"},
    {FT_ERR_DIVIDE_BY_ZERO, "Division by zero"},
    {FT_ERR_BROKEN_PROMISE, "Associated promise was destroyed"},
    {FT_ERR_SOCKET_CREATION_FAILED, "Failed to create socket"},
    {FT_ERR_SOCKET_BIND_FAILED, "Failed to bind socket"},
    {FT_ERR_SOCKET_LISTEN_FAILED, "Failed to listen on socket"},
    {FT_ERR_SOCKET_CONNECT_FAILED, "Failed to connect to server"},
    {FT_ERR_INVALID_IP_FORMAT, "Invalid IP address format"},
    {FT_ERR_SOCKET_ACCEPT_FAILED, "Failed to accept connection"},
    {FT_ERR_SOCKET_SEND_FAILED, "Failed to send data through socket"},
    {FT_ERR_SOCKET_RECEIVE_FAILED, "Failed to receive data from socket"},
    {FT_ERR_SOCKET_CLOSE_FAILED, "Failed to close socket"},
    {FT_ERR_SOCKET_JOIN_GROUP_FAILED, "Socket: Join multicast group failed"},
    {FT_ERR_SOCKET_RESOLVE_FAILED, "Failed to resolve host name"},
    {FT_ERR_SOCKET_RESOLVE_BAD_FLAGS, "Invalid flags provided to resolver"},
    {FT_ERR_SOCKET_RESOLVE_AGAIN, "Temporary failure in name resolution"},
    {FT_ERR_SOCKET_RESOLVE_FAIL, "Non-recoverable failure in name resolution"},
    {FT_ERR_SOCKET_RESOLVE_FAMILY, "Address family not supported for name"},
    {FT_ERR_SOCKET_RESOLVE_SOCKTYPE, "Socket type not supported for requested address"},
    {FT_ERR_SOCKET_RESOLVE_SERVICE, "Service not supported for socket type"},
    {FT_ERR_SOCKET_RESOLVE_MEMORY, "Memory allocation failure during name resolution"},
    {FT_ERR_SOCKET_RESOLVE_NO_NAME, "Name or service not known"},
    {FT_ERR_SOCKET_RESOLVE_OVERFLOW, "Resolver result too large"},
    {FT_ERR_GAME_GENERAL_ERROR, "General Ingame Error"},
    {FT_ERR_GAME_INVALID_MOVE, "Invalid Move"},
    {FT_ERR_MUTEX_NOT_OWNER, "Thread is not the owner of the mutex"},
    {FT_ERR_MUTEX_ALREADY_LOCKED, "Mutex already locked"},
    {FT_ERR_SSL_WANT_READ, "SSL wants to read"},
    {FT_ERR_SSL_WANT_WRITE, "SSL wants to write"},
    {FT_ERR_SSL_ZERO_RETURN, "SSL connection closed"},
    {FT_ERR_BITSET_NO_MEMORY, "Bitset memory allocation failed"},
    {FT_ERR_PRIORITY_QUEUE_EMPTY, "Priority queue is empty"},
    {FT_ERR_PRIORITY_QUEUE_NO_MEMORY, "Priority queue memory allocation failed"},
    {FT_ERR_SSL_SYSCALL_ERROR, "SSL system call error"},
    {FT_ERR_HTTP_PROTOCOL_MISMATCH, "HTTP protocol mismatch"},
    {FT_ERR_API_CIRCUIT_OPEN, "API circuit breaker is open"}
};

static const char *ft_find_custom_error(int error_code)
{
    size_t error_index;
    size_t error_count;

    error_count = sizeof(g_error_strings) / sizeof(g_error_strings[0]);
    error_index = 0;
    while (error_index < error_count)
    {
        if (g_error_strings[error_index].error_code == error_code)
            return (g_error_strings[error_index].error_message);
        error_index++;
    }
    return (NULL);
}

const char* ft_strerror(int error_code)
{
    const char *custom_message;
    const char *system_message;

    custom_message = ft_find_custom_error(error_code);
    if (custom_message != NULL)
        return (custom_message);
    system_message = cmp_system_strerror(error_code);
    if (system_message != NULL)
        return (system_message);
    return ("Unrecognized error code");
}
