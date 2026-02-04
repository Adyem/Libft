#ifndef RNG_STREAM_HPP
# define RNG_STREAM_HPP

#include <random>
#include <cstdint>

#include "../PThread/recursive_mutex.hpp"

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

        void    reseed(uint32_t seed_value);
        void    reseed_from_string(const char *seed_string);

        int     random_int();
        int     dice_roll(int number, int faces);
        float   random_float();
        float   random_normal();
        float   random_exponential(float lambda_value);
        int     random_poisson(double lambda_value);
        int     random_binomial(int trial_count, double success_probability);
        int     random_geometric(double success_probability);
        float   random_gamma(float shape, float scale);
        float   random_beta(float alpha, float beta);
        float   random_chi_squared(float degrees_of_freedom);

        pt_recursive_mutex   *mutex_handle();
        const pt_recursive_mutex   *mutex_handle() const;
};

#endif
