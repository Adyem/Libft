#include "bitset.hpp"

static int compile_bitset_usage()
{
    ft_bitset bitset_value(8);

    bitset_value.set(1);
    return (bitset_value.test(1) ? 1 : 0);
}

static int bitset_dummy = compile_bitset_usage();
