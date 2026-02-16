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
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void    abort_lifecycle_error(const char *method_name, const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;
        int     random_int_unlocked();
        float   random_float_unlocked();

    public:
        rng_stream();
        explicit rng_stream(uint32_t seed_value);
        rng_stream(const rng_stream &other) = delete;
        rng_stream &operator=(const rng_stream &other) = delete;
        rng_stream(rng_stream &&other) noexcept = delete;
        rng_stream &operator=(rng_stream &&other) noexcept = delete;
        ~rng_stream();

        int     initialize();
        int     initialize(const rng_stream &other);
        int     initialize(rng_stream &&other);
        int     destroy();
        int     enable_thread_safety();
        int     disable_thread_safety();
        bool    is_thread_safe() const;

        int     reseed(uint32_t seed_value);
        int     reseed_from_string(const char *seed_string);

        Pair<int, int> random_int();
        Pair<int, int> dice_roll(int number, int faces);
        Pair<int, float> random_float();
        Pair<int, float> random_normal();
        Pair<int, float> random_exponential(float lambda_value);
        Pair<int, int> random_poisson(double lambda_value);
        Pair<int, int> random_binomial(int trial_count, double success_probability);
        Pair<int, int> random_geometric(double success_probability);
        Pair<int, float> random_gamma(float shape, float scale);
        Pair<int, float> random_beta(float alpha, float beta);
        Pair<int, float> random_chi_squared(float degrees_of_freedom);

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex   *get_mutex_for_validation() const;
#endif
};

#endif
