#include "../../Basic/basic.hpp"
#include "utils.hpp"

#include <cstring>
#include <string>

int test_efficiency_strlen(void)
{
    const size_t iterations = 100000;
    std::string string(1000, 'a');
    volatile size_t sink = 0;
    auto standard_strlen = static_cast<size_t (*)(const char *)>(std::strlen);

    auto start_std = clock_type::now();
    for (size_t index = 0; index < iterations; ++index)
    {
        prevent_optimization(string.data());
        sink += standard_strlen(string.c_str());
        prevent_optimization((void*)&sink);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t index = 0; index < iterations; ++index)
    {
        prevent_optimization(string.data());
        sink += ft_strlen(string.c_str());
        prevent_optimization((void*)&sink);
    }
    auto end_ft = clock_type::now();
    print_comparison("strlen", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

