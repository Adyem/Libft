#ifndef RNG_INTERNAL_HPP
# define RNG_INTERNAL_HPP

#include "../CPP_class/nullptr.hpp"
#include <ctime>
#include <cstdlib>

extern bool g_srand_init;

inline __attribute__((always_inline)) void ft_init_srand(void)
{
    if (g_srand_init == false)
    {
        srand(static_cast<unsigned>(time(ft_nullptr)));
        g_srand_init = true;
    }
	return ;
}

#endif
