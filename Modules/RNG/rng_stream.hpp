#ifndef RNG_STREAM_HPP
# define RNG_STREAM_HPP

#include <random>
#include <cstdint>

#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

class rng_stream
{
    private:
        std::mt19937 _engine;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        int32_t random_int_unlocked();
        float random_float_unlocked();

    public:
        rng_stream() noexcept;
        rng_stream(const rng_stream &other) noexcept;
        rng_stream(rng_stream &&other) noexcept;
        rng_stream &operator=(const rng_stream &other) noexcept = delete;
        rng_stream &operator=(rng_stream &&other) noexcept = delete;
        ~rng_stream() noexcept;

        int32_t initialize() noexcept;
        int32_t initialize(const rng_stream &other) noexcept;
        int32_t initialize(rng_stream &&other) noexcept;
        int32_t destroy() noexcept;
        uint32_t move(rng_stream &other) noexcept;

        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t reseed(uint32_t seed_value) noexcept;
        int32_t reseed_from_string(const char *seed_string) noexcept;

        Pair<int32_t, int32_t> random_int();
        Pair<int32_t, int32_t> dice_roll(int32_t number, int32_t faces);
        Pair<int32_t, float> random_float();
        Pair<int32_t, float> random_normal();
        Pair<int32_t, float> random_exponential(float lambda_value);
        Pair<int32_t, int32_t> random_poisson(double lambda_value);
        Pair<int32_t, int32_t> random_binomial(int32_t trial_count,
            double success_probability);
        Pair<int32_t, int32_t> random_geometric(double success_probability);
        Pair<int32_t, float> random_gamma(float shape, float scale);
        Pair<int32_t, float> random_beta(float alpha, float beta);
        Pair<int32_t, float> random_chi_squared(float degrees_of_freedom);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const;
#endif
};

#endif
