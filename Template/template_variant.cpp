#include "variant.hpp"

static int check_variant_usage()
{
    ft_variant<int, const char*> variant_value;
    variant_value.emplace<int>(42);
    int stored_integer = variant_value.get<int>();
    (void)stored_integer;
    variant_value.emplace<const char*>("hello");
    const char *stored_string = variant_value.get<const char*>();
    (void)stored_string;
    variant_value.visit([](const auto &value)
    {
        (void)value;
    });
    variant_value.reset();
    return (static_cast<int>(variant_value.holds_alternative<int>())
            + static_cast<int>(variant_value.holds_alternative<const char*>()));
}

static volatile int template_variant_result = check_variant_usage();
