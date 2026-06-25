#ifndef MATH_ROLL_HPP
# define MATH_ROLL_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

#include <cstdint>

int32_t     *math_roll(const char *expression);
int32_t     *math_eval(const char *expression);

#endif
