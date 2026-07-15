#include "file_utils.hpp"
#include <new>
#include <string>

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
