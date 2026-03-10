#include "optional.hpp"
#include <cstdint>

static int32_t check_optional_usage()
{
    ft_optional<int32_t> initial_value(21);
    ft_optional<int32_t> moved_value;
    (void)moved_value.initialize(ft_move(initial_value.value()));
    initial_value.reset();

    if (moved_value.has_value())
    {
        int32_t result = moved_value.value();
        (void)result;
    }
    moved_value.reset();
    return (static_cast<int32_t>(initial_value.has_value())
            + static_cast<int32_t>(moved_value.has_value()));
}

static volatile int32_t template_optional_result = check_optional_usage();
