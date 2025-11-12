#include "compatebility_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>

void cmp_set_force_unsetenv_result(int result, int errno_value);
void cmp_clear_force_unsetenv_result(void);
void cmp_set_force_unsetenv_windows_errors(int last_error, int socket_error);
void cmp_set_force_putenv_result(int result, int errno_value);
void cmp_clear_force_putenv_result(void);
void cmp_set_force_putenv_windows_error(int last_error);
void cmp_set_force_cpu_count_success(unsigned int cpu_count);
void cmp_set_force_cpu_count_failure(int errno_value);
void cmp_clear_force_cpu_count_result(void);
void cmp_set_force_total_memory_success(unsigned long long memory_size);
void cmp_set_force_total_memory_failure(int errno_value);
#if defined(_WIN32) || defined(_WIN64)
void cmp_set_force_total_memory_windows_failure(unsigned long last_error);
#endif
void cmp_clear_force_total_memory_result(void);

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <windows.h>
# include <sysinfoapi.h>
#else
# include <unistd.h>
# if defined(__APPLE__) && defined(__MACH__)
#  include <sys/types.h>
#  include <sys/sysctl.h>
# endif
#endif

static int global_force_unsetenv_enabled = 0;
static int global_force_unsetenv_result = 0;
static int global_force_unsetenv_errno_value = 0;
static int global_force_putenv_enabled = 0;
static int global_force_putenv_result = 0;
static int global_force_putenv_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
static int global_force_unsetenv_last_error = 0;
static int global_force_unsetenv_socket_error = 0;
static int global_force_putenv_last_error = 0;
#endif
static int global_force_cpu_count_enabled = 0;
static int global_force_cpu_count_should_fail = 0;
static unsigned int global_force_cpu_count_value = 0;
static int global_force_cpu_count_errno_value = 0;
static int global_force_total_memory_enabled = 0;
static int global_force_total_memory_should_fail = 0;
static unsigned long long global_force_total_memory_value = 0;
static int global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
static unsigned long global_force_total_memory_last_error = 0;
#endif

