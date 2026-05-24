#include "filesystem.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <new>

static ft_string *filesystem_string_from_owned_c_string(char *value)
{
    ft_string *result;

    if (value == ft_nullptr)
        return (ft_nullptr);
    result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
    {
        cma_free(value);
        return (ft_nullptr);
    }
    if (result->initialize(value) != FT_ERR_SUCCESS)
    {
        cma_free(value);
        delete result;
        return (ft_nullptr);
    }
    cma_free(value);
    return (result);
}

ft_string *filesystem_canonical_path(const char *path)
{
    char *canonical_path;
    int32_t error_code;

    canonical_path = ft_nullptr;
    error_code = cmp_path_canonical(path, &canonical_path);
    if (error_code != FT_ERR_SUCCESS)
        return (ft_nullptr);
    return (filesystem_string_from_owned_c_string(canonical_path));
}
