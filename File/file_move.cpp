#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

int file_move(const char *source_path, const char *destination_path)
{
    int move_result;
    int delete_error;
    int error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    move_result = cmp_file_move(source_path, destination_path);
    if (move_result == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    error_code = ft_errno;
    if (error_code != FT_ERR_INVALID_OPERATION)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (file_copy(source_path, destination_path) != 0)
        return (-1);
    if (file_delete(source_path) != 0)
    {
        delete_error = ft_errno;
        file_delete(destination_path);
        if (delete_error == FT_ERR_SUCCESSS)
            delete_error = FT_ERR_INTERNAL;
        ft_global_error_stack_push(delete_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
