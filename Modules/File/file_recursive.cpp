#include "file_utils.hpp"
#include "open_dir.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
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

static ft_string *file_recursive_normalize_system_path(const char *path)
{
    ft_string *normalized_path;

    normalized_path = file_path_normalize(path);
    if (normalized_path == ft_nullptr)
        return (ft_nullptr);
    if (normalized_path->get_error() != FT_ERR_SUCCESS)
        return (normalized_path);
#if defined(_WIN32) || defined(_WIN64)
    char *path_buffer;
    ft_size_t index;

    path_buffer = normalized_path->print();
    if (path_buffer != ft_nullptr)
    {
        index = 0;
        while (path_buffer[index] != '\0')
        {
            if (path_buffer[index] == '/')
                path_buffer[index] = '\\';
            ++index;
        }
    }
#endif
    return (normalized_path);
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
    ft_string *normalized_path;
    const char *system_path;
    file_type type_value;
    file_dir *directory_stream;
    file_dirent *entry;
    ft_string *child_path;
    int32_t error_code;
    int32_t close_error;

    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    normalized_path = file_recursive_normalize_system_path(path);
    if (normalized_path == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    if (normalized_path->get_error() != FT_ERR_SUCCESS)
    {
        error_code = normalized_path->get_error();
        file_recursive_delete_string(normalized_path);
        return (error_code);
    }
    system_path = normalized_path->c_str();
    type_value = file_get_type(system_path);
    if (type_value == FILE_TYPE_MISSING)
    {
        file_recursive_delete_string(normalized_path);
        return (FT_ERR_SUCCESS);
    }
    if (type_value == FILE_TYPE_REGULAR || type_value == FILE_TYPE_SYMLINK)
    {
        error_code = FT_ERR_INVALID_OPERATION;
        ft_size_t retry_index;

        retry_index = 0;
        while (retry_index < 20U)
        {
            error_code = file_delete(system_path);
            if (error_code != FT_ERR_INVALID_OPERATION
                && error_code != FT_ERR_PERMISSION_DENIED)
                break ;
#if defined(_WIN32) || defined(_WIN64)
            Sleep(100);
#else
            usleep(100000);
#endif
            retry_index++;
        }
        file_recursive_delete_string(normalized_path);
        return (error_code);
    }
    if (type_value != FILE_TYPE_DIRECTORY)
    {
        file_recursive_delete_string(normalized_path);
        return (FT_ERR_UNSUPPORTED_TYPE);
    }
    directory_stream = file_opendir(system_path);
    if (directory_stream == ft_nullptr)
    {
        file_recursive_delete_string(normalized_path);
        return (FT_ERR_FILE_OPEN_FAILED);
    }
    error_code = FT_ERR_SUCCESS;
    entry = file_readdir(directory_stream);
    while (entry != ft_nullptr)
    {
        if (file_recursive_is_dot_entry(entry->d_name) == FT_FALSE)
        {
            child_path = file_path_join(system_path, entry->d_name);
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
            {
                break ;
            }
        }
        entry = file_readdir(directory_stream);
    }
    close_error = file_closedir(directory_stream);
    if (error_code != FT_ERR_SUCCESS)
    {
        file_recursive_delete_string(normalized_path);
        return (error_code);
    }
    if (close_error != FT_ERR_SUCCESS)
    {
        file_recursive_delete_string(normalized_path);
        return (close_error);
    }
    error_code = file_recursive_remove_directory(system_path);
    file_recursive_delete_string(normalized_path);
    return (error_code);
}

static ft_bool file_recursive_should_include(file_path_filter_callback filter_callback,
    const char *path, ft_bool is_directory, void *user_context) noexcept
{
    if (filter_callback == ft_nullptr)
        return (FT_TRUE);
    return (filter_callback(path, is_directory, user_context));
}

static int32_t file_recursive_prepare_destination_directory(const char *destination_path)
{
    file_type destination_type;

    destination_type = file_get_type(destination_path);
    if (destination_type == FILE_TYPE_UNKNOWN)
        return (FT_ERR_IO);
    if (destination_type == FILE_TYPE_MISSING)
        return (file_create_directory(destination_path, 0755));
    if (destination_type == FILE_TYPE_DIRECTORY)
        return (FT_ERR_SUCCESS);
    return (FT_ERR_ALREADY_EXISTS);
}

static int32_t file_recursive_transfer_directory(const char *source_path,
    const char *destination_path, file_path_filter_callback filter_callback,
    void *user_context, ft_bool move_sources)
{
    file_dir *directory_stream;
    file_dirent *entry;
    ft_string *source_child;
    ft_string *destination_child;
    file_type source_type;
    ft_bool include_child;
    int32_t error_code;
    int32_t close_error;

    directory_stream = file_opendir(source_path);
    if (directory_stream == ft_nullptr)
        return (FT_ERR_FILE_OPEN_FAILED);
    error_code = FT_ERR_SUCCESS;
    entry = file_readdir(directory_stream);
    while (entry != ft_nullptr)
    {
        if (file_recursive_is_dot_entry(entry->d_name) == FT_FALSE)
        {
            source_child = file_path_join(source_path, entry->d_name);
            if (source_child == ft_nullptr)
            {
                error_code = FT_ERR_NO_MEMORY;
                break ;
            }
            if (source_child->get_error() != FT_ERR_SUCCESS)
                error_code = source_child->get_error();
            else
            {
                source_type = file_get_type(source_child->c_str());
                if (source_type == FILE_TYPE_UNKNOWN)
                    error_code = FT_ERR_IO;
                else
                {
                    include_child = file_recursive_should_include(filter_callback,
                            source_child->c_str(),
                            source_type == FILE_TYPE_DIRECTORY, user_context);
                    if (include_child == FT_TRUE)
                    {
                        destination_child = file_path_join(destination_path,
                                entry->d_name);
                        if (destination_child == ft_nullptr)
                        {
                            error_code = FT_ERR_NO_MEMORY;
                        }
                        else if (destination_child->get_error() != FT_ERR_SUCCESS)
                        {
                            error_code = destination_child->get_error();
                        }
                        else if (source_type == FILE_TYPE_DIRECTORY)
                        {
                            error_code = file_recursive_prepare_destination_directory(
                                    destination_child->c_str());
                            if (error_code == FT_ERR_SUCCESS)
                            {
                                error_code = file_recursive_transfer_directory(
                                        source_child->c_str(),
                                        destination_child->c_str(),
                                        filter_callback, user_context,
                                        move_sources);
                                if (error_code == FT_ERR_SUCCESS
                                    && move_sources == FT_TRUE)
                                    (void)file_delete(source_child->c_str());
                            }
                        }
                        else if (source_type == FILE_TYPE_REGULAR)
                        {
                            if (move_sources == FT_TRUE)
                                error_code = file_move(source_child->c_str(),
                                        destination_child->c_str());
                            else
                                error_code = file_copy(source_child->c_str(),
                                        destination_child->c_str());
                        }
                        else if (source_type == FILE_TYPE_SYMLINK)
                        {
                            if (move_sources == FT_TRUE)
                                error_code = file_move(source_child->c_str(),
                                        destination_child->c_str());
                            else
                                error_code = FT_ERR_UNSUPPORTED_TYPE;
                        }
                        else
                        {
                            error_code = FT_ERR_UNSUPPORTED_TYPE;
                        }
                        file_recursive_delete_string(destination_child);
                    }
                }
            }
            file_recursive_delete_string(source_child);
            if (error_code != FT_ERR_SUCCESS)
                break ;
        }
        entry = file_readdir(directory_stream);
    }
    if (error_code == FT_ERR_SUCCESS && move_sources == FT_TRUE)
        (void)file_delete(source_path);
    close_error = file_closedir(directory_stream);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (close_error != FT_ERR_SUCCESS)
        return (close_error);
    return (FT_ERR_SUCCESS);
}

int32_t file_copy_directory(const char *source_path, const char *destination_path)
{
    return (file_copy_directory_filtered(source_path, destination_path,
            ft_nullptr, ft_nullptr));
}

int32_t file_copy_directory_filtered(const char *source_path,
    const char *destination_path, file_path_filter_callback filter_callback,
    void *user_context)
{
    file_type source_type;
    int32_t error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    source_type = file_get_type(source_path);
    if (source_type != FILE_TYPE_DIRECTORY)
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_recursive_should_include(filter_callback, source_path, FT_TRUE,
            user_context) == FT_FALSE)
        return (FT_ERR_SUCCESS);
    if (file_path_equal(source_path, destination_path) == FT_TRUE)
        return (FT_ERR_SUCCESS);
    if (file_path_is_inside_root(source_path, destination_path) == FT_TRUE)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = file_recursive_prepare_destination_directory(destination_path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (file_recursive_transfer_directory(source_path, destination_path,
            filter_callback, user_context, FT_FALSE));
}

int32_t file_move_directory_filtered(const char *source_path,
    const char *destination_path, file_path_filter_callback filter_callback,
    void *user_context)
{
    file_type source_type;
    int32_t error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    source_type = file_get_type(source_path);
    if (source_type != FILE_TYPE_DIRECTORY)
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_recursive_should_include(filter_callback, source_path, FT_TRUE,
            user_context) == FT_FALSE)
        return (FT_ERR_SUCCESS);
    if (file_path_equal(source_path, destination_path) == FT_TRUE)
        return (FT_ERR_SUCCESS);
    if (file_path_is_inside_root(source_path, destination_path) == FT_TRUE)
        return (FT_ERR_INVALID_ARGUMENT);
    error_code = file_recursive_prepare_destination_directory(destination_path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (file_recursive_transfer_directory(source_path, destination_path,
            filter_callback, user_context, FT_TRUE));
}
