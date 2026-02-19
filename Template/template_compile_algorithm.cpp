#include "../Basic/basic.hpp"
#include "algorithm.hpp"
#include "constructor.hpp"
#include "container_serialization.hpp"
#include "invoke.hpp"
#include "move.hpp"
#include "static_cast.hpp"
#include "string_view.hpp"
#include "swap.hpp"

static int instantiate_free_functions()
{
    int data[] = {1, 2, 3};
    ft_sort(data, data + 3);
    ft_sort(data, data + 3, [](const int &left, const int &right)
    {
        return (left < right);
    });
    ft_binary_search(data, data + 3, 2);
    ft_binary_search(data, data + 3, 2, [](const int &left, const int &right)
    {
        return (left < right);
    });
    ft_shuffle(data, data + 3);
    ft_reverse(data, data + 3);
    int max_value = ft_max(1, 2);
    int min_value = ft_min(1, 2);
    ft_swap(data[0], data[1]);
    construct_at(&data[0], 5);
    destroy_at(&data[0]);
    int sample = 7;
    auto moved_sample = ft_move(sample);
    (void)moved_sample;
    int target_value = 0;
    static_cast_or_construct<int>(&target_value);
    int invoked_value = ft_invoke([](int value)
    {
        return (value + 1);
    }, 5);
    (void)invoked_value;
    ft_string serialized;
    default_string_serializer<int>(42, serialized);
    default_string_serializer<const char *>("abc", serialized);
    ft_string_view<char> view("hello");
    (void)view.size();
    ft_string_view<char> substring;
    (void)view.substr(1, substring);
    (void)substring.compare(view);
    return (max_value + min_value);
}

static int functions_dummy = (instantiate_free_functions(), 0);
