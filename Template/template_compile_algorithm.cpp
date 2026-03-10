#include "../Basic/basic.hpp"
#include <cstdint>
#include "algorithm.hpp"
#include "constructor.hpp"
#include "container_serialization.hpp"
#include "invoke.hpp"
#include "move.hpp"
#include "math.hpp"
#include "static_cast.hpp"
#include "string_view.hpp"
#include "swap.hpp"

static int32_t instantiate_free_functions()
{
    int32_t data[] = {1, 2, 3};
    ft_sort(data, data + 3);
    ft_sort(data, data + 3, [](const int32_t &left, const int32_t &right)
    {
        return (left < right);
    });
    ft_binary_search(data, data + 3, 2);
    ft_binary_search(data, data + 3, 2, [](const int32_t &left, const int32_t &right)
    {
        return (left < right);
    });
    ft_shuffle(data, data + 3);
    ft_reverse(data, data + 3);
    int32_t max_value = ft_max(1, 2);
    int32_t min_value = ft_min(1, 2);
    ft_swap(data[0], data[1]);
    construct_at(&data[0], 5);
    destroy_at(&data[0]);
    int32_t sample = 7;
    auto moved_sample = ft_move(sample);
    (void)moved_sample;
    int32_t target_value = 0;
    static_cast_or_construct<int32_t>(&target_value);
    int32_t invoked_value = ft_invoke([](int32_t value)
    {
        return (value + 1);
    }, 5);
    (void)invoked_value;
    ft_string serialized;
    default_string_serializer<int32_t>(42, serialized);
    default_string_serializer<const char *>("abc", serialized);
    ft_string_view<char> view("hello");
    (void)view.size();
    ft_string_view<char> substring;
    (void)view.substr(1, substring);
    (void)substring.compare(view);
    return (max_value + min_value);
}

static int32_t functions_dummy = (instantiate_free_functions(), 0);
