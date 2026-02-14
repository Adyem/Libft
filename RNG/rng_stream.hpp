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
        mutable pt_recursive_mutex _mutex;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     random_int_unlocked();
        float   random_float_unlocked();

    public:
        rng_stream();
        explicit rng_stream(uint32_t seed_value);
        rng_stream(const rng_stream &other);
        rng_stream &operator=(const rng_stream &other);
        rng_stream(rng_stream &&other) noexcept;
        rng_stream &operator=(rng_stream &&other) noexcept;
        ~rng_stream();

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

        pt_recursive_mutex   *mutex_handle();
        const pt_recursive_mutex   *mutex_handle() const;
};

#endif
