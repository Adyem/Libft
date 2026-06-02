#include "file_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t file_move(const char *source_path, const char *destination_path)
{
    int32_t move_result;
    int32_t delete_error;
    int32_t cleanup_error;
    int32_t error_code;

    if (source_path == ft_nullptr || destination_path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    move_result = cmp_file_move(source_path, destination_path, &error_code);
    if (move_result == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    if (error_code != FT_ERR_INVALID_OPERATION)
        return (error_code);
    if (file_copy(source_path, destination_path) != FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    if (cmp_file_delete(source_path, &delete_error) != FT_ERR_SUCCESS)
    {
        cmp_file_delete(destination_path, &cleanup_error);
        return (FT_ERR_IO);
    }
    return (FT_ERR_SUCCESS);
}
