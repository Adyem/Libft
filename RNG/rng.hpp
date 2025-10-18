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
int ft_random_binomial(int trial_count, double success_probability);
int ft_random_geometric(double success_probability);
uint32_t ft_random_seed(const char *seed_str = ft_nullptr);
int rng_secure_bytes(unsigned char *buffer, size_t length);
uint32_t ft_random_uint32(void);
void ft_generate_uuid(char out[37]);
int rng_secure_bytes_with_fallback(unsigned char *buffer, size_t length, int *fallback_used);
int rng_secure_uint64(uint64_t *value, int *fallback_used = ft_nullptr);
int rng_secure_uint32(uint32_t *value, int *fallback_used = ft_nullptr);
int rng_stream_seed(uint64_t base_seed, uint64_t stream_identifier, uint64_t *stream_seed);
int rng_stream_seed_sequence(uint64_t base_seed, uint64_t stream_identifier, uint32_t *buffer, size_t count);
int rng_stream_seed_from_string(const char *seed_string, uint64_t stream_identifier, uint64_t *stream_seed);
int rng_stream_seed_sequence_from_string(const char *seed_string, uint64_t stream_identifier, uint32_t *buffer, size_t count);

#endif
