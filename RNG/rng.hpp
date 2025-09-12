#ifndef RNG_HPP
# define RNG_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

#include <ctime>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include "../CPP_class/class_nullptr.hpp"

int ft_random_int(void);
int ft_dice_roll(int number, int faces);
float ft_random_float(void);
float ft_random_normal(void);
float ft_random_exponential(float lambda_value);
int ft_random_poisson(double lambda_value);
int ft_random_seed(const char *seed_str = ft_nullptr);
int rng_secure_bytes(unsigned char *buffer, size_t length);
uint32_t ft_random_uint32(void);

#endif
