#include "file_utils.hpp"
#include "open_dir.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif

static ft_bool file_recursive_is_dot_entry(const char *name) noexcept
{
    if (name == ft_nullptr)
        return (FT_FALSE);
    if (name[0] == '.' && name[1] == '\0')
        return (FT_TRUE);
    if (name[0] == '.' && name[1] == '.' && name[2] == '\0')
        return (FT_TRUE);
    return (FT_FALSE);
}

static void file_recursive_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

static int32_t file_recursive_remove_directory(const char *path) noexcept
{
#if defined(_WIN32) || defined(_WIN64)
    if (RemoveDirectoryA(path) == 0)
        return (FT_ERR_IO);
#else
    if (rmdir(path) != 0)
        return (FT_ERR_IO);
#endif
    return (FT_ERR_SUCCESS);
}

static int32_t file_recursive_create_component(char *path_buffer,
    ft_size_t component_end, mode_t mode)
{
    char saved_character;
    file_type type_value;
    int32_t error_code;

    saved_character = path_buffer[component_end];
    path_buffer[component_end] = '\0';
    if (path_buffer[0] != '\0')
    {
        error_code = file_create_directory(path_buffer, mode);
        if (error_code == FT_ERR_ALREADY_EXISTS)
        {
            type_value = file_get_type(path_buffer);
            if (type_value != FILE_TYPE_DIRECTORY)
            {
                path_buffer[component_end] = saved_character;
                return (FT_ERR_ALREADY_EXISTS);
            }
        }
        else if (error_code != FT_ERR_SUCCESS)
        {
            path_buffer[component_end] = saved_character;
            return (error_code);
        }
    }
    path_buffer[component_end] = saved_character;
    return (FT_ERR_SUCCESS);
}

int32_t file_create_directories(const char *path, mode_t mode)
{
    ft_string *normalized_path;
    char *path_buffer;
    ft_size_t index;
    int32_t error_code;

    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    normalized_path = file_path_normalize(path);
    if (normalized_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (normalized_path->get_error() != FT_ERR_SUCCESS)
    {
        error_code = normalized_path->get_error();
        file_recursive_delete_string(normalized_path);
        return (error_code);
    }
    path_buffer = normalized_path->print();
    if (path_buffer == ft_nullptr)
    {
        file_recursive_delete_string(normalized_path);
        return (FT_ERR_INVALID_STATE);
    }
    index = 0;
    while (path_buffer[index] != '\0')
    {
        if ((path_buffer[index] == '/' || path_buffer[index] == '\\') && index > 0
            && path_buffer[index - 1] != ':')
        {
            error_code = file_recursive_create_component(path_buffer, index, mode);
            if (error_code != FT_ERR_SUCCESS)
            {
                file_recursive_delete_string(normalized_path);
                return (error_code);
            }
        }
        ++index;
    }
    error_code = file_recursive_create_component(path_buffer, index, mode);
    file_recursive_delete_string(normalized_path);
    if (error_code == FT_ERR_ALREADY_EXISTS)
        return (FT_ERR_SUCCESS);
    return (error_code);
}

int32_t file_delete_recursive(const char *path)
{
    file_type type_value;
    file_dir *directory_stream;
    file_dirent *entry;
    ft_string *child_path;
    int32_t error_code;
    int32_t close_error;

    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    type_value = file_get_type(path);
    if (type_value == FILE_TYPE_MISSING)
        return (FT_ERR_SUCCESS);
    if (type_value == FILE_TYPE_REGULAR || type_value == FILE_TYPE_SYMLINK)
        return (file_delete(path));
    if (type_value != FILE_TYPE_DIRECTORY)
        return (FT_ERR_UNSUPPORTED_TYPE);
    directory_stream = file_opendir(path);
    if (directory_stream == ft_nullptr)
        return (FT_ERR_FILE_OPEN_FAILED);
    error_code = FT_ERR_SUCCESS;
    entry = file_readdir(directory_stream);
    while (entry != ft_nullptr)
    {
        if (file_recursive_is_dot_entry(entry->d_name) == FT_FALSE)
        {
            child_path = file_path_join(path, entry->d_name);
            if (child_path == ft_nullptr)
            {
                error_code = FT_ERR_NO_MEMORY;
                break ;
            }
            if (child_path->get_error() != FT_ERR_SUCCESS)
                error_code = child_path->get_error();
            else
                error_code = file_delete_recursive(child_path->c_str());
            file_recursive_delete_string(child_path);
            if (error_code != FT_ERR_SUCCESS)
                break ;
        }
        entry = file_readdir(directory_stream);
    }
    close_error = file_closedir(directory_stream);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (close_error != FT_ERR_SUCCESS)
        return (close_error);
    return (file_recursive_remove_directory(path));
}

static int32_t file_copy_directory_entry(const char *source_path,
    const char *destination_path, const char *entry_name)
{
    ft_string *source_child;
    ft_string *destination_child;
    file_type source_type;
    int32_t error_code;

    source_child = file_path_join(source_path, entry_name);
    if (source_child == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (source_child->get_error() != FT_ERR_SUCCESS)
    {
        error_code = source_child->get_error();
        file_recursive_delete_string(source_child);
        return (error_code);
    }
    destination_child = file_path_join(destination_path, entry_name);
    if (destination_child == ft_nullptr)
    {
        file_recursive_delete_string(source_child);
        return (FT_ERR_NO_MEMORY);
    }
    source_type = file_get_type(source_child->c_str());
    if (destination_child->get_error() != FT_ERR_SUCCESS)
        error_code = destination_child->get_error();
    else if (source_type == FILE_TYPE_DIRECTORY)
        error_code = file_copy_directory(source_child->c_str(), destination_child->c_str());
    else if (source_type == FILE_TYPE_REGULAR)
        error_code = file_copy(source_child->c_str(), destination_child->c_str());
    else
        error_code = FT_ERR_UNSUPPORTED_TYPE;
    file_recursive_delete_string(destination_child);
    file_recursive_delete_string(source_child);
    return (error_code);
}

int32_t file_copy_directory(const char *source_path, const char *destination_path)
{
    file_type source_type;
    file_type destination_type;
    file_dir *directory_stream;
    file_dirent *entry;
    int32_t error_code;
    int32_t close_error;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    source_type = file_get_type(source_path);
    if (source_type != FILE_TYPE_DIRECTORY)
        return (FT_ERR_INVALID_ARGUMENT);
    destination_type = file_get_type(destination_path);
    if (destination_type == FILE_TYPE_MISSING)
        error_code = file_create_directory(destination_path, 0755);
    else if (destination_type == FILE_TYPE_DIRECTORY)
        error_code = FT_ERR_SUCCESS;
    else
        error_code = FT_ERR_ALREADY_EXISTS;
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    directory_stream = file_opendir(source_path);
    if (directory_stream == ft_nullptr)
        return (FT_ERR_FILE_OPEN_FAILED);
    error_code = FT_ERR_SUCCESS;
    entry = file_readdir(directory_stream);
    while (entry != ft_nullptr)
    {
        if (file_recursive_is_dot_entry(entry->d_name) == FT_FALSE)
        {
            error_code = file_copy_directory_entry(source_path, destination_path,
                    entry->d_name);
            if (error_code != FT_ERR_SUCCESS)
                break ;
        }
        entry = file_readdir(directory_stream);
    }
    close_error = file_closedir(directory_stream);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (close_error);
}
