#ifndef FT_BITSET_HPP
#define FT_BITSET_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstddef>
#include <climits>

class ft_bitset
{
    private:
        static const size_t BITS_PER_BLOCK = sizeof(size_t) * CHAR_BIT;

        size_t                       _size;
        size_t                       _block_count;
        size_t*                      _data;
        mutable pt_recursive_mutex*  _mutex;
        static thread_local int32_t  _last_error;

        static int32_t set_last_operation_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        size_t block_index(size_t position) const
        {
            return (position / BITS_PER_BLOCK);
        }

        size_t bit_mask(size_t position) const
        {
            return (static_cast<size_t>(1) << (position % BITS_PER_BLOCK));
        }

        void destroy_data()
        {
            if (this->_data != ft_nullptr)
            {
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            this->_size = 0;
            this->_block_count = 0;
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (this->_mutex->lock() != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            if (lock_acquired == false || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (this->_mutex->unlock() != FT_ERR_SUCCESS)
                return (set_last_operation_error(FT_ERR_SYS_MUTEX_UNLOCK_FAILED));
            return (FT_ERR_SUCCESS);
        }

    public:
        explicit ft_bitset(size_t bits = 0)
            : _size(bits),
              _block_count((bits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK),
              _data(ft_nullptr),
              _mutex(ft_nullptr)
        {
            size_t index;

            if (this->_block_count > 0)
            {
                this->_data = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_block_count));
                if (this->_data == ft_nullptr)
                {
                    this->_size = 0;
                    this->_block_count = 0;
                    set_last_operation_error(FT_ERR_BITSET_NO_MEMORY);
                    return ;
                }
                index = 0;
                while (index < this->_block_count)
                {
                    this->_data[index] = 0;
                    index += 1;
                }
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_bitset()
        {
            bool lock_acquired;

            lock_acquired = false;
            if (this->lock_internal(&lock_acquired) == FT_ERR_SUCCESS)
            {
                this->destroy_data();
                (void)this->unlock_internal(lock_acquired);
            }
            (void)this->disable_thread_safety();
            return ;
        }

        ft_bitset(const ft_bitset&) = delete;
        ft_bitset& operator=(const ft_bitset&) = delete;
        ft_bitset(ft_bitset&& other) = delete;
        ft_bitset& operator=(ft_bitset&& other) = delete;

        void set(size_t position)
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_last_operation_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] |= this->bit_mask(position);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void reset(size_t position)
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_last_operation_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] &= ~this->bit_mask(position);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void flip(size_t position)
        {
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_last_operation_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] ^= this->bit_mask(position);
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        bool test(size_t position) const
        {
            bool lock_acquired;
            int lock_result;
            bool result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (false);
            if (position >= this->_size)
            {
                set_last_operation_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return (false);
            }
            result = (this->_data[this->block_index(position)] & this->bit_mask(position)) != 0;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        size_t size() const
        {
            size_t current_size;
            bool lock_acquired;
            int lock_result;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (0);
            current_size = this->_size;
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (current_size);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_result;
            size_t index;

            lock_acquired = false;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            index = 0;
            while (index < this->_block_count)
            {
                this->_data[index] = 0;
                index += 1;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            if (this->_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_last_operation_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int destroy_result;

            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(destroy_result));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            if (this->lock_internal(lock_acquired) != FT_ERR_SUCCESS)
                return (-1);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t last_operation_error()
        {
            return (_last_error);
        }

        static const char *last_operation_error_str()
        {
            return (ft_strerror(_last_error));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

thread_local int32_t ft_bitset::_last_error = FT_ERR_SUCCESS;

#endif
