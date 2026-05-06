#ifndef RNG_HPP
# define RNG_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

#include <ctime>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "rng_stream.hpp"

int32_t ft_random_int(void);
int32_t ft_dice_roll(int32_t number, int32_t faces);
float ft_random_float(void);
float ft_random_normal(void);
float ft_random_exponential(float lambda_value);
int32_t ft_random_poisson(double lambda_value);
int32_t ft_random_binomial(int32_t trial_count, double success_probability);
int32_t ft_random_geometric(double success_probability);
float ft_random_gamma(float shape, float scale);
float ft_random_beta(float alpha, float beta);
float ft_random_chi_squared(float degrees_of_freedom);
uint32_t ft_random_seed(const char *seed_string = ft_nullptr);
int64_t rng_seed_value(const char *seed_string = ft_nullptr);
int64_t rng_seed_value(const ft_string &seed_string);
int32_t rng_secure_bytes(unsigned char *buffer, ft_size_t length);
uint32_t ft_random_uint32(void);
void ft_generate_uuid(char output_buffer[37]);
int32_t rng_secure_bytes_with_fallback(unsigned char *buffer, ft_size_t length, int32_t *fallback_used);
int32_t rng_secure_uint64(uint64_t *value, int32_t *fallback_used = ft_nullptr);
int32_t rng_secure_uint32(uint32_t *value, int32_t *fallback_used = ft_nullptr);
int32_t rng_stream_seed(uint64_t base_seed, uint64_t stream_identifier, uint64_t *stream_seed);
int32_t rng_stream_seed_sequence(uint64_t base_seed, uint64_t stream_identifier, uint32_t *buffer, ft_size_t count);
int32_t rng_stream_seed_from_string(const char *seed_string, uint64_t stream_identifier, uint64_t *stream_seed);
int32_t rng_stream_seed_sequence_from_string(const char *seed_string, uint64_t stream_identifier, uint32_t *buffer, ft_size_t count);
double rng_gamma_pdf(double shape, double scale, double value);
double rng_gamma_cdf(double shape, double scale, double value);
double rng_beta_pdf(double alpha, double beta, double value);
double rng_beta_cdf(double alpha, double beta, double value);
double rng_chi_squared_pdf(double degrees_of_freedom, double value);
double rng_chi_squared_cdf(double degrees_of_freedom, double value);
int32_t ft_random_int_vector(int32_t minimum_value, int32_t maximum_value, int32_t *output_values, ft_size_t output_count);
int32_t ft_random_float_vector(float *output_values, ft_size_t output_count);

#endif
