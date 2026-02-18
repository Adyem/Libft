#ifndef DOCUMENT_BACKEND_HPP
#define DOCUMENT_BACKEND_HPP

#include <cstddef>
#include "../CPP_class/class_string.hpp"
#include "../Networking/openssl_support.hpp"
#include <stdint.h>

class pt_mutex;

class ft_document_source
{
    private:
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_document_source() noexcept;
        virtual ~ft_document_source();
        ft_document_source(const ft_document_source &other) = delete;
        ft_document_source &operator=(const ft_document_source &other) = delete;
        ft_document_source(ft_document_source &&other) = delete;
        ft_document_source &operator=(ft_document_source &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;
        virtual int read_all(ft_string &output) = 0;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_document_sink
{
    private:
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_document_sink() noexcept;
        virtual ~ft_document_sink();
        ft_document_sink(const ft_document_sink &other) = delete;
        ft_document_sink &operator=(const ft_document_sink &other) = delete;
        ft_document_sink(ft_document_sink &&other) = delete;
        ft_document_sink &operator=(ft_document_sink &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) = 0;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_file_document_source : public ft_document_source
{
    private:
        ft_string _path;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        explicit ft_file_document_source(const char *file_path) noexcept;
        virtual ~ft_file_document_source();
        ft_file_document_source(const ft_file_document_source &other) = delete;
        ft_file_document_source &operator=(const ft_file_document_source &other) = delete;
        ft_file_document_source(ft_file_document_source &&other) = delete;
        ft_file_document_source &operator=(ft_file_document_source &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_path(const char *file_path) noexcept;
        const char *get_path() const noexcept;
        virtual int read_all(ft_string &output) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_file_document_sink : public ft_document_sink
{
    private:
        ft_string _path;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        explicit ft_file_document_sink(const char *file_path) noexcept;
        virtual ~ft_file_document_sink();
        ft_file_document_sink(const ft_file_document_sink &other) = delete;
        ft_file_document_sink &operator=(const ft_file_document_sink &other) = delete;
        ft_file_document_sink(ft_file_document_sink &&other) = delete;
        ft_file_document_sink &operator=(ft_file_document_sink &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_path(const char *file_path) noexcept;
        const char *get_path() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_memory_document_source : public ft_document_source
{
    private:
        const char *_data_pointer;
        size_t _data_length;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_memory_document_source() noexcept;
        ft_memory_document_source(const char *data_pointer, size_t data_length) noexcept;
        virtual ~ft_memory_document_source();
        ft_memory_document_source(const ft_memory_document_source &other) = delete;
        ft_memory_document_source &operator=(const ft_memory_document_source &other) = delete;
        ft_memory_document_source(ft_memory_document_source &&other) = delete;
        ft_memory_document_source &operator=(ft_memory_document_source &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_data(const char *data_pointer, size_t data_length) noexcept;
        const char *get_data() const noexcept;
        size_t get_length() const noexcept;
        virtual int read_all(ft_string &output) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_memory_document_sink : public ft_document_sink
{
    private:
        ft_string *_storage_pointer;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_memory_document_sink() noexcept;
        explicit ft_memory_document_sink(ft_string *storage_pointer) noexcept;
        virtual ~ft_memory_document_sink();
        ft_memory_document_sink(const ft_memory_document_sink &other) = delete;
        ft_memory_document_sink &operator=(const ft_memory_document_sink &other) = delete;
        ft_memory_document_sink(ft_memory_document_sink &&other) = delete;
        ft_memory_document_sink &operator=(ft_memory_document_sink &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_storage(ft_string *storage_pointer) noexcept;
        ft_string *get_storage() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#if NETWORKING_HAS_OPENSSL
class ft_http_document_source : public ft_document_source
{
    private:
        ft_string _host;
        ft_string _path;
        ft_string _port;
        bool _use_ssl;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_http_document_source() noexcept;
        ft_http_document_source(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        virtual ~ft_http_document_source();
        ft_http_document_source(const ft_http_document_source &other) = delete;
        ft_http_document_source &operator=(const ft_http_document_source &other) = delete;
        ft_http_document_source(ft_http_document_source &&other) = delete;
        ft_http_document_source &operator=(ft_http_document_source &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        const char *get_host() const noexcept;
        const char *get_path() const noexcept;
        const char *get_port() const noexcept;
        bool is_ssl_enabled() const noexcept;
        virtual int read_all(ft_string &output) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_http_document_sink : public ft_document_sink
{
    private:
        ft_string _host;
        ft_string _path;
        ft_string _port;
        bool _use_ssl;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_http_document_sink() noexcept;
        ft_http_document_sink(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        virtual ~ft_http_document_sink();
        ft_http_document_sink(const ft_http_document_sink &other) = delete;
        ft_http_document_sink &operator=(const ft_http_document_sink &other) = delete;
        ft_http_document_sink(ft_http_document_sink &&other) = delete;
        ft_http_document_sink &operator=(ft_http_document_sink &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        const char *get_host() const noexcept;
        const char *get_path() const noexcept;
        const char *get_port() const noexcept;
        bool is_ssl_enabled() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};
#endif

#endif
