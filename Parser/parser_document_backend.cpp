#include "document_backend.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CMA/CMA.hpp"
#include "../Networking/http_client.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <fcntl.h>

ft_document_source::ft_document_source() noexcept
    : _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_document_source::~ft_document_source()
{
    return ;
}

int ft_document_source::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_document_source::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_document_source::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_document_source::assign_error(int error_code) const noexcept
{
    this->set_error(error_code);
    return ;
}

ft_document_sink::ft_document_sink() noexcept
    : _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_document_sink::~ft_document_sink()
{
    return ;
}

int ft_document_sink::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_document_sink::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_document_sink::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_document_sink::assign_error(int error_code) const noexcept
{
    this->set_error(error_code);
    return ;
}

ft_file_document_source::ft_file_document_source(const char *file_path) noexcept
    : _path()
{
    this->set_path(file_path);
    return ;
}

ft_file_document_source::~ft_file_document_source()
{
    return ;
}

void ft_file_document_source::set_path(const char *file_path) noexcept
{
    if (file_path == ft_nullptr)
    {
        this->_path.clear();
        return ;
    }
    this->_path = file_path;
    return ;
}

const char *ft_file_document_source::get_path() const noexcept
{
    return (this->_path.c_str());
}

int ft_file_document_source::read_all(ft_string &output) 
{
    su_file *file_stream;
    long file_size_long;
    size_t file_size;
    char *content_pointer;
    size_t read_count;
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    file_stream = su_fopen(this->_path.c_str());
    if (file_stream == ft_nullptr)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_IO;
        this->assign_error(error_code);
        return (error_code);
    }
    file_size_long = 0;
    file_size = 0;
    content_pointer = ft_nullptr;
    read_count = 0;
    error_code = FT_ER_SUCCESSS;
    if (su_fseek(file_stream, 0, SEEK_END) != 0)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_IO;
    }
    if (error_code == FT_ER_SUCCESSS)
    {
        file_size_long = su_ftell(file_stream);
        if (file_size_long < 0)
            error_code = FT_ERR_IO;
    }
    if (error_code == FT_ER_SUCCESSS)
    {
        if (su_fseek(file_stream, 0, SEEK_SET) != 0)
        {
            error_code = ft_errno;
            if (error_code == FT_ER_SUCCESSS)
                error_code = FT_ERR_IO;
        }
    }
    if (error_code == FT_ER_SUCCESSS)
    {
        if (static_cast<unsigned long long>(file_size_long)
            > static_cast<unsigned long long>(static_cast<size_t>(-1)))
            error_code = FT_ERR_OUT_OF_RANGE;
    }
    if (error_code == FT_ER_SUCCESSS)
    {
        file_size = static_cast<size_t>(file_size_long);
        content_pointer = static_cast<char *>(cma_malloc(file_size + 1));
        if (content_pointer == ft_nullptr)
            error_code = FT_ERR_NO_MEMORY;
    }
    if (error_code == FT_ER_SUCCESSS)
    {
        read_count = su_fread(content_pointer, 1, file_size, file_stream);
        if (read_count != file_size)
        {
            error_code = ft_errno;
            if (error_code == FT_ER_SUCCESSS)
                error_code = FT_ERR_IO;
        }
    }
    if (error_code == FT_ER_SUCCESSS)
        content_pointer[file_size] = '\0';
    if (su_fclose(file_stream) != 0 && error_code == FT_ER_SUCCESSS)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_IO;
    }
    if (error_code != FT_ER_SUCCESSS)
    {
        if (content_pointer != ft_nullptr)
            cma_free(content_pointer);
        this->assign_error(error_code);
        return (error_code);
    }
    output.assign(content_pointer, file_size);
    if (output.get_error() != FT_ER_SUCCESSS)
    {
        error_code = output.get_error();
        cma_free(content_pointer);
        this->assign_error(error_code);
        return (error_code);
    }
    cma_free(content_pointer);
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}

ft_file_document_sink::ft_file_document_sink(const char *file_path) noexcept
    : _path()
{
    this->set_path(file_path);
    return ;
}

