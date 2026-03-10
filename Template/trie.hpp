#ifndef FT_TEMPLATE_TRIE_HPP
#define FT_TEMPLATE_TRIE_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "unordered_map.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ValueType>
class ft_trie
{
    private:
        struct node_value
        {
            ft_size_t      _key_length;
            int32_t     _unset_value;
            ValueType  *_value_pointer;
        };

        node_value                                  *_data;
        ft_unordered_map<char, ft_trie<ValueType>*> _children;
        mutable pt_recursive_mutex                  *_mutex;
        uint8_t                                      _initialised_state;

        static const uint8_t _state_uninitialised = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialised = 2;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "ft_trie lifecycle error: %s: %s\n", method_name, reason);
            su_abort();
            return ;
        }

        void abort_if_not_initialised(const char *method_name) const
        {
            if (this->_initialised_state == _state_initialised)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialised");
            return ;
        }

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        int32_t unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            return (FT_ERR_SUCCESS);
        }

        ft_size_t key_length(const char *key) const
        {
            ft_size_t index;

            index = 0;
            while (key[index] != '\0')
                ++index;
            return (index);
        }

        void destroy_children_unlocked()
        {
            typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

            while (child_iterator != this->_children.end())
            {
                if (child_iterator->second != ft_nullptr)
                    delete child_iterator->second;
                ++child_iterator;
            }
            this->_children.clear();
            return ;
        }

        int32_t insert_helper(const char *key, int32_t unset_value, ValueType *value_pointer)
        {
            ft_trie<ValueType> *current_node;
            const char *key_iterator;
            ft_size_t length_value;

            if (key == ft_nullptr)
                return (set_error(FT_ERR_INVALID_ARGUMENT));
            length_value = this->key_length(key);
            current_node = this;
            key_iterator = key;
            while (*key_iterator != '\0')
            {
                char character;
                ft_trie<ValueType> *new_child;
                int32_t child_result;

                character = *key_iterator;
                if (current_node->_children[character] == ft_nullptr)
                {
                    new_child = new (std::nothrow) ft_trie<ValueType>();
                    if (new_child == ft_nullptr)
                        return (set_error(FT_ERR_NO_MEMORY));
                    if (new_child->initialize() != FT_ERR_SUCCESS)
                    {
                        delete new_child;
                        return (set_error(FT_ERR_INVALID_STATE));
                    }
                    current_node->_children[character] = new_child;
                    if (this->_mutex != ft_nullptr)
                    {
                        child_result = new_child->enable_thread_safety();
                        if (child_result != FT_ERR_SUCCESS)
                        {
                            current_node->_children[character] = ft_nullptr;
                            delete new_child;
                            return (set_error(child_result));
                        }
                    }
                }
                current_node = current_node->_children[character];
                ++key_iterator;
            }
            if (current_node->_data == ft_nullptr)
            {
                current_node->_data = static_cast<node_value*>(cma_malloc(sizeof(node_value)));
                if (current_node->_data == ft_nullptr)
                    return (set_error(FT_ERR_NO_MEMORY));
            }
            current_node->_data->_unset_value = unset_value;
            current_node->_data->_key_length = length_value;
            current_node->_data->_value_pointer = value_pointer;
            return (set_error(FT_ERR_SUCCESS));
        }

    public:
        ft_trie()
            : _data(ft_nullptr), _children(), _mutex(ft_nullptr),
              _initialised_state(_state_uninitialised)
        {
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_trie()
        {
            if (this->_initialised_state == _state_initialised)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_trie(const ft_trie &other) = delete;
        ft_trie &operator=(const ft_trie &other) = delete;
        ft_trie(ft_trie &&other) = delete;
        ft_trie &operator=(ft_trie &&other) = delete;

        int32_t initialize()
        {
            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_trie::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_data = ft_nullptr;
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t destroy()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            if (this->_initialised_state != _state_initialised)
                return (set_error(FT_ERR_SUCCESS));
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            this->destroy_children_unlocked();
            if (this->_data != ft_nullptr)
            {
                cma_free(this->_data);
                this->_data = ft_nullptr;
            }
            (void)this->unlock_internal(lock_acquired);
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t insert(const char *key, ValueType *value_pointer, int32_t unset_value = 0)
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            int32_t insert_result;

            this->abort_if_not_initialised("ft_trie::insert");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_error(lock_error));
            insert_result = this->insert_helper(key, unset_value, value_pointer);
            (void)this->unlock_internal(lock_acquired);
            return (set_error(insert_result));
        }

        const node_value *search(const char *key) const
        {
            const ft_trie<ValueType> *current_node;
            const char *key_iterator;
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_trie::search");
            if (key == ft_nullptr)
            {
                set_error(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return (ft_nullptr);
            }
            current_node = this;
            key_iterator = key;
            while (*key_iterator != '\0')
            {
                typename ft_unordered_map<char, ft_trie<ValueType>*>::const_iterator child_iterator(current_node->_children.find(*key_iterator));
                if (child_iterator == current_node->_children.end()
                    || child_iterator->second == ft_nullptr)
                {
                    (void)this->unlock_internal(lock_acquired);
                    set_error(FT_ERR_NOT_FOUND);
                    return (ft_nullptr);
                }
                current_node = child_iterator->second;
                ++key_iterator;
            }
            (void)this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return (current_node->_data);
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;
            typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

            this->abort_if_not_initialised("ft_trie::enable_thread_safety");
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
            while (child_iterator != this->_children.end())
            {
                if (child_iterator->second != ft_nullptr)
                {
                    initialize_result = child_iterator->second->enable_thread_safety();
                    if (initialize_result != FT_ERR_SUCCESS)
                        return (set_error(initialize_result));
                }
                ++child_iterator;
            }
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int32_t destroy_result;
            typename ft_unordered_map<char, ft_trie<ValueType>*>::iterator child_iterator(this->_children.begin());

            if (this->_initialised_state != _state_initialised
                && this->_initialised_state != _state_destroyed)
                return (set_error(FT_ERR_INVALID_STATE));
            while (child_iterator != this->_children.end())
            {
                if (child_iterator->second != ft_nullptr)
                    (void)child_iterator->second->disable_thread_safety();
                ++child_iterator;
            }
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
            this->abort_if_not_initialised("ft_trie::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("ft_trie::lock");
            lock_result = this->lock_internal(lock_acquired);
            return (set_error(lock_result));
        }

        void unlock(ft_bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t get_error() noexcept
        {
            return (_last_error);
        }

        static const char *get_error_str() noexcept
        {
            return (ft_strerror(_last_error));
        }

};

template <typename ValueType>
thread_local int32_t ft_trie<ValueType>::_last_error = FT_ERR_SUCCESS;

#endif
