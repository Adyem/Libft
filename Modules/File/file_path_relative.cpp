#include "file_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include <new>
#include <stdint.h>

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

static ft_size_t file_path_length(const char *path)
{
    ft_size_t length;

    length = 0;
    while (path[length] != '\0')
        length++;
    return (length);
}

static ft_bool file_path_range_equal_case_insensitive(const char *left,
    ft_size_t left_start, const char *right, ft_size_t right_start,
    ft_size_t length)
{
    ft_size_t index;

    index = 0;
    while (index < length)
    {
        if (file_path_lower_character(left[left_start + index])
            != file_path_lower_character(right[right_start + index]))
            return (FT_FALSE);
        index++;
    }
    return (FT_TRUE);
}

static uint8_t file_path_root_kind(const char *path)
{
    ft_size_t length;

    length = file_path_length(path);
    if (length >= 2 && file_path_is_drive_letter(path[0]) == FT_TRUE
        && path[1] == ':')
    {
        if (length >= 3 && path[2] == '/')
            return (2U);
        return (3U);
    }
    if (length >= 2 && path[0] == '/' && path[1] == '/')
        return (4U);
    if (length >= 1 && path[0] == '/')
        return (1U);
    return (0U);
}

static ft_bool file_path_roots_are_compatible(const char *from_path,
    const char *to_path)
{
    uint8_t from_kind;
    uint8_t to_kind;
    ft_size_t from_length;
    ft_size_t to_length;
    ft_size_t from_server_end;
    ft_size_t to_server_end;
    ft_size_t from_share_end;
    ft_size_t to_share_end;

    from_kind = file_path_root_kind(from_path);
    to_kind = file_path_root_kind(to_path);
    if (from_kind != to_kind)
        return (FT_FALSE);
    if (from_kind == 2U || from_kind == 3U)
    {
        if (file_path_lower_character(from_path[0])
            == file_path_lower_character(to_path[0]))
            return (FT_TRUE);
        return (FT_FALSE);
    }
    if (from_kind != 4U)
        return (FT_TRUE);
    from_length = file_path_length(from_path);
    to_length = file_path_length(to_path);
    from_server_end = 2;
    while (from_server_end < from_length && from_path[from_server_end] != '/')
        from_server_end++;
    to_server_end = 2;
    while (to_server_end < to_length && to_path[to_server_end] != '/')
        to_server_end++;
    if (from_server_end == 2 || to_server_end == 2
        || from_server_end == from_length || to_server_end == to_length)
        return (FT_FALSE);
    from_share_end = from_server_end + 1;
    while (from_share_end < from_length && from_path[from_share_end] != '/')
        from_share_end++;
    to_share_end = to_server_end + 1;
    while (to_share_end < to_length && to_path[to_share_end] != '/')
        to_share_end++;
    if (from_share_end == from_server_end + 1
        || to_share_end == to_server_end + 1)
        return (FT_FALSE);
    if (from_server_end - 2 != to_server_end - 2
        || from_share_end - from_server_end != to_share_end - to_server_end)
        return (FT_FALSE);
    if (file_path_range_equal_case_insensitive(from_path, 2, to_path, 2,
            from_server_end - 2) == FT_FALSE)
        return (FT_FALSE);
    if (file_path_range_equal_case_insensitive(from_path, from_server_end + 1,
            to_path, to_server_end + 1, from_share_end - from_server_end - 1)
        == FT_FALSE)
        return (FT_FALSE);
    return (FT_TRUE);
}

static int32_t file_path_collect_components(const ft_string &path,
    ft_vector<ft_string> &components, ft_size_t component_index_to_skip)
{
    const char *data;
    ft_size_t data_length;
    ft_size_t index;
    ft_size_t segment_start;
    ft_size_t component_index;
    ft_string component;
    int32_t error_code;

    data = path.c_str();
    data_length = path.size();
    index = 0;
    component_index = 0;
    while (index < data_length)
    {
        while (index < data_length && data[index] == '/')
            index++;
        segment_start = index;
        while (index < data_length && data[index] != '/')
            index++;
        if (index == segment_start)
            break ;
        if (component_index >= component_index_to_skip)
        {
            error_code = component.initialize();
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
            error_code = component.append(data + segment_start,
                index - segment_start);
            if (error_code == FT_ERR_SUCCESS)
                error_code = components.push_back(component);
            (void)component.destroy();
            if (error_code != FT_ERR_SUCCESS)
                return (error_code);
        }
        component_index++;
    }
    return (FT_ERR_SUCCESS);
}

