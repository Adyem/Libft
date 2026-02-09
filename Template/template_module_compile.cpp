#include "../Basic/basic.hpp"
#include "algorithm.hpp"
#include "bitset.hpp"
#include "cancellation.hpp"
#include "circular_buffer.hpp"
#include "container_serialization.hpp"
#include "constructor.hpp"
#include "deque.hpp"
#include "event_emitter.hpp"
#include "function.hpp"
#include "future.hpp"
#include "graph.hpp"
#include "invoke.hpp"
#include "iterator.hpp"
#include "map.hpp"
#include "math.hpp"
#include "matrix.hpp"
#include "move.hpp"
#include "optional.hpp"
#include "pair.hpp"
#include "pool.hpp"
#include "priority_queue.hpp"
#include "promise.hpp"
#include "queue.hpp"
#include "set.hpp"
#include "shared_ptr.hpp"
#include "stack.hpp"
#include "static_cast.hpp"
#include "string_view.hpp"
#include "swap.hpp"
#include "thread_pool.hpp"
#include "trie.hpp"
#include "tuple.hpp"
#include "unique_ptr.hpp"
#include "unordered_map.hpp"
#include "variant.hpp"
#include "vector.hpp"

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
    auto substring = view.substr(1, 3);
    (void)substring.compare(view);
    return (max_value + min_value);
}

static int functions_dummy = (instantiate_free_functions(), 0);

template class ft_vector<int>;
template class ft_queue<int>;
template class ft_stack<int>;
template class ft_deque<int>;
template class ft_priority_queue<int>;
template class ft_circular_buffer<int>;
template class ft_map<int, int>;
template class ft_set<int>;
template class ft_unordered_map<int, int>;
template class ft_sharedptr<int>;
template class ft_uniqueptr<int>;
template class ft_optional<int>;
template class ft_variant<int, const char *>;
template class ft_future<int>;
template class ft_future<void>;
template class ft_promise<int>;
template class ft_promise<void>;
template class ft_event_emitter<int>;
template class ft_function<int(int)>;
template class ft_graph<int>;
template class ft_matrix<int>;
template class Pool<int>;
template class Iterator<int>;
template class ft_trie<int>;
template class ft_tuple<int, double>;
template class ft_string_view<char>;