ft_file_document_sink::~ft_file_document_sink()
{
    return ;
}

void ft_file_document_sink::set_path(const char *file_path) noexcept
{
    if (file_path == ft_nullptr)
    {
        this->_path.clear();
        return ;
    }
    this->_path = file_path;
    return ;
}

const char *ft_file_document_sink::get_path() const noexcept
{
    return (this->_path.c_str());
}

int ft_file_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    su_file *file_stream;
    size_t written_count;
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    if (data_pointer == ft_nullptr && data_length != 0)
    {
        this->assign_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    file_stream = su_fopen(this->_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_stream == ft_nullptr)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_IO;
        this->assign_error(error_code);
        return (error_code);
    }
    written_count = 0;
    error_code = FT_ER_SUCCESSS;
    if (data_length > 0)
    {
        written_count = su_fwrite(data_pointer, 1, data_length, file_stream);
        if (written_count != data_length)
        {
            error_code = ft_errno;
            if (error_code == FT_ER_SUCCESSS)
                error_code = FT_ERR_IO;
        }
    }
    if (su_fclose(file_stream) != 0 && error_code == FT_ER_SUCCESSS)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = FT_ERR_IO;
    }
    if (error_code != FT_ER_SUCCESSS)
    {
        this->assign_error(error_code);
        return (error_code);
    }
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}

ft_memory_document_source::ft_memory_document_source() noexcept
    : _data_pointer(ft_nullptr), _data_length(0)
{
    return ;
}

ft_memory_document_source::ft_memory_document_source(const char *data_pointer, size_t data_length) noexcept
    : _data_pointer(data_pointer), _data_length(data_length)
{
    return ;
}

ft_memory_document_source::~ft_memory_document_source()
{
    return ;
}

void ft_memory_document_source::set_data(const char *data_pointer, size_t data_length) noexcept
{
    this->_data_pointer = data_pointer;
    this->_data_length = data_length;
    return ;
}

const char *ft_memory_document_source::get_data() const noexcept
{
    return (this->_data_pointer);
}

size_t ft_memory_document_source::get_length() const noexcept
{
    return (this->_data_length);
}

int ft_memory_document_source::read_all(ft_string &output)
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    if (this->_data_pointer == ft_nullptr && this->_data_length != 0)
    {
        this->assign_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    output.assign(this->_data_pointer, this->_data_length);
    error_code = output.get_error();
    if (error_code != FT_ER_SUCCESSS)
    {
        this->assign_error(error_code);
        return (error_code);
    }
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}

ft_memory_document_sink::ft_memory_document_sink() noexcept
    : _storage_pointer(ft_nullptr)
{
    return ;
}

ft_memory_document_sink::ft_memory_document_sink(ft_string *storage_pointer) noexcept
    : _storage_pointer(storage_pointer)
{
    return ;
}

ft_memory_document_sink::~ft_memory_document_sink()
{
    return ;
}

void ft_memory_document_sink::set_storage(ft_string *storage_pointer) noexcept
{
    this->_storage_pointer = storage_pointer;
    return ;
}

ft_string *ft_memory_document_sink::get_storage() const noexcept
{
    return (this->_storage_pointer);
}

