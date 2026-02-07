#include "optional.hpp"

namespace template_optional_compile
{
static int check_optional_usage()
{
    ft_optional<int> initial_value(21);
    ft_optional<int> moved_value(ft_move(initial_value));

    if (moved_value.has_value())
    {
        int result = moved_value.value();
        (void)result;
    }
    moved_value.reset();
    return (static_cast<int>(initial_value.has_value())
            + static_cast<int>(moved_value.has_value()));
}

static volatile int template_optional_result = check_optional_usage();
}
