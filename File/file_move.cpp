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
        return (-1);
    move_result = cmp_file_move(source_path, destination_path, &error_code);
    if (move_result == 0)
        return (0);
    if (error_code != FT_ERR_INVALID_OPERATION)
        return (-1);
    if (file_copy(source_path, destination_path) != 0)
        return (-1);
    if (cmp_file_delete(source_path, &delete_error) != 0)
    {
        cmp_file_delete(destination_path, &cleanup_error);
        return (-1);
    }
    return (0);
}