#if defined(_WIN32) || defined(_WIN64)
static int cmp_translate_windows_error(int error_code)
{
    if (error_code == WSAEINTR)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAEWOULDBLOCK)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAEINPROGRESS)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAEALREADY)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAENOTSOCK)
        return (FT_ERR_INVALID_HANDLE);
    if (error_code == WSAEDESTADDRREQ)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == WSAEMSGSIZE)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == WSAEPROTOTYPE)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == WSAENOPROTOOPT)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == WSAEPROTONOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == WSAESOCKTNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == WSAEOPNOTSUPP)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == WSAEPFNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == WSAEAFNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == WSAEADDRINUSE)
        return (FT_ERR_ALREADY_EXISTS);
    if (error_code == WSAEADDRNOTAVAIL)
        return (FT_ERR_NOT_FOUND);
    if (error_code == WSAENETDOWN)
        return (FT_ERR_IO);
    if (error_code == WSAENETUNREACH)
        return (FT_ERR_IO);
    if (error_code == WSAENETRESET)
        return (FT_ERR_IO);
    if (error_code == WSAECONNABORTED)
        return (FT_ERR_IO);
    if (error_code == WSAECONNRESET)
        return (FT_ERR_IO);
    if (error_code == WSAENOBUFS)
        return (FT_ERR_NO_MEMORY);
    if (error_code == WSAEISCONN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAENOTCONN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAESHUTDOWN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAETOOMANYREFS)
        return (FT_ERR_FULL);
    if (error_code == WSAETIMEDOUT)
        return (FT_ERR_IO);
    if (error_code == WSAECONNREFUSED)
        return (FT_ERR_SOCKET_CONNECT_FAILED);
    if (error_code == WSAEHOSTDOWN)
        return (FT_ERR_IO);
    if (error_code == WSAEHOSTUNREACH)
        return (FT_ERR_IO);
    if (error_code == WSAEPROCLIM)
        return (FT_ERR_FULL);
    if (error_code == WSAEUSERS)
        return (FT_ERR_FULL);
    if (error_code == WSAEDQUOT)
        return (FT_ERR_FULL);
    if (error_code == WSAESTALE)
        return (FT_ERR_IO);
    if (error_code == WSAEREMOTE)
        return (FT_ERR_IO);
    if (error_code == WSAEDISCON)
        return (FT_ERR_IO);
    if (error_code == WSASYSNOTREADY)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAVERNOTSUPPORTED)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == WSANOTINITIALISED)
        return (FT_ERR_INVALID_STATE);
    if (error_code == WSAHOST_NOT_FOUND)
        return (FT_ERR_SOCKET_RESOLVE_NO_NAME);
    if (error_code == WSATRY_AGAIN)
        return (FT_ERR_SOCKET_RESOLVE_AGAIN);
    if (error_code == WSANO_RECOVERY)
        return (FT_ERR_SOCKET_RESOLVE_FAIL);
    if (error_code == WSANO_DATA)
        return (FT_ERR_SOCKET_RESOLVE_NO_NAME);
    if (error_code == ERROR_FILE_NOT_FOUND)
        return (FT_ERR_IO);
    if (error_code == ERROR_PATH_NOT_FOUND)
        return (FT_ERR_IO);
    if (error_code == ERROR_ACCESS_DENIED)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == ERROR_SHARING_VIOLATION)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == ERROR_LOCK_VIOLATION)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == ERROR_ALREADY_EXISTS)
        return (FT_ERR_ALREADY_EXISTS);
    if (error_code == ERROR_FILE_EXISTS)
        return (FT_ERR_ALREADY_EXISTS);
    if (error_code == ERROR_INVALID_HANDLE)
        return (FT_ERR_INVALID_HANDLE);
    if (error_code == ERROR_NOT_ENOUGH_MEMORY)
        return (FT_ERR_NO_MEMORY);
    if (error_code == ERROR_OUTOFMEMORY)
        return (FT_ERR_NO_MEMORY);
    if (error_code == ERROR_DISK_FULL)
        return (FT_ERR_FULL);
    if (error_code == ERROR_HANDLE_EOF)
        return (FT_ERR_END_OF_FILE);
    if (error_code == ERROR_INVALID_PARAMETER)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == ERROR_INVALID_NAME)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == ERROR_BROKEN_PIPE)
        return (FT_ERR_IO);
    return (static_cast<int>(error_code) + ERRNO_OFFSET);
}
#else
static int cmp_translate_posix_error(int error_code)
{
    if (error_code == EINTR)
        return (FT_ERR_INVALID_STATE);
    if (error_code == EAGAIN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == EWOULDBLOCK)
        return (FT_ERR_INVALID_STATE);
    if (error_code == EINPROGRESS)
        return (FT_ERR_INVALID_STATE);
    if (error_code == EALREADY)
        return (FT_ERR_INVALID_STATE);
    if (error_code == ENOENT)
        return (FT_ERR_IO);
    if (error_code == ENOTDIR)
        return (FT_ERR_IO);
    if (error_code == EISDIR)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == EACCES)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == EPERM)
        return (FT_ERR_INVALID_OPERATION);
    if (error_code == EINVAL)
        return (FT_ERR_INVALID_OPERATION);
#if defined(EFAULT)
    if (error_code == EFAULT)
        return (FT_ERR_INVALID_POINTER);
#endif
#if defined(E2BIG)
    if (error_code == E2BIG)
        return (FT_ERR_INVALID_ARGUMENT);
#endif
    if (error_code == EDOM)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == ESRCH)
        return (FT_ERR_NOT_FOUND);
    if (error_code == EEXIST)
        return (FT_ERR_ALREADY_EXISTS);
    if (error_code == EBUSY)
        return (FT_ERR_INVALID_STATE);
    if (error_code == ECHILD)
        return (FT_ERR_NOT_FOUND);
#if defined(EDEADLK)
    if (error_code == EDEADLK)
        return (FT_ERR_INVALID_STATE);
#endif
    if (error_code == EADDRINUSE)
        return (FT_ERR_ALREADY_EXISTS);
    if (error_code == EADDRNOTAVAIL)
        return (FT_ERR_NOT_FOUND);
    if (error_code == EBADF)
        return (FT_ERR_INVALID_HANDLE);
#if defined(ENODEV)
    if (error_code == ENODEV)
        return (FT_ERR_NOT_FOUND);
#endif
#if defined(ENXIO)
    if (error_code == ENXIO)
        return (FT_ERR_NOT_FOUND);
#endif
    if (error_code == ENOTDIR)
        return (FT_ERR_IO);
    if (error_code == ENOENT)
        return (FT_ERR_IO);
#if defined(ENFILE)
    if (error_code == ENFILE)
        return (FT_ERR_FULL);
#endif
    if (error_code == EMFILE)
        return (FT_ERR_FULL);
#if defined(EFBIG)
    if (error_code == EFBIG)
        return (FT_ERR_FULL);
