#include "file_utils.hpp"
#include <stdint.h>
#include <new>
#include <string>

struct file_path_root_info
{
    uint8_t root_kind;
    char drive_letter;
    std::string unc_server;
    std::string unc_share;
};

static ft_bool file_path_is_drive_letter(char character)
{
    if ((character >= 'A' && character <= 'Z')
        || (character >= 'a' && character <= 'z'))
        return (FT_TRUE);
    return (FT_FALSE);
}

static char file_path_lower_character(char character)
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character - 'A' + 'a'));
    return (character);
}

static file_path_root_info file_path_get_root_info(
    const std::string &path)
{
    file_path_root_info root_info;
    ft_size_t separator_index;
    ft_size_t share_end;

    root_info.root_kind = 0U;
    root_info.drive_letter = '\0';
    if (path.size() >= 2 && file_path_is_drive_letter(path[0]) == FT_TRUE
        && path[1] == ':')
    {
        root_info.drive_letter = file_path_lower_character(path[0]);
        if (path.size() >= 3 && path[2] == '/')
            root_info.root_kind = 2U;
        else
            root_info.root_kind = 3U;
        return (root_info);
    }
    if (path.size() >= 2 && path[0] == '/' && path[1] == '/')
    {
        root_info.root_kind = 4U;
        separator_index = path.find('/', 2);
        if (separator_index == std::string::npos)
            return (root_info);
        root_info.unc_server = path.substr(2, separator_index - 2);
        share_end = path.find('/', separator_index + 1);
        if (share_end == std::string::npos)
            root_info.unc_share = path.substr(separator_index + 1);
        else
            root_info.unc_share = path.substr(separator_index + 1,
                share_end - separator_index - 1);
        return (root_info);
    }
    if (!path.empty() && path[0] == '/')
        root_info.root_kind = 1U;
    return (root_info);
}

static ft_bool file_path_roots_are_compatible(const std::string &from_path,
    const std::string &to_path)
{
    file_path_root_info from_root;
    file_path_root_info to_root;

    from_root = file_path_get_root_info(from_path);
    to_root = file_path_get_root_info(to_path);
    if (from_root.root_kind != to_root.root_kind)
        return (FT_FALSE);
    if (from_root.root_kind == 2U || from_root.root_kind == 3U)
    {
        if (from_root.drive_letter == to_root.drive_letter)
            return (FT_TRUE);
        return (FT_FALSE);
    }
    if (from_root.root_kind == 4U)
    {
        if (from_root.unc_server == to_root.unc_server
            && from_root.unc_share == to_root.unc_share)
            return (FT_TRUE);
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_string *file_path_relative(const char *from_path, const char *to_path)
{
    ft_string *from_normalized;
    ft_string *to_normalized;
    ft_string *result;
    std::string from_text;
    std::string to_text;
    std::string relative_text;
    ft_size_t common_length;
    ft_size_t from_index;
    ft_size_t to_index;

    if (from_path == ft_nullptr || to_path == ft_nullptr)
        return (ft_nullptr);
    from_normalized = file_path_normalize(from_path);
    to_normalized = file_path_normalize(to_path);
    if (from_normalized == ft_nullptr || to_normalized == ft_nullptr)
    {
        if (from_normalized != ft_nullptr)
            delete from_normalized;
        if (to_normalized != ft_nullptr)
            delete to_normalized;
        return (ft_nullptr);
    }
    from_text = from_normalized->c_str();
    to_text = to_normalized->c_str();
    delete from_normalized;
    delete to_normalized;
    from_index = 0;
    while (from_index < from_text.size())
    {
        if (from_text[from_index] == '\\')
            from_text[from_index] = '/';
        from_index++;
    }
    to_index = 0;
    while (to_index < to_text.size())
    {
        if (to_text[to_index] == '\\')
            to_text[to_index] = '/';
        to_index++;
    }
    if (file_path_roots_are_compatible(from_text, to_text) == FT_FALSE)
    {
        return (ft_nullptr);
    }
    if (from_text == to_text)
    {
        result = new (std::nothrow) ft_string();
        if (result == ft_nullptr)
            return (ft_nullptr);
        if (result->initialize(".") != FT_ERR_SUCCESS)
        {
            delete result;
            return (ft_nullptr);
        }
        return (result);
    }
    common_length = 0;
    while (common_length < from_text.size() && common_length < to_text.size()
        && from_text[common_length] == to_text[common_length])
        common_length++;
    if (common_length == from_text.size()
        && to_text.size() > common_length
        && to_text[common_length] == '/')
    {
        to_index = common_length + 1;
        common_length = from_text.size();
        from_index = common_length;
        while (to_index < to_text.size())
        {
            relative_text.push_back(to_text[to_index]);
            to_index++;
        }
        if (relative_text.empty())
            relative_text = ".";
        result = new (std::nothrow) ft_string();
        if (result == ft_nullptr)
            return (ft_nullptr);
        if (result->initialize(relative_text.c_str()) != FT_ERR_SUCCESS)
        {
            delete result;
            return (ft_nullptr);
        }
        return (result);
    }
    while (common_length > 0 && from_text[common_length - 1] != '/')
        common_length--;
    from_index = common_length;
    while (from_index < from_text.size())
    {
        if (from_text[from_index] != '/')
        {
            relative_text.append("../");
            while (from_index < from_text.size()
                && from_text[from_index] != '/')
                from_index++;
        }
        from_index++;
    }
    to_index = common_length;
    while (to_index < to_text.size())
    {
        relative_text.push_back(to_text[to_index]);
        to_index++;
    }
    if (!relative_text.empty() && relative_text.back() == '/')
        relative_text.pop_back();
    if (relative_text.empty())
        relative_text = ".";
    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    if (result->initialize(relative_text.c_str()) != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}
