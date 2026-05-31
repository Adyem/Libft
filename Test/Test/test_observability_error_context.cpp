#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Observability/observability.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_observability_error_context_set_and_clear)
{
    ft_error_context context;

    observability_error_context_clear(&context);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.error_code);
    FT_ASSERT_EQ(0, context.platform_error);
    FT_ASSERT_EQ(ft_nullptr, context.module);

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_error_context_set(&context, "File", "read_all",
            "config.json", FT_ERR_IO, "short read", 5));
    FT_ASSERT_EQ(FT_ERR_IO, context.error_code);
    FT_ASSERT_EQ(5, context.platform_error);
    FT_ASSERT_EQ("File", context.module);
    FT_ASSERT_EQ("read_all", context.operation);
    FT_ASSERT_EQ("config.json", context.resource);
    FT_ASSERT_EQ("short read", context.detail);

    observability_error_context_clear(&context);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, context.error_code);
    FT_ASSERT_EQ(ft_nullptr, context.operation);
    return (1);
}

FT_TEST(test_observability_error_context_rejects_invalid_arguments)
{
    ft_error_context context;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        observability_error_context_set(ft_nullptr, "File", "read_all",
            ft_nullptr, FT_ERR_IO, ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        observability_error_context_set(&context, ft_nullptr, "read_all",
            ft_nullptr, FT_ERR_IO, ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        observability_error_context_set(&context, "File", ft_nullptr,
            ft_nullptr, FT_ERR_IO, ft_nullptr, 0));
    return (1);
}