#endif
#if defined(ENOLCK)
    if (error_code == ENOLCK)
        return (FT_ERR_INVALID_STATE);
#endif
    if (error_code == ENOTSOCK)
        return (FT_ERR_INVALID_HANDLE);
#if defined(ENOTTY)
    if (error_code == ENOTTY)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ENOTBLK)
    if (error_code == ENOTBLK)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ESPIPE)
    if (error_code == ESPIPE)
        return (FT_ERR_INVALID_OPERATION);
#endif
    if (error_code == EDESTADDRREQ)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == EMSGSIZE)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == EPROTOTYPE)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == ENOPROTOOPT)
        return (FT_ERR_INVALID_ARGUMENT);
    if (error_code == EPROTONOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == ESOCKTNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == EOPNOTSUPP)
        return (FT_ERR_INVALID_OPERATION);
#if defined(ENOTSUP)
    if (error_code == ENOTSUP)
        return (FT_ERR_INVALID_OPERATION);
#endif
    if (error_code == EPFNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
    if (error_code == EAFNOSUPPORT)
        return (FT_ERR_UNSUPPORTED_TYPE);
#if defined(ENAMETOOLONG)
    if (error_code == ENAMETOOLONG)
        return (FT_ERR_INVALID_ARGUMENT);
#endif
#if defined(ELOOP)
    if (error_code == ELOOP)
        return (FT_ERR_INVALID_OPERATION);
#endif
    if (error_code == ENOBUFS)
        return (FT_ERR_NO_MEMORY);
    if (error_code == ENOMEM)
        return (FT_ERR_NO_MEMORY);
    if (error_code == EIO)
        return (FT_ERR_IO);
#if defined(ENODATA)
    if (error_code == ENODATA)
        return (FT_ERR_IO);
#endif
#if defined(ETIME)
    if (error_code == ETIME)
        return (FT_ERR_IO);
#endif
#if defined(EBADMSG)
    if (error_code == EBADMSG)
        return (FT_ERR_IO);
#endif
#if defined(EMULTIHOP)
    if (error_code == EMULTIHOP)
        return (FT_ERR_IO);
#endif
#if defined(EPROTO)
    if (error_code == EPROTO)
        return (FT_ERR_IO);
#endif
#if defined(EREMOTEIO)
    if (error_code == EREMOTEIO)
        return (FT_ERR_IO);
#endif
#if defined(ENONET)
    if (error_code == ENONET)
        return (FT_ERR_IO);
#endif
    if (error_code == ENOSPC)
        return (FT_ERR_FULL);
#if defined(EOVERFLOW)
    if (error_code == EOVERFLOW)
        return (FT_ERR_OUT_OF_RANGE);
#endif
    if (error_code == ERANGE)
        return (FT_ERR_OUT_OF_RANGE);
#if defined(EILSEQ)
    if (error_code == EILSEQ)
        return (FT_ERR_INVALID_ARGUMENT);
#endif
#if defined(ECANCELED)
    if (error_code == ECANCELED)
        return (FT_ERR_TERMINATED);
#endif
#if defined(ENOSYS)
    if (error_code == ENOSYS)
        return (FT_ERR_UNSUPPORTED_TYPE);
#endif
#if defined(ENOLINK)
    if (error_code == ENOLINK)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ENOSTR)
    if (error_code == ENOSTR)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ENOSR)
    if (error_code == ENOSR)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ENOTEMPTY)
    if (error_code == ENOTEMPTY)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(EROFS)
    if (error_code == EROFS)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(EXDEV)
    if (error_code == EXDEV)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ETXTBSY)
    if (error_code == ETXTBSY)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(EUCLEAN)
    if (error_code == EUCLEAN)
        return (FT_ERR_INVALID_OPERATION);
#endif
#if defined(ENOTUNIQ)
    if (error_code == ENOTUNIQ)
        return (FT_ERR_ALREADY_EXISTS);
#endif
#if defined(EOWNERDEAD)
    if (error_code == EOWNERDEAD)
        return (FT_ERR_INVALID_STATE);
#endif
#if defined(ENOTRECOVERABLE)
    if (error_code == ENOTRECOVERABLE)
        return (FT_ERR_INVALID_STATE);
#endif
#if defined(ERFKILL)
    if (error_code == ERFKILL)
        return (FT_ERR_INVALID_STATE);
#endif
#if defined(EHWPOISON)
    if (error_code == EHWPOISON)
        return (FT_ERR_INVALID_STATE);