int ft_memory_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    if (this->_storage_pointer == ft_nullptr)
    {
        this->assign_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (data_pointer == ft_nullptr && data_length != 0)
    {
        this->assign_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_storage_pointer->assign(data_pointer, data_length);
    error_code = this->_storage_pointer->get_error();
    if (error_code != FT_ER_SUCCESSS)
    {
        this->assign_error(error_code);
        return (error_code);
    }
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}

ft_http_document_source::ft_http_document_source() noexcept
    : _host(), _path(), _port(), _use_ssl(false)
{
    return ;
}

ft_http_document_source::ft_http_document_source(const char *host, const char *path, bool use_ssl, const char *port) noexcept
    : _host(), _path(), _port(), _use_ssl(use_ssl)
{
    this->configure(host, path, use_ssl, port);
    return ;
}

ft_http_document_source::~ft_http_document_source()
{
    return ;
}

void ft_http_document_source::configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept
{
    if (host == ft_nullptr)
        this->_host.clear();
    else
        this->_host = host;
    if (path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = path;
    if (port == ft_nullptr)
        this->_port.clear();
    else
        this->_port = port;
    this->_use_ssl = use_ssl;
    return ;
}

const char *ft_http_document_source::get_host() const noexcept
{
    return (this->_host.c_str());
}

const char *ft_http_document_source::get_path() const noexcept
{
    return (this->_path.c_str());
}

const char *ft_http_document_source::get_port() const noexcept
{
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

bool ft_http_document_source::is_ssl_enabled() const noexcept
{
    return (this->_use_ssl);
}

int ft_http_document_source::read_all(ft_string &output)
{
    ft_string response;
    const char *port_pointer;
    int request_status;
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    if (this->_host.size() == 0 || this->_path.size() == 0)
    {
        this->assign_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    port_pointer = ft_nullptr;
    if (this->_port.size() > 0)
        port_pointer = this->_port.c_str();
    request_status = http_get(this->_host.c_str(), this->_path.c_str(), response, this->_use_ssl, port_pointer);
    if (request_status != 0)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = request_status;
        this->assign_error(error_code);
        return (error_code);
    }
    output = response;
    error_code = output.get_error();
    if (error_code != FT_ER_SUCCESSS)
    {
        this->assign_error(error_code);
        return (error_code);
    }
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}

ft_http_document_sink::ft_http_document_sink() noexcept
    : _host(), _path(), _port(), _use_ssl(false)
{
    return ;
}

ft_http_document_sink::ft_http_document_sink(const char *host, const char *path, bool use_ssl, const char *port) noexcept
    : _host(), _path(), _port(), _use_ssl(use_ssl)
{
    this->configure(host, path, use_ssl, port);
    return ;
}

ft_http_document_sink::~ft_http_document_sink()
{
    return ;
}

void ft_http_document_sink::configure(const char *host, const char *path, bool use_ssl, const char *port) noexcept
{
    if (host == ft_nullptr)
        this->_host.clear();
    else
        this->_host = host;
    if (path == ft_nullptr)
        this->_path.clear();
    else
        this->_path = path;
    if (port == ft_nullptr)
        this->_port.clear();
    else
        this->_port = port;
    this->_use_ssl = use_ssl;
    return ;
}

const char *ft_http_document_sink::get_host() const noexcept
{
    return (this->_host.c_str());
}

const char *ft_http_document_sink::get_path() const noexcept
{
    return (this->_path.c_str());
}

const char *ft_http_document_sink::get_port() const noexcept
{
    if (this->_port.size() == 0)
        return (ft_nullptr);
    return (this->_port.c_str());
}

bool ft_http_document_sink::is_ssl_enabled() const noexcept
{
    return (this->_use_ssl);
}

int ft_http_document_sink::write_all(const char *data_pointer, size_t data_length)
{
    ft_string body;
    ft_string response;
    const char *port_pointer;
    int request_status;
    int error_code;
    int entry_errno;

    entry_errno = ft_errno;
    if (this->_host.size() == 0 || this->_path.size() == 0)
    {
        this->assign_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (data_pointer == ft_nullptr && data_length != 0)
    {
        this->assign_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    body.assign(data_pointer, data_length);
    error_code = body.get_error();
    if (error_code != FT_ER_SUCCESSS)
    {
        this->assign_error(error_code);
        return (error_code);
    }
    port_pointer = ft_nullptr;
    if (this->_port.size() > 0)
        port_pointer = this->_port.c_str();
    request_status = http_post(this->_host.c_str(), this->_path.c_str(), body, response, this->_use_ssl, port_pointer);
    if (request_status != 0)
    {
        error_code = ft_errno;
        if (error_code == FT_ER_SUCCESSS)
            error_code = request_status;
        this->assign_error(error_code);
        return (error_code);
    }
    this->assign_error(FT_ER_SUCCESSS);
    ft_errno = entry_errno;
    return (FT_ER_SUCCESSS);
}
