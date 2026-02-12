#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../File/open_dir.hpp"
#include "../Basic/basic.hpp"
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
    int operation_error = FT_ERR_SUCCESS;
    {
        unsigned long long operation_id = joined.last_operation_id();

        if (operation_id != 0)
            operation_error = joined.pop_operation_error(operation_id);
        if (operation_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(operation_error);
            return (joined);
        }
    }
    char separator = cmp_path_separator();
    if (joined.size() != 0)
    {
        const char *joined_data = joined.c_str();
        if (joined_data[joined.size() - 1] != separator)
            joined.append(separator);
    }
    {
        unsigned long long operation_id = joined.last_operation_id();

        if (operation_id != 0)
            operation_error = joined.pop_operation_error(operation_id);
        if (operation_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(operation_error);
            return (joined);
        }
    }
    size_t index = 0;
    if (right_path != ft_nullptr)
    {
        while (right_path[index] == separator)
            index += 1;
        while (right_path[index] != '\0')
        {
            joined.append(right_path[index]);
            {
                unsigned long long operation_id = joined.last_operation_id();

                if (operation_id != 0)
                    operation_error = joined.pop_operation_error(operation_id);
                if (operation_error != FT_ERR_SUCCESS)
                {
                    ft_global_error_stack_push(operation_error);
                    return (joined);
                }
            }
            index += 1;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (joined);
}

static int su_copy_file_streams(su_file *source_stream, su_file *destination_stream,
    int *error_code_out)
{
    char buffer[8192];
    int result = 0;
    int error_code = FT_ERR_SUCCESS;

    while (result == 0)
    {
        size_t bytes_read = su_fread(buffer, 1, sizeof(buffer), source_stream);
        int read_error = ft_global_error_stack_drop_last_error();
        if (bytes_read == 0)
        {
            if (read_error == FT_ERR_SUCCESS)
                break;
            error_code = read_error;
            result = -1;
            break;
        }
        if (read_error != FT_ERR_SUCCESS)
        {
            error_code = read_error;
            result = -1;
            break;
        }
        size_t bytes_written = su_fwrite(buffer, 1, bytes_read, destination_stream);
        int write_error = ft_global_error_stack_drop_last_error();
        if (bytes_written != bytes_read)
        {
            if (write_error == FT_ERR_SUCCESS)
                write_error = FT_ERR_IO;
            error_code = write_error;
            result = -1;
            break;
        }
        if (write_error != FT_ERR_SUCCESS)
        {
            error_code = write_error;
            result = -1;
            break;
        }
    }
    if (error_code_out != ft_nullptr)
        *error_code_out = error_code;
    return (result);
}

int su_copy_file(const char *source_path, const char *destination_path)
{
    su_file *source_stream;
    su_file *destination_stream;
    int result;
    int close_error;
    int error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    source_stream = su_fopen(source_path, O_RDONLY);
    if (source_stream == ft_nullptr)
    {
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_FILE_OPEN_FAILED;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    destination_stream = su_fopen(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_stream == ft_nullptr)
    {
        su_fclose(source_stream);
        ft_global_error_stack_drop_last_error();
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_FILE_OPEN_FAILED;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_drop_last_error();
    result = su_copy_file_streams(source_stream, destination_stream, &error_code);
    close_error = su_fclose(source_stream);
    if (close_error != 0)
    {
        int close_error_code = ft_global_error_stack_drop_last_error();

        if (result == 0)
        {
            if (close_error_code == FT_ERR_SUCCESS)
                close_error_code = FT_ERR_IO;
            error_code = close_error_code;
            result = -1;
        }
    }
    else
        ft_global_error_stack_drop_last_error();
    close_error = su_fclose(destination_stream);
    if (close_error != 0)
    {
        int close_error_code = ft_global_error_stack_drop_last_error();

        if (result == 0)
        {
            if (close_error_code == FT_ERR_SUCCESS)
                close_error_code = FT_ERR_IO;
            error_code = close_error_code;
            result = -1;
        }
    }
    else
        ft_global_error_stack_drop_last_error();
    if (result == 0)
        error_code = FT_ERR_SUCCESS;
    ft_global_error_stack_push(error_code);
    return (result);
}

static int su_ensure_directory_exists(const char *path, int *error_code_out)
{
    int directory_status;
    int directory_exists;
    int error_code;

    directory_exists = 0;
    directory_status = cmp_directory_exists(path, &directory_exists, &error_code);
    if (directory_status == FT_ERR_SUCCESS && directory_exists == 1)
    {
        if (error_code_out != ft_nullptr)
            *error_code_out = FT_ERR_SUCCESS;
        return (0);
    }
    if (directory_status == FT_ERR_SUCCESS && directory_exists == 0)
    {
        if (error_code_out != ft_nullptr)
            *error_code_out = FT_ERR_ALREADY_EXISTS;
        return (-1);
    }
    if (directory_status != FT_ERR_SUCCESS && directory_status != FT_ERR_IO)
    {
        if (error_code_out != ft_nullptr)
            *error_code_out = directory_status;
        return (-1);
    }
    if (cmp_file_create_directory(path, 0755, &error_code) != 0)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        if (error_code_out != ft_nullptr)
            *error_code_out = error_code;
        return (-1);
    }
    if (error_code_out != ft_nullptr)
        *error_code_out = FT_ERR_SUCCESS;
    return (0);
}

static int su_copy_directory_contents(const char *source_path, const char *destination_path,
    int *error_code_out)
{
    file_dir *directory_stream;
    int result;
    int error_code;

    directory_stream = cmp_dir_open(source_path, &error_code);
    if (directory_stream == ft_nullptr)
    {
        if (error_code_out != ft_nullptr)
            *error_code_out = error_code;
        return (-1);
    }
    result = 0;
    error_code = FT_ERR_SUCCESS;
    while (result == 0)
    {
        file_dirent *directory_entry = cmp_dir_read(directory_stream, &error_code);
        if (directory_entry == ft_nullptr)
        {
            if (error_code == FT_ERR_SUCCESS)
                break;
            result = -1;
            break;
        }
        if (su_should_skip_directory_entry(directory_entry->d_name))
            continue;
        ft_string source_child = su_join_paths(source_path, directory_entry->d_name);
        {
            int path_error = ft_global_error_stack_drop_last_error();

            if (path_error != FT_ERR_SUCCESS)
            {
                error_code = path_error;
                result = -1;
                break;
            }
        }
        ft_string destination_child = su_join_paths(destination_path, directory_entry->d_name);
        {
            int path_error = ft_global_error_stack_drop_last_error();

            if (path_error != FT_ERR_SUCCESS)
            {
                error_code = path_error;
                result = -1;
                break;
            }
        }
        int child_is_directory = 0;
        int child_status;

        child_status = cmp_directory_exists(source_child.c_str(),
            &child_is_directory, &error_code);
        if (child_status != FT_ERR_SUCCESS)
        {
            result = -1;
            error_code = child_status;
            break;
        }
        if (child_is_directory == 1)
        {
            int destination_status;
            int destination_is_directory = 0;

            destination_status = cmp_directory_exists(destination_child.c_str(),
                &destination_is_directory, &error_code);
            if (destination_status == FT_ERR_SUCCESS)
            {
                if (destination_is_directory == 0)
                {
                    error_code = FT_ERR_ALREADY_EXISTS;
                    result = -1;
                    break;
                }
            }
            else if (destination_status == FT_ERR_IO)
            {
                if (cmp_file_create_directory(destination_child.c_str(), 0755, &error_code) != FT_ERR_SUCCESS)
                {
                    if (error_code == FT_ERR_SUCCESS)
                        error_code = FT_ERR_INTERNAL;
                    result = -1;
                    break;
                }
            }
            else
            {
                error_code = destination_status;
                result = -1;
                break;
            }
            if (su_copy_directory_contents(source_child.c_str(),
                    destination_child.c_str(), &error_code) != 0)
            {
                result = -1;
                break;
            }
        }
        else
        {
            if (su_copy_file(source_child.c_str(), destination_child.c_str()) != 0)
            {
                error_code = ft_global_error_stack_drop_last_error();
                result = -1;
                break;
            }
            ft_global_error_stack_drop_last_error();
        }
    }
    {
        int close_error_code;
        int close_result;

        close_result = cmp_dir_close(directory_stream, &close_error_code);
        if (close_result != 0 && result == 0)
        {
            error_code = close_error_code;
            result = -1;
        }
    }
    if (result != 0 && error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_INTERNAL;
    if (result == 0)
        error_code = FT_ERR_SUCCESS;
    if (error_code_out != ft_nullptr)
        *error_code_out = error_code;
    return (result);
}

int su_copy_directory_recursive(const char *source_path, const char *destination_path)
{
    int error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int source_is_directory;
    int source_status;

    source_is_directory = 0;
    source_status = cmp_directory_exists(source_path, &source_is_directory,
        &error_code);
    if (source_status != FT_ERR_SUCCESS || source_is_directory != 1)
    {
        if (source_status == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        else
            error_code = source_status;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (su_ensure_directory_exists(destination_path, &error_code) != 0)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (su_copy_directory_contents(source_path, destination_path, &error_code) != 0)
    {
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

int su_inspect_permissions(const char *path, mode_t *permissions_out)
{
    mode_t permissions;
    int error_code;

    if (path == ft_nullptr || permissions_out == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (cmp_file_get_permissions(path, &permissions, &error_code) != 0)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    *permissions_out = permissions;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}
