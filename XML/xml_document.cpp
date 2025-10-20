#include "xml.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstdio>
#include <cerrno>
#include <new>

xml_document::thread_guard::thread_guard(const xml_document *document) noexcept
    : _document(document), _lock_acquired(false), _status(0), _entry_errno(ft_errno)
{
    if (!this->_document)
        return ;
    this->_status = this->_document->lock(&this->_lock_acquired);
    if (this->_status == 0)
        ft_errno = this->_entry_errno;
    return ;
}

xml_document::thread_guard::~thread_guard() noexcept
{
    if (!this->_document)
        return ;
    this->_document->unlock(this->_lock_acquired);
    return ;
}

int xml_document::thread_guard::get_status() const noexcept
{
    return (this->_status);
}

bool xml_document::thread_guard::lock_acquired() const noexcept
{
    return (this->_lock_acquired);
}

static const char *skip_whitespace(const char *string)
{
    while (string && (*string == ' ' || *string == '\n' || *string == '\t' || *string == '\r'))
        string++;
    return (string);
}

static int  is_xml_whitespace(char character)
{
    if (character == ' ')
        return (1);
    if (character == '\n')
        return (1);
    if (character == '\t')
        return (1);
    if (character == '\r')
        return (1);
    return (0);
}

static char *duplicate_range(const char *start, size_t length)
{
    char *copy = static_cast<char *>(cma_malloc(length + 1));
    if (!copy)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < length)
    {
        copy[index] = start[index];
        index++;
    }
    copy[length] = '\0';
    return (copy);
}

static int translate_vector_error(int error_code)
{
    if (error_code == ER_SUCCESS)
        return (ER_SUCCESS);
    if (error_code == FT_ERR_NO_MEMORY)
        return (FT_ERR_NO_MEMORY);
    return (error_code);
}

xml_node::xml_node() noexcept
    : mutex(ft_nullptr), thread_safe_enabled(false), name(ft_nullptr), text(ft_nullptr), children(), attributes()
{
    return ;
}

xml_node::~xml_node() noexcept
{
    size_t child_index = 0;
    size_t child_size = this->children.size();
    while (child_index < child_size)
    {
        xml_node *child = this->children[child_index];
        if (child)
            delete child;
        child_index++;
    }
    ft_unordered_map<char*, char*>::iterator it = this->attributes.begin();
    ft_unordered_map<char*, char*>::iterator end = this->attributes.end();
    while (it != end)
    {
        if (it->first)
            cma_free(it->first);
        if (it->second)
            cma_free(it->second);
        ++it;
    }
    if (this->name)
        cma_free(this->name);
    if (this->text)
        cma_free(this->text);
    xml_node_teardown_thread_safety(this);
    return ;
}

