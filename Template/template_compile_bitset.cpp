#include "bitset.hpp"

static int compile_bitset_usage()
{
    ft_bitset bitset_value(8);
    int test_value;

    bitset_value.set(1);
    test_value = 0;
    if (bitset_value.test(1))
        test_value = 1;
    return (test_value);
}

static int bitset_dummy = compile_bitset_usage();
