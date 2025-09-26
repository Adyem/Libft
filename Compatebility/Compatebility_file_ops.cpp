#include "compatebility_internal.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include "../CPP_class/class_nullptr.hpp"
# include <windows.h>
# include <stdio.h>

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
    DWORD file_attributes = GetFileAttributesA(path);
    if (file_attributes != INVALID_FILE_ATTRIBUTES &&
        !(file_attributes & FILE_ATTRIBUTE_DIRECTORY))
        return (1);
    return (0);
}

int cmp_file_delete(const char *path)
{
    if (remove(path) == 0)
        return (0);
    return (-1);
}

int cmp_file_move(const char *source_path, const char *destination_path)
{
    DWORD last_error;

    if (global_force_cross_device_move == false)
    {
        if (MoveFileExA(source_path, destination_path,
                MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
            return (0);
        last_error = GetLastError();
    }
    else
        last_error = ERROR_NOT_SAME_DEVICE;
    if (last_error == ERROR_NOT_SAME_DEVICE)
    {
        if (cmp_file_copy(source_path, destination_path) == 0)
        {
            if (cmp_file_delete(source_path) == 0)
                return (0);
            cmp_file_delete(destination_path);
        }
    }
    return (-1);
}

int cmp_file_copy(const char *source_path, const char *destination_path)
{
    if (CopyFileA(source_path, destination_path, 0))
        return (0);
    return (-1);
}

int cmp_file_create_directory(const char *path, mode_t mode)
{
    (void)mode;
    if (CreateDirectoryA(path, ft_nullptr))
        return (0);
    return (-1);
}

#else
# include <filesystem>
# include <cstdio>
# include <cerrno>

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
    if (stat(path, &stat_buffer) == 0 && S_ISREG(stat_buffer.st_mode))
        return (1);
    return (0);
}

int cmp_file_delete(const char *path)
{
    if (unlink(path) == 0)
        return (0);
    return (-1);
}

int cmp_file_move(const char *source_path, const char *destination_path)
{
    if (global_force_cross_device_move != false)
        errno = EXDEV;
    if (global_force_cross_device_move == false)
    {
        if (rename(source_path, destination_path) == 0)
            return (0);
        if (errno != EXDEV)
            return (-1);
    }
    std::error_code copy_error_code;

    std::filesystem::copy_file(source_path, destination_path,
        std::filesystem::copy_options::overwrite_existing, copy_error_code);
    if (copy_error_code.value() == 0)
    {
        if (unlink(source_path) == 0)
            return (0);
        std::error_code remove_error_code;

        std::filesystem::remove(destination_path, remove_error_code);
    }
    return (-1);
}

int cmp_file_copy(const char *source_path, const char *destination_path)
{
    std::error_code copy_error_code;
    std::filesystem::copy_file(source_path, destination_path,
        std::filesystem::copy_options::overwrite_existing, copy_error_code);
    if (copy_error_code.value() == 0)
        return (0);
    return (-1);
}

int cmp_file_create_directory(const char *path, mode_t mode)
{
    if (mkdir(path, mode) == 0)
        return (0);
    return (-1);
}

#endif
