#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

int file_move(const char *source_path, const char *destination_path)
{
    int move_result;
    int delete_error;
    int cleanup_error;
    int error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    move_result = cmp_file_move(source_path, destination_path, &error_code);
    if (move_result == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (0);
    }
    if (error_code != FT_ERR_INVALID_OPERATION)
    {
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (file_copy(source_path, destination_path) != 0)
        return (-1);
    if (cmp_file_delete(source_path, &delete_error) != 0)
    {
        cmp_file_delete(destination_path, &cleanup_error);
        if (delete_error == FT_ERR_SUCCESS)
            delete_error = FT_ERR_INTERNAL;
        ft_global_error_stack_push(delete_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}
