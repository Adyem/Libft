#include <utility>
#ifndef ft_move
# define ft_move std::move
#endif
#include "../Template/promise.hpp"
#include "../PThread/PThread.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>

int test_ft_promise_set_get(void)
{
    ft_promise<int> p;
    p.set_value(42);
    return (p.is_ready() && p.get() == 42 && p.get_error() == ER_SUCCESS);
}

int test_ft_promise_not_ready(void)
{
    ft_promise<int> p;
    p.get();
    return (p.get_error() == FT_EINVAL);
}

int test_pt_async_basic(void)
{
    ft_promise<int> p;
    if (pt_async(p, []() { return 7; }) != 0)
        return 0;
    while (!p.is_ready())
        usleep(1000);
    return (p.get() == 7);
}
