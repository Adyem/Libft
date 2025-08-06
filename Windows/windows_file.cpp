#if defined(_WIN32) || defined(_WIN64)

#include "windows_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../Libft/libft.hpp"
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <thread>
#include <windows.h>
#include <cstdio>

static HANDLE g_handles[1024];
static pt_mutex g_file_mutex;

static int store_handle(HANDLE h)
{
    int index = 3;
    g_file_mutex.lock(GetCurrentThreadId());
    while (index < 1024)
    {
        if (g_handles[index] == ft_nullptr)
        {
            g_handles[index] = h;
            g_file_mutex.unlock(GetCurrentThreadId());
            return (index);
        }
        index++;
    }
    g_file_mutex.unlock(GetCurrentThreadId());
    return (-1);
}

static HANDLE retrieve_handle(int fd)
{
    if (fd < 0 || fd >= 1024)
	{
		g_file_mutex.unlock(GetCurrentThreadId());
		return (INVALID_HANDLE_VALUE);
	}
    return (g_handles[fd]);
}

static void clear_handle(int fd)
{
    g_file_mutex.lock(GetCurrentThreadId());
    if (fd < 0 || fd >= 1024)
    {
        g_file_mutex.unlock(GetCurrentThreadId());
        return;
    }
    g_handles[fd] = ft_nullptr;
    g_file_mutex.unlock(GetCurrentThreadId());
    return;
}

int ft_open(const char *pathname, int flags, int mode)
{
	g_file_mutex.lock(GetCurrentThreadId());
    DWORD desiredAccess       = 0;
    DWORD creationDisposition = 0;
    DWORD flagsAndAttributes  = FILE_ATTRIBUTE_NORMAL;
    (void)mode;
    if (flags & O_DIRECTORY)
	{
        desiredAccess       = FILE_LIST_DIRECTORY;
        creationDisposition = OPEN_EXISTING;
        flagsAndAttributes  = FILE_FLAG_BACKUP_SEMANTICS;
    }
    else 
	{
        if ((flags & O_RDWR) == O_RDWR)
            desiredAccess = GENERIC_READ | GENERIC_WRITE;
        else if (flags & O_WRONLY)
            desiredAccess = GENERIC_WRITE;
        else
            desiredAccess = GENERIC_READ;
        if ((flags & O_CREAT) && (flags & O_EXCL))
            creationDisposition = CREATE_NEW;
        else if ((flags & O_CREAT) && (flags & O_TRUNC))
            creationDisposition = CREATE_ALWAYS;
        else if (flags & O_CREAT)
            creationDisposition = OPEN_ALWAYS;
        else if (flags & O_TRUNC)
            creationDisposition = TRUNCATE_EXISTING;
        else
            creationDisposition = OPEN_EXISTING;
        if (flags & O_APPEND)
            desiredAccess |= FILE_APPEND_DATA;
    }
    HANDLE hFile = CreateFileA(pathname, desiredAccess, FILE_SHARE_READ | FILE_SHARE_WRITE,
        ft_nullptr, creationDisposition, flagsAndAttributes, ft_nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
	{
		g_file_mutex.unlock(GetCurrentThreadId());
        return (-1);
	}
    int fd = store_handle(hFile);
    if (fd < 0)
	{
        CloseHandle(hFile);
		g_file_mutex.unlock(GetCurrentThreadId());
        return (-1);
    }
	g_file_mutex.unlock(GetCurrentThreadId());
    return (fd);
}

ssize_t ft_read(int fd, void *buf, unsigned int count)
{
	g_file_mutex.lock(GetCurrentThreadId());
    HANDLE hFile = retrieve_handle(fd);
    if (hFile == INVALID_HANDLE_VALUE)
	{
		g_file_mutex.unlock(GetCurrentThreadId());
        return (-1);
	}
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(hFile, &info))
	{
        if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			g_file_mutex.unlock(GetCurrentThreadId());
            return (-1);
		}
    }
    DWORD bytesRead = 0;
    BOOL ok = ReadFile(hFile, buf, count, &bytesRead, NULL);
    if (!ok)
	{
		g_file_mutex.unlock(GetCurrentThreadId());
        return (-1);
	}
    return (bytesRead);
}

ssize_t ft_write(int fd, const void *buf, unsigned int count)
{
    g_file_mutex.lock(GetCurrentThreadId());
    HANDLE hFile = retrieve_handle(fd);
    if (hFile == INVALID_HANDLE_VALUE)
        return (-1);
    DWORD bytesWritten = 0;
    BOOL ok = WriteFile(hFile, buf, count, &bytesWritten, NULL);
    g_file_mutex.unlock(GetCurrentThreadId());
    if (!ok)
        return (-1);
    return (bytesWritten);
}

int ft_close(int fd)
{
    HANDLE hFile = retrieve_handle(fd);
    if (hFile == INVALID_HANDLE_VALUE)
	{
		g_file_mutex.unlock(GetCurrentThreadId());
		return (-1);
	}
    if (!CloseHandle(hFile))
	{
		g_file_mutex.unlock(GetCurrentThreadId());
		return (-1);
	}
    clear_handle(fd);
	g_file_mutex.unlock(GetCurrentThreadId());
    return (0);
}

void ft_initialize_standard_file_descriptors()
{
	static int initialized = 0;

	if (initialized == 1)
		return ;
    HANDLE hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (hStdInput != INVALID_HANDLE_VALUE)
    {
        int fdInput = _open_osfhandle(reinterpret_cast<intptr_t>(hStdInput), _O_RDONLY);
        if (fdInput != -1)
            _dup2(fdInput, 0);
    }
    if (hStdOutput != INVALID_HANDLE_VALUE)
    {
        int fdOutput = _open_osfhandle(reinterpret_cast<intptr_t>(hStdOutput), _O_WRONLY);
        if (fdOutput != -1)
            _dup2(fdOutput, 1);
    }
    if (hStdError != INVALID_HANDLE_VALUE)
    {
        int fdError = _open_osfhandle(reinterpret_cast<intptr_t>(hStdError), _O_WRONLY);
        if (fdError != -1)
            _dup2(fdError, 2);
    }
    g_file_mutex.lock(GetCurrentThreadId());
    g_handles[0] = hStdInput;
    g_handles[1] = hStdOutput;
    g_handles[2] = hStdError;
    g_file_mutex.unlock(GetCurrentThreadId());
    _setmode(0, _O_BINARY);
    _setmode(1, _O_BINARY);
    _setmode(2, _O_BINARY);
	initialized = 1;
	return ;
}

#endif
