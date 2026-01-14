#ifndef ERRNO_HPP
# define ERRNO_HPP

#define ERRNO_OFFSET 2000

#ifndef FT_TYPES_HPP
# define FT_TYPES_HPP

typedef unsigned long long ft_size_t;

#endif

int ft_map_system_error(int error_code);
int ft_set_errno_from_system_error(int error_code);
void ft_global_error_stack_push(int error_code);
int ft_global_error_stack_pop_last(void);
int ft_global_error_stack_pop_newest(void);
void ft_global_error_stack_pop_all(void);
int ft_global_error_stack_error_at(ft_size_t index);
int ft_global_error_stack_last_error(void);
const char *ft_global_error_stack_error_str_at(ft_size_t index);
const char *ft_global_error_stack_last_error_str(void);

enum PTErrorCode
{
    FT_ERR_SUCCESSS = 0,
    FT_ERR_NO_MEMORY = 1,
    FT_ERR_FILE_OPEN_FAILED = 2,
    FT_ERR_INVALID_ARGUMENT = 3,
    FT_ERR_INVALID_POINTER = 4,
    FT_ERR_INVALID_HANDLE = 5,
    FT_ERR_INVALID_OPERATION = 6,
    FT_ERR_INVALID_STATE = 7,
    FT_ERR_NOT_FOUND = 8,
    FT_ERR_ALREADY_EXISTS = 9,
    FT_ERR_OUT_OF_RANGE = 10,
    FT_ERR_EMPTY = 11,
    FT_ERR_FULL = 12,
    FT_ERR_IO = 13,
    FT_ERR_SYSTEM = 14,
    FT_ERR_OVERLAP = 15,
    FT_ERR_TERMINATED = 16,
    FT_ERR_INTERNAL = 17,
    FT_ERR_CONFIGURATION = 18,
    FT_ERR_UNSUPPORTED_TYPE = 19,
    FT_ERR_ALREADY_INITIALIZED = 20,
    FT_ERR_INITIALIZATION_FAILED = 21,
    FT_ERR_END_OF_FILE = 22,
    FT_ERR_DIVIDE_BY_ZERO = 23,
    FT_ERR_BROKEN_PROMISE = 24,
    FT_ERR_SOCKET_CREATION_FAILED = 25,
    FT_ERR_SOCKET_BIND_FAILED = 26,
    FT_ERR_SOCKET_LISTEN_FAILED = 27,
    FT_ERR_SOCKET_CONNECT_FAILED = 28,
    FT_ERR_NETWORK_CONNECT_FAILED = 29,
    FT_ERR_INVALID_IP_FORMAT = 30,
    FT_ERR_SOCKET_ACCEPT_FAILED = 31,
    FT_ERR_SOCKET_SEND_FAILED = 32,
    FT_ERR_SOCKET_RECEIVE_FAILED = 33,
    FT_ERR_SOCKET_CLOSE_FAILED = 34,
    FT_ERR_SOCKET_JOIN_GROUP_FAILED = 35,
    FT_ERR_SOCKET_RESOLVE_FAILED = 36,
    FT_ERR_SOCKET_RESOLVE_BAD_FLAGS = 37,
    FT_ERR_SOCKET_RESOLVE_AGAIN = 38,
    FT_ERR_SOCKET_RESOLVE_FAIL = 39,
    FT_ERR_SOCKET_RESOLVE_FAMILY = 40,
    FT_ERR_SOCKET_RESOLVE_SOCKTYPE = 41,
    FT_ERR_SOCKET_RESOLVE_SERVICE = 42,
    FT_ERR_SOCKET_RESOLVE_MEMORY = 43,
    FT_ERR_SOCKET_RESOLVE_NO_NAME = 44,
    FT_ERR_SOCKET_RESOLVE_OVERFLOW = 45,
    FT_ERR_GAME_GENERAL_ERROR = 46,
    FT_ERR_GAME_INVALID_MOVE = 47,
    FT_ERR_THREAD_BUSY = 48,
    FT_ERR_MUTEX_NOT_OWNER = 49,
    FT_ERR_MUTEX_ALREADY_LOCKED = 50,
    FT_ERR_SSL_WANT_READ = 51,
    FT_ERR_SSL_WANT_WRITE = 52,
    FT_ERR_SSL_ZERO_RETURN = 53,
    FT_ERR_BITSET_NO_MEMORY = 54,
    FT_ERR_PRIORITY_QUEUE_EMPTY = 55,
    FT_ERR_PRIORITY_QUEUE_NO_MEMORY = 56,
    FT_ERR_CRYPTO_INVALID_PADDING = 57,
    FT_ERR_DATABASE_UNAVAILABLE = 58,
    FT_ERR_TIMEOUT = 59,
    FT_ERR_SYS_NO_MEMORY = 60,
    FT_ERR_SYS_INVALID_STATE = 61,
    FT_ERR_SYS_MUTEX_LOCK_FAILED = 62,
    FT_ERR_SYS_MUTEX_ALREADY_LOCKED = 63,
    FT_ERR_SYS_MUTEX_NOT_OWNER = 64,
    FT_ERR_SYS_MUTEX_UNLOCK_FAILED = 65,
    FT_ERR_SYS_INTERNAL = 66,
    FT_ERR_SSL_SYSCALL_ERROR = 2005,
    FT_ERR_HTTP_PROTOCOL_MISMATCH = 2006,
    FT_ERR_API_CIRCUIT_OPEN = 2007,
};

const char* ft_strerror(int error_code);
void        ft_perror(const char *error_msg);
void            ft_exit(const char *error_msg, int exit_code);

#endif
