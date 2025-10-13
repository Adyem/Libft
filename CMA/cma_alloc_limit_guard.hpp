#ifndef CMA_ALLOC_LIMIT_GUARD_HPP
# define CMA_ALLOC_LIMIT_GUARD_HPP

#include "CMA.hpp"
#include "../Errno/errno.hpp"

class cma_alloc_limit_guard
{
    private:
        ft_size_t _previous_limit;
        bool _active;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        explicit cma_alloc_limit_guard(ft_size_t new_limit);
        ~cma_alloc_limit_guard();

        cma_alloc_limit_guard(const cma_alloc_limit_guard &) = delete;
        cma_alloc_limit_guard &operator=(const cma_alloc_limit_guard &) = delete;

        cma_alloc_limit_guard(cma_alloc_limit_guard &&other) noexcept;
        cma_alloc_limit_guard &operator=(cma_alloc_limit_guard &&other) noexcept;

        void reset(ft_size_t new_limit);
        bool is_active() const;

        int get_error() const;
        const char *get_error_str() const;
};

cma_alloc_limit_guard cma_make_alloc_limit_guard(ft_size_t new_limit);

#endif