static ft_bool file_path_components_equal(const ft_string &left,
    const ft_string &right, ft_bool case_insensitive)
{
    ft_size_t index;

    if (left.size() != right.size())
        return (FT_FALSE);
    index = 0;
    while (index < left.size())
    {
        if (case_insensitive == FT_TRUE)
        {
            if (file_path_lower_character(left.c_str()[index])
                != file_path_lower_character(right.c_str()[index]))
                return (FT_FALSE);
        }
        else if (left.c_str()[index] != right.c_str()[index])
            return (FT_FALSE);
        index++;
    }
    return (FT_TRUE);
}

static void file_path_convert_backslashes(ft_string &path)
{
    char *data;
    ft_size_t index;

    data = path.print();
    if (data == ft_nullptr)
        return ;
    index = 0;
    while (index < path.size())
    {
        if (data[index] == '\\')
            data[index] = '/';
        index++;
    }
    return ;
}

ft_string *file_path_relative(const char *from_path, const char *to_path)
{
    ft_string *from_normalized;
    ft_string *to_normalized;
    ft_string *result;
    ft_string relative_text;
    ft_vector<ft_string> from_components;
    ft_vector<ft_string> to_components;
    ft_size_t common_component_count;
    ft_size_t index;
    uint8_t root_kind;
    ft_bool case_insensitive;
    int32_t error_code;

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
    file_path_convert_backslashes(*from_normalized);
    file_path_convert_backslashes(*to_normalized);
    if (file_path_roots_are_compatible(from_normalized->c_str(),
            to_normalized->c_str()) == FT_FALSE)
    {
        delete from_normalized;
        delete to_normalized;
        return (ft_nullptr);
    }
    error_code = from_components.initialize();
    if (error_code == FT_ERR_SUCCESS)
        error_code = to_components.initialize();
    if (error_code == FT_ERR_SUCCESS)
    {
        root_kind = file_path_root_kind(from_normalized->c_str());
        error_code = file_path_collect_components(*from_normalized,
            from_components, root_kind == 4U ? 2 :
            ((root_kind == 2U || root_kind == 3U) ? 1 : 0));
    }
    if (error_code == FT_ERR_SUCCESS)
        error_code = file_path_collect_components(*to_normalized,
            to_components, root_kind == 4U ? 2 :
            ((root_kind == 2U || root_kind == 3U) ? 1 : 0));
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)from_components.destroy();
        (void)to_components.destroy();
        delete from_normalized;
        delete to_normalized;
        return (ft_nullptr);
    }
    case_insensitive = root_kind >= 2U ? FT_TRUE : FT_FALSE;
    common_component_count = 0;
    while (common_component_count < from_components.size()
        && common_component_count < to_components.size())
    {
        if (file_path_components_equal(from_components[common_component_count],
                to_components[common_component_count], case_insensitive)
            == FT_FALSE)
            break ;
        common_component_count++;
    }
    error_code = relative_text.initialize();
    index = common_component_count;
    while (error_code == FT_ERR_SUCCESS && index < from_components.size())
    {
        error_code = relative_text.append("../");
        index++;
    }
    index = common_component_count;
    while (error_code == FT_ERR_SUCCESS && index < to_components.size())
    {
        if (relative_text.size() > 0 && relative_text.c_str()[relative_text.size() - 1] != '/')
            error_code = relative_text.append('/');
        if (error_code == FT_ERR_SUCCESS)
            error_code = relative_text.append(to_components[index].c_str());
        index++;
    }
    if (error_code == FT_ERR_SUCCESS && relative_text.size() == 0)
        error_code = relative_text.append('.');
    result = ft_nullptr;
    if (error_code == FT_ERR_SUCCESS)
    {
        result = new (std::nothrow) ft_string();
        if (result != ft_nullptr && result->initialize(relative_text.c_str())
            != FT_ERR_SUCCESS)
        {
            delete result;
            result = ft_nullptr;
        }
    }
    (void)relative_text.destroy();
    (void)from_components.destroy();
    (void)to_components.destroy();
    delete from_normalized;
    delete to_normalized;
    return (result);
}
