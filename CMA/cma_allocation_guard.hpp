#ifndef CMA_ALLOCATION_GUARD_HPP
# define CMA_ALLOCATION_GUARD_HPP

#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

class cma_allocation_guard
{
    private:
        void    *_pointer;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        cma_allocation_guard();
        explicit cma_allocation_guard(void *memory_pointer);
        ~cma_allocation_guard();

        cma_allocation_guard(const cma_allocation_guard &) = delete;
        cma_allocation_guard &operator=(const cma_allocation_guard &) = delete;

        cma_allocation_guard(cma_allocation_guard &&other) noexcept;
        cma_allocation_guard &operator=(cma_allocation_guard &&other) noexcept;

        void reset(void *memory_pointer);
        void *release();
        void *get() const;
        bool owns_allocation() const;

        int get_error() const;
        const char *get_error_str() const;
};

cma_allocation_guard cma_make_allocation_guard(void *memory_pointer);

#endif
