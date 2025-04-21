#include <unistd.h>
#include <fcntl.h>
#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/syscall.h>
#endif

FT_DIR* ft_opendir(const char* directoryPath)
{
    int fileDescriptor = ft_open(directoryPath, O_DIRECTORY | O_RDONLY, 0);
    if (fileDescriptor < 0)
        return (ft_nullptr);
    FT_DIR* directoryStream = reinterpret_cast<FT_DIR*>(cma_malloc(sizeof(FT_DIR)));
    if (!directoryStream)
    {
        ft_close(fileDescriptor);
        return (ft_nullptr);
    }
    memset(directoryStream, 0, sizeof(FT_DIR));
    directoryStream->fd = fileDescriptor;
    directoryStream->buffer_size = 4096;
    directoryStream->buffer = reinterpret_cast<char*>(cma_malloc(directoryStream->buffer_size));
    if (!directoryStream->buffer)
    {
        cma_free(directoryStream);
        ft_close(fileDescriptor);
        return (ft_nullptr);
    }
    directoryStream->buffer_used   = 0;
    directoryStream->buffer_offset = 0;
    return (directoryStream);
}

ft_dirent* ft_readdir(FT_DIR* dir)
{
	if (!dir)
		return ft_nullptr;
	#ifdef _WIN32
	    WIN32_FIND_DATAA* fd = &dir->w_findData;
	    if (dir->first_read)
	        dir->first_read = false;
	    else
	    {
	        if (!FindNextFileA(reinterpret_cast<HANDLE>(dir->fd), fd))
	            return ft_nullptr;
	    }
	    static ft_dirent entry;
	    ft_bzero(&entry, sizeof(entry));
	    BY_HANDLE_FILE_INFORMATION info;
	    if (GetFileInformationByHandle(reinterpret_cast<HANDLE>(dir->fd), &info))
	        entry.d_ino = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) |
	                       info.nFileIndexLow;
	    strncpy(entry.d_name, fd->cFileName, sizeof(entry.d_name) - 1);
	    entry.d_type = (fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DT_DIR : DT_REG;
	    return &entry;	
	#else
    	if (dir->buffer_offset >= static_cast<size_t>(dir->buffer_used))
    	{
    	    dir->buffer_offset = 0;
    	    long n = syscall(SYS_getdents64, dir->fd,
    	                     reinterpret_cast<linux_dirent64*>(dir->buffer),
    	                     dir->buffer_size);
    	    if (n <= 0)
    	        return ft_nullptr;
    	    dir->buffer_used = n;
    	}
    	linux_dirent64* raw = reinterpret_cast<linux_dirent64*>
    	                      (dir->buffer + dir->buffer_offset);
    	if (raw->d_reclen == 0)
    	    return ft_nullptr;
    	static ft_dirent entry;
    	ft_bzero(&entry, sizeof(entry));
    	entry.d_ino  = raw->d_ino;
    	entry.d_type = raw->d_type;
   		strncpy(entry.d_name, raw->d_name, sizeof(entry.d_name) - 1);
    	dir->buffer_offset += raw->d_reclen;
    	return &entry;
	#endif
}

int ft_closedir(FT_DIR* directoryStream)
{
    if (!directoryStream)
        return (-1);
    ft_close(directoryStream->fd);
    cma_free(directoryStream->buffer);
    cma_free(directoryStream);
    return (0);
}