static const char *parse_node(const char *string, xml_node **out_node)
{
    string = skip_whitespace(string);
    if (!string || *string != '<')
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    const char *name_start = string;
    while (*string && *string != '>' && *string != ' ' && *string != '/')
        string++;
    size_t name_length = static_cast<size_t>(string - name_start);
    char *name = duplicate_range(name_start, name_length);
    if (!name)
        return (ft_nullptr);
    xml_node *node = new (std::nothrow) xml_node();
    if (!node)
    {
        cma_free(name);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    node->name = name;
    if (xml_node_prepare_thread_safety(node) != 0)
    {
        delete node;
        return (ft_nullptr);
    }
    char quote_character;

    quote_character = '\0';
    while (*string)
    {
        if (*string == '>' && quote_character == '\0')
            break ;
        if (*string == '"' || *string == '\'')
        {
            if (quote_character == '\0')
                quote_character = *string;
            else if (quote_character == *string)
                quote_character = '\0';
        }
        string++;
    }
    if (*string != '>')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    const char *tag_end = string;
    int self_closing = 0;
    if (tag_end > name_start)
    {
        const char *before_close = tag_end - 1;
        while (before_close >= name_start)
        {
            if (*before_close == '/')
            {
                self_closing = 1;
                break;
            }
            if (*before_close != ' ' && *before_close != '\n' && *before_close != '\t' && *before_close != '\r')
                break;
            before_close--;
        }
    }
    string++;
    if (self_closing)
    {
        *out_node = node;
        ft_errno = ER_SUCCESS;
        return (string);
    }
    const char *text_start = string;
    if (*string == '<' && string[1] == '/');
    else if (*string == '<')
    {
        while (1)
        {
            xml_node *child = ft_nullptr;
            string = parse_node(string, &child);
            if (!string)
            {
                delete node;
                return (ft_nullptr);
            }
            node->children.push_back(child);
            int children_error_code = node->children.get_error();
            if (children_error_code != ER_SUCCESS)
            {
                delete child;
                delete node;
                ft_errno = translate_vector_error(children_error_code);
                return (ft_nullptr);
            }
            string = skip_whitespace(string);
            if (*string == '<' && string[1] == '/')
                break;
        }
    }
    else
    {
        while (*string && !(*string == '<' && string[1] == '/'))
            string++;
        size_t text_length = static_cast<size_t>(string - text_start);
        node->text = duplicate_range(text_start, text_length);
        if (!node->text && text_length > 0)
        {
            delete node;
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    if (*string != '<' || string[1] != '/')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string += 2;
    string = skip_whitespace(string);
    const char *closing_name_start = string;
    while (*string && *string != '>' && !is_xml_whitespace(*string))
        string++;
    size_t closing_name_length = static_cast<size_t>(string - closing_name_start);
    size_t opening_name_length = ft_strlen(node->name);
    if (closing_name_length != opening_name_length)
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    size_t name_index = 0;
    while (name_index < closing_name_length)
    {
        if (closing_name_start[name_index] != node->name[name_index])
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        name_index++;
    }
    while (*string && *string != '>')
    {
        if (!is_xml_whitespace(*string))
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        string++;
    }
    if (*string != '>')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    *out_node = node;
    ft_errno = ER_SUCCESS;
    return (string);
}

xml_document::xml_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr), _thread_safe_enabled(false), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    if (this->prepare_thread_safety() != 0)
        return ;
    return ;
}

xml_document::~xml_document() noexcept
{
    {
        thread_guard guard(this);

        if (guard.get_status() == 0)
        {
            if (this->_root)
            {
                delete this->_root;
                this->_root = ft_nullptr;
            }
        }
    }
    this->teardown_thread_safety();
    return ;
}

void xml_document::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int xml_document::load_from_string(const char *xml) noexcept
{
    thread_guard guard(this);
    int          lock_error;

    if (guard.get_status() != 0)
    {
        lock_error = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE;
        this->set_error(lock_error);
        return (lock_error);
    }
    if (!xml)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (this->_root)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    xml_node *node = ft_nullptr;
    const char *end = parse_node(xml, &node);
    if (!end)
    {
        int error_code;

        error_code = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (error_code);
    }
    const char *remaining = skip_whitespace(end);
    if (remaining && *remaining != '\0')
    {
        delete node;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_root = node;
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

static int read_file_content(const char *file_path, char **out_content)
{
    errno = 0;
    FILE *file = std::fopen(file_path, "rb");
    if (!file)
    {
        int error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        ft_errno = error_code;
        return (error_code);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        int error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        ft_errno = error_code;
        return (error_code);
    }
    long size = std::ftell(file);
    if (size <= 0)
    {
        std::fclose(file);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_SET) != 0)
    {
        int error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        ft_errno = error_code;
        return (error_code);
    }
    char *buffer = static_cast<char *>(cma_malloc(static_cast<size_t>(size) + 1));
    if (!buffer)
    {
        std::fclose(file);
        ft_errno = FT_ERR_NO_MEMORY;
        return (FT_ERR_NO_MEMORY);
    }
    errno = 0;
    size_t read_size = std::fread(buffer, 1, static_cast<size_t>(size), file);
    std::fclose(file);
    if (read_size != static_cast<size_t>(size))
    {
        cma_free(buffer);
        int error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        ft_errno = error_code;
        return (error_code);
    }
    buffer[size] = '\0';
    *out_content = buffer;
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

int xml_document::load_from_file(const char *file_path) noexcept
{
    char *content;
    int   result;

    content = ft_nullptr;
    result = read_file_content(file_path, &content);
    if (result != ER_SUCCESS)
    {
        thread_guard guard(this);

        if (guard.get_status() != 0)
            this->set_error(ft_errno != ER_SUCCESS ? ft_errno : result);
        else
            this->set_error(result);
        return (result);
    }
    result = this->load_from_string(content);
    cma_free(content);
    return (result);
}

static void append_string(ft_vector<char> &buffer, const char *string)
{
    if (!string)
        return ;
    size_t index = 0;
    size_t length = ft_strlen(string);
    while (index < length)
    {
        buffer.push_back(string[index]);
        index++;
    }
    return ;
}

static int write_node(const xml_node *node, ft_vector<char> &buffer)
{
    bool lock_acquired;
    int lock_status;
    int entry_errno;

    if (!node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    entry_errno = ft_errno;
    lock_acquired = false;
    lock_status = xml_node_lock(node, &lock_acquired);
    if (lock_status != 0)
        return (-1);
    buffer.push_back('<');
    append_string(buffer, node->name);
    buffer.push_back('>');
    if (node->text)
        append_string(buffer, node->text);
    size_t index = 0;
    size_t size = node->children.size();
    while (index < size)
    {
        int child_status;

        child_status = write_node(node->children[index], buffer);
        if (child_status != 0)
        {
            xml_node_unlock(node, lock_acquired);
            return (-1);
        }
        index++;
    }
    buffer.push_back('<');
    buffer.push_back('/');
    append_string(buffer, node->name);
    buffer.push_back('>');
    xml_node_unlock(node, lock_acquired);
    ft_errno = entry_errno;
    return (0);
}

char *xml_document::write_to_string() const noexcept
{
    thread_guard guard(this);
    int          lock_error;

    if (guard.get_status() != 0)
    {
        lock_error = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE;
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    if (!this->_root)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char *result = ft_nullptr;
    int error_code = ER_SUCCESS;
    {
        ft_vector<char> buffer;
        int write_status;
        int buffer_error;

        write_status = write_node(this->_root, buffer);
        if (write_status != 0)
            error_code = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE;
        if (error_code == ER_SUCCESS)
        {
            buffer_error = buffer.get_error();
            if (buffer_error != ER_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == ER_SUCCESS)
        {
            buffer.push_back('\n');
            buffer_error = buffer.get_error();
            if (buffer_error != ER_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == ER_SUCCESS)
        {
            buffer.push_back('\0');
            buffer_error = buffer.get_error();
            if (buffer_error != ER_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == ER_SUCCESS)
        {
            size_t length = buffer.size();
            result = static_cast<char *>(cma_malloc(length));
            if (!result)
                error_code = FT_ERR_NO_MEMORY;
            else
            {
                size_t index = 0;
                while (index < length)
                {
                    result[index] = buffer[index];
                    index++;
                }
            }
        }
    }
    if (error_code != ER_SUCCESS)
    {
        if (result)
            cma_free(result);
        this->set_error(error_code);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (result);
}

int xml_document::write_to_file(const char *file_path) const noexcept
{
    char *content;
    int   error_code;

    content = this->write_to_string();
    if (!content)
    {
        int current_error;

        current_error = this->get_error();
        if (current_error == ER_SUCCESS)
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
            {
                error_code = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_ARGUMENT;
                this->set_error(error_code);
                return (error_code);
            }
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (FT_ERR_INVALID_ARGUMENT);
        }
        return (current_error);
    }
    errno = 0;
    FILE *file = std::fopen(file_path, "wb");
    if (!file)
    {
        error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
                this->set_error(ft_errno != ER_SUCCESS ? ft_errno : error_code);
            else
                this->set_error(error_code);
        }
        int preserved_errno;

        preserved_errno = ft_errno;
        cma_free(content);
        ft_errno = preserved_errno;
        return (error_code);
    }
    size_t length = ft_strlen(content);
    errno = 0;
    size_t written = std::fwrite(content, 1, length, file);
    if (written != length)
    {
        error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
                this->set_error(ft_errno != ER_SUCCESS ? ft_errno : error_code);
            else
                this->set_error(error_code);
        }
        int preserved_errno;

        preserved_errno = ft_errno;
        cma_free(content);
        ft_errno = preserved_errno;
        return (error_code);
    }
    errno = 0;
    if (std::fclose(file) != 0)
    {
        error_code = errno ? ft_map_system_error(errno) : FT_ERR_INVALID_ARGUMENT;
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
                this->set_error(ft_errno != ER_SUCCESS ? ft_errno : error_code);
            else
                this->set_error(error_code);
        }
        int preserved_errno;

        preserved_errno = ft_errno;
        cma_free(content);
        ft_errno = preserved_errno;
        return (error_code);
    }
    cma_free(content);
    {
        thread_guard guard(this);

        if (guard.get_status() != 0)
        {
            this->set_error(ft_errno != ER_SUCCESS ? ft_errno : ER_SUCCESS);
            return (ft_errno != ER_SUCCESS ? ft_errno : ER_SUCCESS);
        }
        this->set_error(ER_SUCCESS);
    }
    return (ER_SUCCESS);
}

xml_node *xml_document::get_root() const noexcept
{
    thread_guard guard(this);
    int          lock_error;

    if (guard.get_status() != 0)
    {
        lock_error = ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE;
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_root);
}

int xml_document::get_error() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE);
    return (this->_error_code);
}

const char *xml_document::get_error_str() const noexcept
{
    thread_guard guard(this);
    const char *message;

    if (guard.get_status() != 0)
        return (ft_strerror(ft_errno != ER_SUCCESS ? ft_errno : FT_ERR_INVALID_STATE));
    message = ft_strerror(this->_error_code);
    return (message);
}

bool xml_document::is_thread_safe_enabled() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (false);
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        this->set_error(ER_SUCCESS);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

int xml_document::prepare_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (!memory)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void xml_document::teardown_thread_safety() noexcept
{
    if (!this->_mutex)
    {
        this->_thread_safe_enabled = false;
        return ;
    }
    this->_mutex->~pt_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    return ;
}

int xml_document::lock(bool *lock_acquired) const noexcept
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || !this->_mutex)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<xml_document *>(this)->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void xml_document::unlock(bool lock_acquired) const noexcept
{
    int entry_errno;

    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<xml_document *>(this)->set_error(mutex_error);
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

