#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Errno/errno.hpp"
#include "../File/open_dir.hpp"
#include "../Libft/libft.hpp"
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>

static bool su_should_skip_directory_entry(const char *entry_name)
{
    if (entry_name == ft_nullptr)
        return (true);
    if (entry_name[0] == '.' && entry_name[1] == '\0')
        return (true);
    if (entry_name[0] == '.' && entry_name[1] == '.' && entry_name[2] == '\0')
        return (true);
    return (false);
}

static ft_string su_join_paths(const char *left_path, const char *right_path)
{
    ft_string joined(left_path);
    if (joined.get_error())
        return (joined);
    char separator = cmp_path_separator();
    if (joined.size() != 0)
    {
        const char *joined_data = joined.c_str();
        if (joined_data[joined.size() - 1] != separator)
            joined.append(separator);
    }
    if (joined.get_error())
        return (joined);
    size_t index = 0;
    if (right_path != ft_nullptr)
    {
        while (right_path[index] == separator)
            index += 1;
        while (right_path[index] != '\0')
        {
            joined.append(right_path[index]);
            if (joined.get_error())
                return (joined);
            index += 1;
        }
    }
    return (joined);
}

static int su_copy_file_streams(su_file *source_stream, su_file *destination_stream)
{
    char buffer[8192];
    int result = 0;

    while (result == 0)
    {
        size_t bytes_read = su_fread(buffer, 1, sizeof(buffer), source_stream);
        if (bytes_read == 0)
        {
            if (ft_errno == ER_SUCCESS)
                break;
            result = -1;
            break;
        }
        size_t bytes_written = su_fwrite(buffer, 1, bytes_read, destination_stream);
        if (bytes_written != bytes_read)
        {
            if (ft_errno == ER_SUCCESS)
                ft_errno = FT_ERR_IO;
            result = -1;
        }
    }
    return (result);
}

int su_copy_file(const char *source_path, const char *destination_path)
{
    su_file *source_stream;
    su_file *destination_stream;
    int result;
    int close_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    source_stream = su_fopen(source_path, O_RDONLY);
    if (source_stream == ft_nullptr)
        return (-1);
    destination_stream = su_fopen(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_stream == ft_nullptr)
    {
        su_fclose(source_stream);
        return (-1);
    }
    result = su_copy_file_streams(source_stream, destination_stream);
    close_error = su_fclose(source_stream);
    if (close_error != 0 && result == 0)
        result = -1;
    close_error = su_fclose(destination_stream);
    if (close_error != 0 && result == 0)
        result = -1;
    if (result == 0)
        ft_errno = ER_SUCCESS;
    return (result);
}

static int su_ensure_directory_exists(const char *path)
{
    int directory_status;

    directory_status = cmp_directory_exists(path);
    if (directory_status == 1)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (directory_status == 0 && ft_errno == ER_SUCCESS)
    {
        ft_errno = FT_ERR_ALREADY_EXISTS;
        return (-1);
    }
    if (cmp_file_create_directory(path, 0755) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

static int su_copy_directory_contents(const char *source_path, const char *destination_path)
{
    file_dir *directory_stream;
    int result;

    directory_stream = cmp_dir_open(source_path);
    if (directory_stream == ft_nullptr)
        return (-1);
    result = 0;
    while (result == 0)
    {
        file_dirent *directory_entry = cmp_dir_read(directory_stream);
        if (directory_entry == ft_nullptr)
        {
            if (ft_errno == ER_SUCCESS)
                break;
            result = -1;
            break;
        }
        if (su_should_skip_directory_entry(directory_entry->d_name))
            continue;
        ft_string source_child = su_join_paths(source_path, directory_entry->d_name);
        if (source_child.get_error())
        {
            ft_errno = source_child.get_error();
            result = -1;
            break;
        }
        ft_string destination_child = su_join_paths(destination_path, directory_entry->d_name);
        if (destination_child.get_error())
        {
            ft_errno = destination_child.get_error();
            result = -1;
            break;
        }
        int child_is_directory = cmp_directory_exists(source_child.c_str());
        if (child_is_directory == 1)
        {
            int destination_status = cmp_directory_exists(destination_child.c_str());
            if (destination_status == 0)
            {
                if (ft_errno == ER_SUCCESS)
                {
                    ft_errno = FT_ERR_ALREADY_EXISTS;
                    result = -1;
                    break;
                }
                if (cmp_file_create_directory(destination_child.c_str(), 0755) != 0)
                {
                    result = -1;
                    break;
                }
                ft_errno = ER_SUCCESS;
            }
            else if (destination_status != 1)
            {
                result = -1;
                break;
            }
            if (su_copy_directory_contents(source_child.c_str(), destination_child.c_str()) != 0)
            {
                result = -1;
                break;
            }
        }
        else if (child_is_directory == 0)
        {
            if (ft_errno != ER_SUCCESS)
            {
                result = -1;
                break;
            }
            if (su_copy_file(source_child.c_str(), destination_child.c_str()) != 0)
            {
                result = -1;
                break;
            }
        }
        else
        {
            result = -1;
            break;
        }
    }
    cmp_dir_close(directory_stream);
    if (result == 0)
        ft_errno = ER_SUCCESS;
    return (result);
}

int su_copy_directory_recursive(const char *source_path, const char *destination_path)
{
    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (cmp_directory_exists(source_path) != 1)
        return (-1);
    if (su_ensure_directory_exists(destination_path) != 0)
        return (-1);
    if (su_copy_directory_contents(source_path, destination_path) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

int su_inspect_permissions(const char *path, mode_t *permissions_out)
{
    mode_t permissions;

    if (path == ft_nullptr || permissions_out == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (cmp_file_get_permissions(path, &permissions) != 0)
        return (-1);
    *permissions_out = permissions;
    ft_errno = ER_SUCCESS;
    return (0);
}
