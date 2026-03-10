#ifndef FT_BITSET_HPP
#define FT_BITSET_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>
#include <climits>

class ft_bitset
{
    private:
        static const ft_size_t BITS_PER_BLOCK = sizeof(ft_size_t) * CHAR_BIT;

        ft_size_t                       _size;
        ft_size_t                       _block_count;
        ft_size_t*                      _data;
        mutable pt_recursive_mutex*  _mutex;
        static thread_local int32_t  _last_error;

        static int32_t set_error(int32_t error_code)
        {
            _last_error = error_code;
            return (error_code);
        }

        ft_size_t block_index(ft_size_t position) const
        {
            return (position / BITS_PER_BLOCK);
        }

        ft_size_t bit_mask(ft_size_t position) const
        {
            return (static_cast<ft_size_t>(1) << (position % BITS_PER_BLOCK));
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

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
                return (set_error(FT_ERR_SYS_MUTEX_LOCK_FAILED));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        int32_t unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE || this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            return (FT_ERR_SUCCESS);
        }

    public:
        explicit ft_bitset(ft_size_t bits = 0)
            : _size(bits),
              _block_count((bits + BITS_PER_BLOCK - 1) / BITS_PER_BLOCK),
              _data(ft_nullptr),
              _mutex(ft_nullptr)
        {
            ft_size_t index;

            if (this->_block_count > 0)
            {
                this->_data = static_cast<ft_size_t*>(cma_malloc(sizeof(ft_size_t) * this->_block_count));
                if (this->_data == ft_nullptr)
                {
                    this->_size = 0;
                    this->_block_count = 0;
                    set_error(FT_ERR_BITSET_NO_MEMORY);
                    return ;
                }
                index = 0;
                while (index < this->_block_count)
                {
                    this->_data[index] = 0;
                    index += 1;
                }
            }
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_bitset()
        {
            ft_bool lock_acquired;

            lock_acquired = FT_FALSE;
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

        void set(ft_size_t position)
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] |= this->bit_mask(position);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void reset(ft_size_t position)
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] &= ~this->bit_mask(position);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        void flip(ft_size_t position)
        {
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            if (position >= this->_size)
            {
                set_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            this->_data[this->block_index(position)] ^= this->bit_mask(position);
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        bool test(ft_size_t position) const
        {
            ft_bool lock_acquired;
            int32_t lock_result;
            bool result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (false);
            if (position >= this->_size)
            {
                set_error(FT_ERR_OUT_OF_RANGE);
                (void)this->unlock_internal(lock_acquired);
                return (false);
            }
            result = (this->_data[this->block_index(position)] & this->bit_mask(position)) != 0;
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (result);
        }

        ft_size_t size() const
        {
            ft_size_t current_size;
            ft_bool lock_acquired;
            int32_t lock_result;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (0);
            current_size = this->_size;
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return (current_size);
        }

        void clear()
        {
            ft_bool lock_acquired;
            int32_t lock_result;
            ft_size_t index;

            lock_acquired = FT_FALSE;
            lock_result = this->lock_internal(&lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return ;
            index = 0;
            while (index < this->_block_count)
            {
                this->_data[index] = 0;
                index += 1;
            }
            set_error(FT_ERR_SUCCESS);
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            if (this->_mutex != ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_error(initialize_result));
            }
            this->_mutex = new_mutex;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int32_t destroy_result;

            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_error(destroy_result));
            return (set_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            return (set_error(this->lock_internal(lock_acquired)));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t get_error()
        {
            return (_last_error);
        }

        static const char *get_error_str()
        {
            return (ft_strerror(_last_error));
        }

};

thread_local int32_t ft_bitset::_last_error = FT_ERR_SUCCESS;

#endif
