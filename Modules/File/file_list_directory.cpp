#include "file_utils.hpp"
#include "open_dir.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static ft_bool file_list_is_dot_entry(const char *name) noexcept
{
    if (name == ft_nullptr)
        return (FT_FALSE);
    if (name[0] == '.' && name[1] == '\0')
        return (FT_TRUE);
    if (name[0] == '.' && name[1] == '.' && name[2] == '\0')
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_bool file_list_is_hidden_entry(const char *name) noexcept
{
    if (name == ft_nullptr)
        return (FT_FALSE);
    if (name[0] == '.' && file_list_is_dot_entry(name) == FT_FALSE)
        return (FT_TRUE);
    return (FT_FALSE);
}

static void file_list_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

int32_t file_readdir_string(file_dir *directory_stream, ft_string *entry_name)
{
    file_dirent *entry;
    ft_size_t name_length;

    if (directory_stream == ft_nullptr || entry_name == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    entry = file_readdir(directory_stream);
    if (entry == ft_nullptr)
        return (FT_ERR_END_OF_FILE);
    name_length = ft_strlen_size_t(entry->d_name);
    if (name_length >= sizeof(entry->d_name) - 1)
        return (FT_ERR_OUT_OF_RANGE);
    return (entry_name->assign(entry->d_name, name_length));
}

static ft_bool file_list_should_include(file_type type_value, uint32_t flags) noexcept
{
    if (type_value == FILE_TYPE_DIRECTORY
        && (flags & FILE_LIST_INCLUDE_DIRECTORIES) != 0)
        return (FT_TRUE);
    if (type_value == FILE_TYPE_REGULAR
        && (flags & FILE_LIST_INCLUDE_FILES) != 0)
        return (FT_TRUE);
    if (type_value == FILE_TYPE_SYMLINK
        && (flags & FILE_LIST_FOLLOW_SYMLINKS) == 0
        && (flags & FILE_LIST_INCLUDE_FILES) != 0)
        return (FT_TRUE);
    return (FT_FALSE);
}

static int32_t file_list_push_path(ft_vector<ft_string> &entries,
    const char *path)
{
    ft_string entry_value;
    int32_t error_code;

    error_code = entry_value.initialize(path);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    error_code = entries.push_back(entry_value);
    (void)entry_value.destroy();
    return (error_code);
}

static int32_t file_list_directory_internal(const char *path,
    ft_vector<ft_string> &entries, uint32_t flags, ft_bool recursive)
{
    file_dir *directory_stream;
    ft_string entry_name;
    ft_string *child_path;
    file_type child_type;
    int32_t error_code;
    int32_t read_error;
    int32_t close_error;

    if (path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (file_get_type(path) != FILE_TYPE_DIRECTORY)
        return (FT_ERR_NOT_DIRECTORY);
    directory_stream = file_opendir(path);
    if (directory_stream == ft_nullptr)
        return (FT_ERR_FILE_OPEN_FAILED);
    error_code = entry_name.initialize();
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)file_closedir(directory_stream);
        return (error_code);
    }
    read_error = file_readdir_string(directory_stream, &entry_name);
    while (read_error == FT_ERR_SUCCESS)
    {
        if (file_list_is_dot_entry(entry_name.c_str()) == FT_FALSE
            && ((flags & FILE_LIST_INCLUDE_HIDDEN) != 0
                || file_list_is_hidden_entry(entry_name.c_str()) == FT_FALSE))
        {
            child_path = ft_nullptr;
            child_path = file_path_join(path, entry_name.c_str());
            if (child_path == ft_nullptr)
                error_code = FT_ERR_NO_MEMORY;
            else if (child_path->get_error() != FT_ERR_SUCCESS)
                error_code = child_path->get_error();
            else
            {
                child_type = file_get_type(child_path->c_str());
                if (file_list_should_include(child_type, flags) == FT_TRUE)
                    error_code = file_list_push_path(entries, child_path->c_str());
                if (error_code == FT_ERR_SUCCESS && recursive == FT_TRUE
                    && child_type == FILE_TYPE_DIRECTORY)
                    error_code = file_list_directory_internal(child_path->c_str(),
                            entries, flags, recursive);
            }
            file_list_delete_string(child_path);
            if (error_code != FT_ERR_SUCCESS)
                break ;
        }
        (void)entry_name.clear();
        read_error = file_readdir_string(directory_stream, &entry_name);
    }
    (void)entry_name.destroy();
    close_error = file_closedir(directory_stream);
    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    if (read_error != FT_ERR_END_OF_FILE)
        return (read_error);
    return (close_error);
}

int32_t file_list_directory(const char *path, ft_vector<ft_string> &entries,
    uint32_t flags)
{
    return (file_list_directory_internal(path, entries, flags, FT_FALSE));
}

int32_t file_list_directory_recursive(const char *path,
    ft_vector<ft_string> &entries, uint32_t flags)
{
    return (file_list_directory_internal(path, entries, flags, FT_TRUE));
}