#endif
#if defined(EIDRM)
    if (error_code == EIDRM)
        return (FT_ERR_NOT_FOUND);
#endif
    if (error_code == ENETDOWN)
        return (FT_ERR_IO);
    if (error_code == ENETUNREACH)
        return (FT_ERR_IO);
    if (error_code == ENETRESET)
        return (FT_ERR_IO);
    if (error_code == ECONNABORTED)
        return (FT_ERR_IO);
    if (error_code == ECONNRESET)
        return (FT_ERR_IO);
    if (error_code == ETIMEDOUT)
        return (FT_ERR_IO);
    if (error_code == ECONNREFUSED)
        return (FT_ERR_SOCKET_CONNECT_FAILED);
    if (error_code == EHOSTDOWN)
        return (FT_ERR_IO);
    if (error_code == EHOSTUNREACH)
        return (FT_ERR_IO);
#if defined(EPROCLIM)
    if (error_code == EPROCLIM)
        return (FT_ERR_FULL);
#endif
#if defined(EUSERS)
    if (error_code == EUSERS)
        return (FT_ERR_FULL);
#endif
#if defined(EDQUOT)
    if (error_code == EDQUOT)
        return (FT_ERR_FULL);
#endif
#if defined(ESTALE)
    if (error_code == ESTALE)
        return (FT_ERR_IO);
#endif
#if defined(EREMOTE)
    if (error_code == EREMOTE)
        return (FT_ERR_IO);
#endif
    if (error_code == EISCONN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == ENOTCONN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == ESHUTDOWN)
        return (FT_ERR_INVALID_STATE);
    if (error_code == ETOOMANYREFS)
        return (FT_ERR_FULL);
    if (error_code == EPIPE)
        return (FT_ERR_IO);
    return (error_code + ERRNO_OFFSET);
}
#endif

int cmp_map_system_error_to_ft(int error_code)
{
    if (error_code == 0)
        return (ER_SUCCESS);
#if defined(_WIN32) || defined(_WIN64)
    return (cmp_translate_windows_error(error_code));
#else
    return (cmp_translate_posix_error(error_code));
#endif
}

int cmp_normalize_ft_errno(int error_code)
{
    if (error_code < ERRNO_OFFSET)
        return (error_code);
    return (cmp_map_system_error_to_ft(error_code - ERRNO_OFFSET));
}

void cmp_set_force_unsetenv_result(int result, int errno_value)
{
    global_force_unsetenv_enabled = 1;
    global_force_unsetenv_result = result;
    global_force_unsetenv_errno_value = errno_value;
#if defined(_WIN32) || defined(_WIN64)
    global_force_unsetenv_last_error = 0;
    global_force_unsetenv_socket_error = 0;
#endif
    return ;
}

void cmp_clear_force_unsetenv_result(void)
{
    global_force_unsetenv_enabled = 0;
    return ;
}

void cmp_set_force_putenv_result(int result, int errno_value)
{
    global_force_putenv_enabled = 1;
    global_force_putenv_result = result;
    global_force_putenv_errno_value = errno_value;
#if defined(_WIN32) || defined(_WIN64)
    global_force_putenv_last_error = 0;
#endif
    return ;
}

void cmp_clear_force_putenv_result(void)
{
    global_force_putenv_enabled = 0;
    return ;
}

void cmp_set_force_unsetenv_windows_errors(int last_error, int socket_error)
{
#if defined(_WIN32) || defined(_WIN64)
    global_force_unsetenv_last_error = last_error;
    global_force_unsetenv_socket_error = socket_error;
#else
    (void)last_error;
    (void)socket_error;
#endif
    return ;
}

void cmp_set_force_putenv_windows_error(int last_error)
{
#if defined(_WIN32) || defined(_WIN64)
    global_force_putenv_last_error = last_error;
#else
    (void)last_error;
#endif
    return ;
}

void cmp_set_force_cpu_count_success(unsigned int cpu_count)
{
    global_force_cpu_count_enabled = 1;
    global_force_cpu_count_should_fail = 0;
    global_force_cpu_count_value = cpu_count;
    global_force_cpu_count_errno_value = 0;
    return ;
}

void cmp_set_force_cpu_count_failure(int errno_value)
{
    global_force_cpu_count_enabled = 1;
    global_force_cpu_count_should_fail = 1;
    global_force_cpu_count_errno_value = errno_value;
    global_force_cpu_count_value = 0;
    return ;
}

