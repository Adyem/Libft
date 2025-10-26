#ifndef DOCUMENT_BACKEND_HPP
#define DOCUMENT_BACKEND_HPP

#include <cstddef>
#include "../CPP_class/class_string_class.hpp"
#include "../Errno/errno.hpp"

class ft_document_source
{
    private:
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    protected:
        void assign_error(int error_code) const noexcept;

    public:
        ft_document_source() noexcept;
        virtual ~ft_document_source();

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        virtual int read_all(ft_string &output) = 0;
};

class ft_document_sink
{
    private:
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    protected:
        void assign_error(int error_code) const noexcept;

    public:
        ft_document_sink() noexcept;
        virtual ~ft_document_sink();

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) = 0;
};

class ft_file_document_source : public ft_document_source
{
    private:
        ft_string _path;

    public:
        explicit ft_file_document_source(const char *file_path) noexcept;
        virtual ~ft_file_document_source();

        void set_path(const char *file_path) noexcept;
        const char *get_path() const noexcept;
        virtual int read_all(ft_string &output) override;
};

class ft_file_document_sink : public ft_document_sink
{
    private:
        ft_string _path;

    public:
        explicit ft_file_document_sink(const char *file_path) noexcept;
        virtual ~ft_file_document_sink();

        void set_path(const char *file_path) noexcept;
        const char *get_path() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;
};

class ft_memory_document_source : public ft_document_source
{
    private:
        const char *_data_pointer;
        size_t _data_length;

    public:
        ft_memory_document_source() noexcept;
        ft_memory_document_source(const char *data_pointer, size_t data_length) noexcept;
        virtual ~ft_memory_document_source();

        void set_data(const char *data_pointer, size_t data_length) noexcept;
        const char *get_data() const noexcept;
        size_t get_length() const noexcept;
        virtual int read_all(ft_string &output) override;
};

class ft_memory_document_sink : public ft_document_sink
{
    private:
        ft_string *_storage_pointer;

    public:
        ft_memory_document_sink() noexcept;
        explicit ft_memory_document_sink(ft_string *storage_pointer) noexcept;
        virtual ~ft_memory_document_sink();

        void set_storage(ft_string *storage_pointer) noexcept;
        ft_string *get_storage() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;
};

class ft_http_document_source : public ft_document_source
{
    private:
        ft_string _host;
        ft_string _path;
        ft_string _port;
        bool _use_ssl;

    public:
        ft_http_document_source() noexcept;
        ft_http_document_source(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        virtual ~ft_http_document_source();

        void configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        const char *get_host() const noexcept;
        const char *get_path() const noexcept;
        const char *get_port() const noexcept;
        bool is_ssl_enabled() const noexcept;
        virtual int read_all(ft_string &output) override;
};

class ft_http_document_sink : public ft_document_sink
{
    private:
        ft_string _host;
        ft_string _path;
        ft_string _port;
        bool _use_ssl;

    public:
        ft_http_document_sink() noexcept;
        ft_http_document_sink(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        virtual ~ft_http_document_sink();

        void configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept;
        const char *get_host() const noexcept;
        const char *get_path() const noexcept;
        const char *get_port() const noexcept;
        bool is_ssl_enabled() const noexcept;
        virtual int write_all(const char *data_pointer, size_t data_length) override;
};

#endif
