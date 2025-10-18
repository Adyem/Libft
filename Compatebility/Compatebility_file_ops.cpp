#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <stdio.h>
# include <errno.h>

void cmp_set_force_cross_device_move(int force_cross_device_move);

static bool global_force_cross_device_move = false;

void cmp_set_force_cross_device_move(int force_cross_device_move)
{
    if (force_cross_device_move != 0)
        global_force_cross_device_move = true;
    else
        global_force_cross_device_move = false;
    return ;
}

int cmp_file_exists(const char *path)
{
    DWORD file_attributes;
    DWORD last_error;

    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    file_attributes = GetFileAttributesA(path);
    if (file_attributes != INVALID_FILE_ATTRIBUTES)
    {
        if ((file_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            ft_errno = ER_SUCCESS;
            return (1);
        }
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (0);
}

int cmp_file_delete(const char *path)
{
    DWORD last_error;

    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (DeleteFileA(path) != 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int cmp_file_move(const char *source_path, const char *destination_path)
{
    DWORD last_error;
    int stored_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (global_force_cross_device_move == false)
    {
        if (MoveFileExA(source_path, destination_path,
                MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        last_error = GetLastError();
    }
    else
        last_error = ERROR_NOT_SAME_DEVICE;
    if (last_error == ERROR_NOT_SAME_DEVICE)
    {
        if (cmp_file_copy(source_path, destination_path) == 0)
        {
            if (cmp_file_delete(source_path) == 0)
            {
                ft_errno = ER_SUCCESS;
                return (0);
            }
            stored_error = ft_errno;
            cmp_file_delete(destination_path);
            ft_errno = stored_error;
        }
    }
    else if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int cmp_file_copy(const char *source_path, const char *destination_path)
{
    DWORD last_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (CopyFileA(source_path, destination_path, 0))
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int cmp_file_create_directory(const char *path, mode_t mode)
{
    (void)mode;
    DWORD last_error;

    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (CreateDirectoryA(path, ft_nullptr))
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        ft_errno = cmp_map_system_error_to_ft(static_cast<int>(last_error));
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

int cmp_file_get_permissions(const char *path, mode_t *mode_out)
{
    struct _stat file_info;

    if (path == ft_nullptr || mode_out == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (_stat(path, &file_info) == 0)
    {
        *mode_out = static_cast<mode_t>(file_info.st_mode);
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (errno != 0)
        ft_errno = cmp_map_system_error_to_ft(errno);
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
}

#else
# include <filesystem>
# include <cstdio>
# include <cerrno>

void cmp_set_force_cross_device_move(int force_cross_device_move);

static bool global_force_cross_device_move = false;

void cmp_set_force_cross_device_move(int force_cross_device_move)
{
    if (force_cross_device_move != 0)
        global_force_cross_device_move = true;
    else
        global_force_cross_device_move = false;
    return ;
}

int cmp_file_exists(const char *path)
{
    struct stat stat_buffer;
    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (stat(path, &stat_buffer) == 0)
    {
        if (S_ISREG(stat_buffer.st_mode))
        {
            ft_errno = ER_SUCCESS;
            return (1);
        }
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    ft_errno = cmp_map_system_error_to_ft(errno);
    return (0);
}

int cmp_file_delete(const char *path)
{
    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (unlink(path) == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = cmp_map_system_error_to_ft(errno);
    return (-1);
}

int cmp_file_move(const char *source_path, const char *destination_path)
{
    std::error_code copy_error_code;
    int delete_errno;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (global_force_cross_device_move != false)
        errno = EXDEV;
    if (global_force_cross_device_move == false)
    {
        if (rename(source_path, destination_path) == 0)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        if (errno != EXDEV)
        {
            ft_errno = cmp_map_system_error_to_ft(errno);
            return (-1);
        }
    }
    std::filesystem::copy_file(source_path, destination_path,
        std::filesystem::copy_options::overwrite_existing, copy_error_code);
    if (copy_error_code.value() == 0)
    {
        if (unlink(source_path) == 0)
        {
            ft_errno = ER_SUCCESS;
            return (0);
        }
        delete_errno = errno;
        std::error_code remove_error_code;

        std::filesystem::remove(destination_path, remove_error_code);
        ft_errno = cmp_map_system_error_to_ft(delete_errno);
        return (-1);
    }
    if (copy_error_code.value() != 0)
    {
        int copy_value;

        copy_value = copy_error_code.value();
        ft_errno = cmp_map_system_error_to_ft(copy_value);
    }
    return (-1);
}

int cmp_file_copy(const char *source_path, const char *destination_path)
{
    std::error_code copy_error_code;
    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    std::filesystem::copy_file(source_path, destination_path,
        std::filesystem::copy_options::overwrite_existing, copy_error_code);
    if (copy_error_code.value() == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (copy_error_code.value() != 0)
    {
        int copy_value;

        copy_value = copy_error_code.value();
        ft_errno = cmp_map_system_error_to_ft(copy_value);
    }
    return (-1);
}

int cmp_file_create_directory(const char *path, mode_t mode)
{
    if (path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (mkdir(path, mode) == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = cmp_map_system_error_to_ft(errno);
    return (-1);
}

int cmp_file_get_permissions(const char *path, mode_t *mode_out)
{
    struct stat file_info;

    if (path == ft_nullptr || mode_out == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (stat(path, &file_info) == 0)
    {
        *mode_out = file_info.st_mode;
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = cmp_map_system_error_to_ft(errno);
    return (-1);
}

#endif
