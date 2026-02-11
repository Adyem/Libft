#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

static void cmp_set_error_code(int32_t *error_code_out, int32_t error_code)
{
    if (error_code_out != ft_nullptr)
        *error_code_out = error_code;
    return ;
}

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <stdio.h>
# include <errno.h>

void cmp_set_force_cross_device_move(int32_t force_cross_device_move);

static bool global_force_cross_device_move = false;

void cmp_set_force_cross_device_move(int32_t force_cross_device_move)
{
    if (force_cross_device_move != 0)
        global_force_cross_device_move = true;
    else
        global_force_cross_device_move = false;
    return ;
}

int32_t cmp_file_exists(const char *path, int32_t *error_code_out)
{
    DWORD file_attributes;
    DWORD last_error;

    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    file_attributes = GetFileAttributesA(path);
    if (file_attributes != INVALID_FILE_ATTRIBUTES)
    {
        if ((file_attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
            return (1);
        }
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(static_cast<int32_t>(last_error)));
    else
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
    return (0);
}

int32_t cmp_file_delete(const char *path, int32_t *error_code_out)
{
    DWORD last_error;

    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (DeleteFileA(path) != 0)
    {
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(static_cast<int32_t>(last_error)));
    else
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int32_t cmp_file_move(const char *source_path, const char *destination_path, int32_t *error_code_out)
{
    DWORD last_error;
    int32_t stored_error;
    int32_t delete_error;
    int32_t copy_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (global_force_cross_device_move == false)
    {
        if (MoveFileExA(source_path, destination_path,
                MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
        {
            cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
            return (0);
        }
        last_error = GetLastError();
    }
    else
        last_error = ERROR_NOT_SAME_DEVICE;
    if (last_error == ERROR_NOT_SAME_DEVICE)
    {
        if (cmp_file_copy(source_path, destination_path, &copy_error) == 0)
        {
            if (cmp_file_delete(source_path, &delete_error) == 0)
            {
                cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
                return (0);
            }
            stored_error = delete_error;
            cmp_file_delete(destination_path, &delete_error);
            if (stored_error == FT_ERR_SUCCESSS)
                stored_error = FT_ERR_INTERNAL;
            cmp_set_error_code(error_code_out, stored_error);
            return (-1);
        }
        if (copy_error != FT_ERR_SUCCESSS)
        {
            cmp_set_error_code(error_code_out, copy_error);
            return (-1);
        }
        cmp_set_error_code(error_code_out, FT_ERR_INTERNAL);
        return (-1);
    }
    else if (last_error != 0)
        stored_error = cmp_map_system_error_to_ft(static_cast<int32_t>(last_error));
    else
        stored_error = FT_ERR_INVALID_ARGUMENT;
    if (stored_error == FT_ERR_SUCCESSS)
        stored_error = FT_ERR_INTERNAL;
    cmp_set_error_code(error_code_out, stored_error);
    return (-1);
}

int32_t cmp_file_copy(const char *source_path, const char *destination_path, int32_t *error_code_out)
{
    DWORD last_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (CopyFileA(source_path, destination_path, 0))
    {
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(static_cast<int32_t>(last_error)));
    else
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int32_t cmp_file_create_directory(const char *path, mode_t mode, int32_t *error_code_out)
{
    (void)mode;
    DWORD last_error;

    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (CreateDirectoryA(path, ft_nullptr))
    {
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    last_error = GetLastError();
    if (last_error != 0)
        cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(static_cast<int32_t>(last_error)));
    else
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int32_t cmp_file_get_permissions(const char *path, mode_t *mode_out, int32_t *error_code_out)
{
    struct _stat file_info;

    if (path == ft_nullptr || mode_out == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (_stat(path, &file_info) == 0)
    {
        *mode_out = static_cast<mode_t>(file_info.st_mode);
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    if (errno != 0)
        cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
    else
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

#else
# include <filesystem>
# include <cstdio>
# include <cerrno>
# include <system_error>
# include <new>

void cmp_set_force_cross_device_move(int32_t force_cross_device_move);

static bool global_force_cross_device_move = false;

void cmp_set_force_cross_device_move(int32_t force_cross_device_move)
{
    if (force_cross_device_move != 0)
        global_force_cross_device_move = true;
    else
        global_force_cross_device_move = false;
    return ;
}

int32_t cmp_file_exists(const char *path, int32_t *error_code_out)
{
    struct stat stat_buffer;
    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (stat(path, &stat_buffer) == 0)
    {
        if (S_ISREG(stat_buffer.st_mode))
        {
            cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
            return (1);
        }
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
    return (0);
}

int32_t cmp_file_delete(const char *path, int32_t *error_code_out)
{
    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (unlink(path) == 0)
    {
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
    return (-1);
}

int32_t cmp_file_move(const char *source_path, const char *destination_path, int32_t *error_code_out)
{
    std::error_code copy_error_code;
    std::error_code directory_error_code;
    int32_t delete_errno;
    bool destination_is_directory;

    try
    {
        if (source_path == ft_nullptr || destination_path == ft_nullptr)
        {
            cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        if (global_force_cross_device_move != false)
            errno = EXDEV;
        if (global_force_cross_device_move == false)
        {
            if (rename(source_path, destination_path) == 0)
            {
                cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
                return (0);
            }
            if (errno != EXDEV)
            {
                cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
                return (-1);
            }
        }
        destination_is_directory = std::filesystem::is_directory(destination_path,
            directory_error_code);
        if (directory_error_code.value() != 0
            && directory_error_code.value()
                != static_cast<int32_t>(std::errc::not_a_directory))
        {
            cmp_set_error_code(error_code_out,
                cmp_map_system_error_to_ft(directory_error_code.value()));
            return (-1);
        }
        if (destination_is_directory != false)
        {
            cmp_set_error_code(error_code_out, FT_ERR_INVALID_OPERATION);
            return (-1);
        }
        std::filesystem::copy_file(source_path, destination_path,
            std::filesystem::copy_options::overwrite_existing, copy_error_code);
        if (copy_error_code.value() == 0)
        {
            if (unlink(source_path) == 0)
            {
                cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
                return (0);
            }
            delete_errno = errno;
            std::error_code remove_error_code;

            std::filesystem::remove(destination_path, remove_error_code);
            cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(delete_errno));
            return (-1);
        }
        if (copy_error_code.value() != 0)
        {
            int32_t copy_value;

            copy_value = copy_error_code.value();
            cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(copy_value));
        }
    }
    catch (const std::bad_alloc &)
    {
        cmp_set_error_code(error_code_out, FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INTERNAL);
    }
    return (-1);
}

int32_t cmp_file_copy(const char *source_path, const char *destination_path, int32_t *error_code_out)
{
    std::error_code copy_error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    try
    {
        std::filesystem::copy_file(source_path, destination_path,
            std::filesystem::copy_options::overwrite_existing, copy_error_code);
        if (copy_error_code.value() == 0)
        {
            cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
            return (0);
        }
        if (copy_error_code.value() != 0)
        {
            int32_t copy_value;

            copy_value = copy_error_code.value();
            cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(copy_value));
        }
    }
    catch (const std::bad_alloc &)
    {
        cmp_set_error_code(error_code_out, FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INTERNAL);
    }
    return (-1);
}

int32_t cmp_file_create_directory(const char *path, mode_t mode, int32_t *error_code_out)
{
    if (path == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (mkdir(path, mode) == 0)
    {
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
    return (-1);
}

int32_t cmp_file_get_permissions(const char *path, mode_t *mode_out, int32_t *error_code_out)
{
    struct stat file_info;

    if (path == ft_nullptr || mode_out == ft_nullptr)
    {
        cmp_set_error_code(error_code_out, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (stat(path, &file_info) == 0)
    {
        *mode_out = file_info.st_mode;
        cmp_set_error_code(error_code_out, FT_ERR_SUCCESSS);
        return (0);
    }
    cmp_set_error_code(error_code_out, cmp_map_system_error_to_ft(errno));
    return (-1);
}

#endif