void cmp_clear_force_cpu_count_result(void)
{
    global_force_cpu_count_enabled = 0;
    global_force_cpu_count_should_fail = 0;
    global_force_cpu_count_value = 0;
    global_force_cpu_count_errno_value = 0;
    return ;
}

void cmp_set_force_total_memory_success(unsigned long long memory_size)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 0;
    global_force_total_memory_value = memory_size;
    global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

void cmp_set_force_total_memory_failure(int errno_value)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 1;
    global_force_total_memory_errno_value = errno_value;
    global_force_total_memory_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

#if defined(_WIN32) || defined(_WIN64)
void cmp_set_force_total_memory_windows_failure(unsigned long last_error)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 1;
    global_force_total_memory_last_error = last_error;
    global_force_total_memory_errno_value = 0;
    global_force_total_memory_value = 0;
    return ;
}
#endif

void cmp_clear_force_total_memory_result(void)
{
    global_force_total_memory_enabled = 0;
    global_force_total_memory_should_fail = 0;
    global_force_total_memory_value = 0;
    global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

int cmp_secure_memzero(void *buffer, size_t length)
{
    if (buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (length == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
#if defined(_WIN32) || defined(_WIN64)
    SecureZeroMemory(buffer, length);
    ft_errno = ER_SUCCESS;
    return (0);
#else
    volatile unsigned char *volatile byte_pointer;

    byte_pointer = reinterpret_cast<volatile unsigned char *>(buffer);
    while (length > 0)
    {
        *byte_pointer = 0;
        byte_pointer = byte_pointer + 1;
        length = length - 1;
    }
    ft_errno = ER_SUCCESS;
    return (0);
#endif
}

int cmp_setenv(const char *name, const char *value, int overwrite)
{
    if (name == ft_nullptr || value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    if (!overwrite && getenv(name) != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    errno = 0;
    int result = _putenv_s(name, value);
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = ft_map_system_error(static_cast<int>(last_error));
        else if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    errno = 0;
    int result = setenv(name, value, overwrite);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

int cmp_unsetenv(const char *name)
{
    if (name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        SetLastError(global_force_unsetenv_last_error);
        WSASetLastError(global_force_unsetenv_socket_error);
        int forced_result = global_force_unsetenv_result;
        if (forced_result != 0)
        {
            if (global_force_unsetenv_last_error != 0)
                ft_errno = ft_map_system_error(global_force_unsetenv_last_error);
            else if (global_force_unsetenv_socket_error != 0)
                ft_errno = ft_map_system_error(global_force_unsetenv_socket_error);
            else if (global_force_unsetenv_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_unsetenv_errno_value);
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
        }
        else
            ft_errno = ER_SUCCESS;
        return (forced_result);
    }
    errno = 0;
    int result = _putenv_s(name, "");
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = ft_map_system_error(static_cast<int>(last_error));
        else if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        int forced_result = global_force_unsetenv_result;
        if (forced_result != 0)
        {
            if (global_force_unsetenv_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_unsetenv_errno_value);
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
        }
        else
            ft_errno = ER_SUCCESS;
        return (forced_result);
    }
    errno = 0;
    int result = unsetenv(name);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

int cmp_putenv(char *string)
{
    int result;

    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (global_force_putenv_enabled != 0)
    {
        errno = global_force_putenv_errno_value;
#if defined(_WIN32) || defined(_WIN64)
        SetLastError(global_force_putenv_last_error);
#endif
        result = global_force_putenv_result;
        if (result != 0)
        {
#if defined(_WIN32) || defined(_WIN64)
            if (global_force_putenv_last_error != 0)
                ft_errno = ft_map_system_error(global_force_putenv_last_error);
            else if (global_force_putenv_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_putenv_errno_value);
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
#else
            if (global_force_putenv_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_putenv_errno_value);
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
#endif
        }
        else
            ft_errno = ER_SUCCESS;
        return (result);
    }
#if defined(_WIN32) || defined(_WIN64)
    result = _putenv(string);
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = ft_map_system_error(static_cast<int>(last_error));
        else if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    result = putenv(string);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

char **cmp_get_environ_entries(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (_environ);
#else
    extern char **environ;
    return (environ);
#endif
}

const char *cmp_system_strerror(int error_code)
{
#if defined(_WIN32) || defined(_WIN64)
    static char message_buffer[512];
    DWORD system_error;
    DWORD format_result;

    if (error_code <= ERRNO_OFFSET)
        return (NULL);
    system_error = static_cast<DWORD>(error_code - ERRNO_OFFSET);
    format_result = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        system_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        message_buffer,
        static_cast<DWORD>(sizeof(message_buffer)),
        NULL);
    if (format_result != 0)
    {
        while (format_result > 0)
        {
            char current_character;

            current_character = message_buffer[format_result - 1];
            if (current_character != '\r' && current_character != '\n'
                && current_character != '\t' && current_character != ' '
                && current_character != '\0')
                break;
            message_buffer[format_result - 1] = '\0';
            format_result--;
        }
        if (format_result != 0)
            return (message_buffer);
    }
    return (strerror(static_cast<int>(system_error)));
#else
    if (error_code > ERRNO_OFFSET)
    {
        int standard_errno;

        standard_errno = error_code - ERRNO_OFFSET;
        return (strerror(standard_errno));
    }
    return (NULL);
#endif
}

char *cmp_get_home_directory(void)
{
#if defined(_WIN32) || defined(_WIN64)
    char *home;
    char *home_drive;
    char *home_path;
    char *combined_home;

    home = ft_getenv("USERPROFILE");
    if (home != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (home);
    }
    home_drive = ft_getenv("HOMEDRIVE");
    home_path = ft_getenv("HOMEPATH");
    if (home_drive == ft_nullptr || home_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    combined_home = cma_strjoin_multiple(2, home_drive, home_path);
    if (combined_home == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (combined_home);
#else
    char *home;

    home = ft_getenv("HOME");
    if (home == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (home);
#endif
}

unsigned int cmp_get_cpu_count(void)
{
    if (global_force_cpu_count_enabled != 0)
    {
        if (global_force_cpu_count_should_fail != 0)
        {
            if (global_force_cpu_count_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_cpu_count_errno_value);
            else
                ft_errno = FT_ERR_TERMINATED;
            return (0);
        }
        ft_errno = ER_SUCCESS;
        return (global_force_cpu_count_value);
    }
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO system_info;

    GetSystemInfo(&system_info);
    ft_errno = ER_SUCCESS;
    return (system_info.dwNumberOfProcessors);
#elif defined(__APPLE__) && defined(__MACH__)
    int cpu_count;
    size_t size;

    size = sizeof(cpu_count);
    errno = 0;
    if (sysctlbyname("hw.ncpu", &cpu_count, &size, ft_nullptr, 0) != 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned int>(cpu_count));
#else
    long cpu_count;

    errno = 0;
    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count < 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned int>(cpu_count));
#endif
}

unsigned long long cmp_get_total_memory(void)
{
    if (global_force_total_memory_enabled != 0)
    {
        if (global_force_total_memory_should_fail != 0)
        {
#if defined(_WIN32) || defined(_WIN64)
            if (global_force_total_memory_last_error != 0)
                ft_errno = ft_map_system_error(static_cast<int>(global_force_total_memory_last_error));
            else if (global_force_total_memory_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_total_memory_errno_value);
            else
                ft_errno = FT_ERR_TERMINATED;
#else
            if (global_force_total_memory_errno_value != 0)
                ft_errno = ft_map_system_error(global_force_total_memory_errno_value);
            else
                ft_errno = FT_ERR_TERMINATED;
#endif
            return (0);
        }
        ft_errno = ER_SUCCESS;
        return (global_force_total_memory_value);
    }
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength = sizeof(memory_status);
    if (GlobalMemoryStatusEx(&memory_status) == 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = ft_map_system_error(static_cast<int>(last_error));
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (memory_status.ullTotalPhys);
#elif defined(__APPLE__) && defined(__MACH__)
    unsigned long long memory_size;
    size_t size;

    size = sizeof(memory_size);
    errno = 0;
    if (sysctlbyname("hw.memsize", &memory_size, &size, ft_nullptr, 0) != 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (memory_size);
#else
    long pages;
    long page_size;

    errno = 0;
    pages = sysconf(_SC_PHYS_PAGES);
    if (pages < 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    errno = 0;
    page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size < 0)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned long long>(pages) *
            static_cast<unsigned long long>(page_size));
#endif
}

std::time_t cmp_timegm(std::tm *time_pointer)
{
    std::time_t conversion_result;

    if (time_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (static_cast<std::time_t>(-1));
    }
    errno = 0;
#if defined(_WIN32) || defined(_WIN64)
    conversion_result = _mkgmtime(time_pointer);
#else
    conversion_result = ::timegm(time_pointer);
#endif
    if (conversion_result == static_cast<std::time_t>(-1))
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = ER_SUCCESS;
    }
    else
        ft_errno = ER_SUCCESS;
    return (conversion_result);
}
