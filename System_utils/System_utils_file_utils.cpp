#include "system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../File/open_dir.hpp"
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

static int su_join_paths(const char *left_path, const char *right_path, ft_string *joined)
{
    char    separator;
    size_t  index;
    int32_t string_error;

    if (left_path == ft_nullptr || right_path == ft_nullptr || joined == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    string_error = joined->initialize(left_path);
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    separator = cmp_path_separator();
    if (joined->size() != 0)
    {
        if (joined->c_str()[joined->size() - 1] != separator)
        {
            string_error = joined->append(separator);
            if (string_error != FT_ERR_SUCCESS)
                return (string_error);
        }
    }
    index = 0;
    while (right_path[index] == separator)
        index += 1;
    string_error = joined->append(right_path + index);
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    return (FT_ERR_SUCCESS);
}

static int su_copy_file_streams(su_file *source_stream, su_file *destination_stream)
{
    char    buffer[8192];
    size_t  bytes_read;
    size_t  bytes_written;

    while (true)
    {
        bytes_read = su_fread(buffer, 1, sizeof(buffer), source_stream);
        if (bytes_read == 0)
            return (0);
        bytes_written = su_fwrite(buffer, 1, bytes_read, destination_stream);
        if (bytes_written != bytes_read)
            return (-1);
    }
}

int su_copy_file(const char *source_path, const char *destination_path)
{
    su_file *source_stream;
    su_file *destination_stream;
    int     result;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (-1);
    source_stream = su_fopen(source_path, O_RDONLY);
    if (source_stream == ft_nullptr)
        return (-1);
    destination_stream = su_fopen(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_stream == ft_nullptr)
    {
        (void)su_fclose(source_stream);
        return (-1);
    }
    result = su_copy_file_streams(source_stream, destination_stream);
    if (su_fclose(source_stream) != 0)
        result = -1;
    if (su_fclose(destination_stream) != 0)
        result = -1;
    return (result);
}

static int su_ensure_directory_exists(const char *path)
{
    int directory_status;
    int directory_exists;
    int error_code;

    directory_exists = 0;
    directory_status = cmp_directory_exists(path, &directory_exists, &error_code);
    if (directory_status == FT_ERR_SUCCESS && directory_exists == 1)
        return (0);
    if (cmp_file_create_directory(path, 0755, &error_code) != 0)
        return (-1);
    return (0);
}

static int su_copy_directory_contents(const char *source_path, const char *destination_path)
{
    file_dir     *directory_stream;
    file_dirent  *directory_entry;
    int          error_code;
    int          child_is_directory;
    int          child_status;

    directory_stream = cmp_dir_open(source_path, &error_code);
    if (directory_stream == ft_nullptr)
        return (-1);
    while (true)
    {
        directory_entry = cmp_dir_read(directory_stream, &error_code);
        if (directory_entry == ft_nullptr)
            break;
        if (su_should_skip_directory_entry(directory_entry->d_name))
            continue ;
        ft_string source_child;
        ft_string destination_child;

        if (su_join_paths(source_path, directory_entry->d_name, &source_child) != FT_ERR_SUCCESS)
            break;
        if (su_join_paths(destination_path, directory_entry->d_name, &destination_child) != FT_ERR_SUCCESS)
            break;
        child_is_directory = 0;
        child_status = cmp_directory_exists(source_child.c_str(), &child_is_directory, &error_code);
        if (child_status != FT_ERR_SUCCESS)
            break;
        if (child_is_directory == 1)
        {
            if (su_ensure_directory_exists(destination_child.c_str()) != 0)
                break;
            if (su_copy_directory_contents(source_child.c_str(), destination_child.c_str()) != 0)
                break;
        }
        else
        {
            if (su_copy_file(source_child.c_str(), destination_child.c_str()) != 0)
                break;
        }
    }
    if (directory_entry == ft_nullptr && error_code == FT_ERR_SUCCESS)
    {
        if (cmp_dir_close(directory_stream, &error_code) != 0)
            return (-1);
        return (0);
    }
    (void)cmp_dir_close(directory_stream, &error_code);
    return (-1);
}

int su_copy_directory_recursive(const char *source_path, const char *destination_path)
{
    int source_is_directory;
    int source_status;
    int error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (-1);
    source_is_directory = 0;
    source_status = cmp_directory_exists(source_path, &source_is_directory, &error_code);
    if (source_status != FT_ERR_SUCCESS || source_is_directory != 1)
        return (-1);
    if (su_ensure_directory_exists(destination_path) != 0)
        return (-1);
    if (su_copy_directory_contents(source_path, destination_path) != 0)
        return (-1);
    return (0);
}

int su_inspect_permissions(const char *path, mode_t *permissions_out)
{
    mode_t  permissions;
    int     error_code;

    if (path == ft_nullptr || permissions_out == ft_nullptr)
        return (-1);
    if (cmp_file_get_permissions(path, &permissions, &error_code) != 0)
        return (-1);
    *permissions_out = permissions;
    return (0);
}

int su_chmod(const char *path, int owner_permissions, int group_permissions,
    int other_permissions)
{
    int error_code;

    if (path == ft_nullptr)
        return (-1);
    if (cmp_file_set_permissions(path, owner_permissions, group_permissions,
            other_permissions, &error_code) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
