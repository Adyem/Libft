#include "filesystem.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../File/file_utils.hpp"
#include "../File/open_dir.hpp"

static ft_bool filesystem_is_dot_entry(const char *name) noexcept
{
    if (name == ft_nullptr)
    {
        return (FT_FALSE);
    }
    if (name[0] == '.' && name[1] == '\0')
    {
        return (FT_TRUE);
    }
    if (name[0] == '.' && name[1] == '.' && name[2] == '\0')
    {
        return (FT_TRUE);
    }
    return (FT_FALSE);
}

static void filesystem_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

static int32_t filesystem_walk_child(const char *path,
    filesystem_walk_callback callback, void *user_context)
{
    ft_bool is_directory;
    int32_t callback_error;

    is_directory = FT_FALSE;
    if (file_dir_exists(path) == FT_ERR_SUCCESS)
    {
        is_directory = FT_TRUE;
    }
    callback_error = callback(path, is_directory, user_context);
    if (callback_error != FT_ERR_SUCCESS)
    {
        return (callback_error);
    }
    if (is_directory == FT_TRUE)
    {
        return (filesystem_walk_recursive(path, callback, user_context));
    }
    return (FT_ERR_SUCCESS);
}

int32_t filesystem_walk_recursive(const char *root_path,
    filesystem_walk_callback callback, void *user_context)
{
    file_dir *directory_stream;
    file_dirent *entry;
    ft_string *child_path;
    int32_t walk_error;
    int32_t close_error;

    if (root_path == ft_nullptr || callback == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    directory_stream = file_opendir(root_path);
    if (directory_stream == ft_nullptr)
    {
        return (FT_ERR_FILE_OPEN_FAILED);
    }
    walk_error = FT_ERR_SUCCESS;
    entry = file_readdir(directory_stream);
    while (entry != ft_nullptr)
    {
        if (filesystem_is_dot_entry(entry->d_name) == FT_FALSE)
        {
            child_path = file_path_join(root_path, entry->d_name);
            if (child_path == ft_nullptr)
            {
                walk_error = FT_ERR_INVALID_STATE;
                break ;
            }
            if (child_path->get_error() != FT_ERR_SUCCESS)
            {
                walk_error = child_path->get_error();
                filesystem_delete_string(child_path);
                break ;
            }
            walk_error = filesystem_walk_child(child_path->c_str(), callback,
                    user_context);
            filesystem_delete_string(child_path);
            if (walk_error != FT_ERR_SUCCESS)
            {
                break ;
            }
        }
        entry = file_readdir(directory_stream);
    }
    close_error = file_closedir(directory_stream);
    if (walk_error != FT_ERR_SUCCESS)
    {
        return (walk_error);
    }
    return (close_error);
}
