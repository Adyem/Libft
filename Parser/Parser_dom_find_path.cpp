#include "dom.hpp"
#include "../CPP_class/class_nullptr.hpp"

int ft_dom_find_path(const ft_dom_node *root, const ft_string &path, const ft_dom_node **out_node) noexcept
{

    if (!root || !out_node)
        return (FT_ERR_INVALID_ARGUMENT);
    *out_node = ft_nullptr;
    const char *raw_path;

    raw_path = path.c_str();
    if (!raw_path)
        return (FT_ERR_INVALID_ARGUMENT);
    size_t path_length;

    path_length = path.size();
    if (path_length == 0)
    {
        *out_node = root;
        return (FT_ERR_SUCCESS);
    }
    const ft_dom_node *current_node;
    size_t start_index;
    int status;
    int error_code;

    current_node = root;
    start_index = 0;
    status = 0;
    error_code = FT_ERR_SUCCESS;
    while (start_index < path_length && status == 0)
    {
        size_t end_index;
        size_t segment_length;

        end_index = start_index;
        while (end_index < path_length && raw_path[end_index] != '/')
            end_index += 1;
        segment_length = end_index - start_index;
        if (segment_length == 0)
        {
            status = -1;
            error_code = FT_ERR_INVALID_ARGUMENT;
        }
        else
        {
            ft_string segment;

            segment.assign(raw_path + start_index, segment_length);
            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            {
                status = -1;
                error_code = ft_string::last_operation_error();
            }
            else
            {
                ft_dom_node *child_node;

                child_node = current_node->find_child(segment);
                if (!child_node)
                {
                    status = -1;
                    error_code = FT_ERR_NOT_FOUND;
                }
                else
                {
                    current_node = child_node;
                    if (end_index >= path_length)
                        start_index = path_length;
                    else
                        start_index = end_index + 1;
                }
            }
        }
    }
    if (status != 0)
        return (error_code);
    *out_node = current_node;
    return (FT_ERR_SUCCESS);
}
