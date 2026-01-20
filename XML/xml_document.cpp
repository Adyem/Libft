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

struct xml_namespace_entry
{
    char *prefix;
    char *uri;
    const xml_namespace_entry *parent;
    xml_namespace_entry *next_local;
};

xml_document::thread_guard::thread_guard(const xml_document *document) noexcept
    : _document(document), _lock_acquired(false), _status(0)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (!this->_document)
        return ;
    this->_status = this->_document->lock(&this->_lock_acquired);
    if (this->_status == 0)
        ft_errno = FT_ERR_SUCCESSS;
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
    if (error_code == FT_ERR_SUCCESSS)
        return (FT_ERR_SUCCESSS);
    if (error_code == FT_ERR_NO_MEMORY)
        return (FT_ERR_NO_MEMORY);
    return (error_code);
}

static const char *xml_skip_whitespace_range(const char *cursor, const char *end)
{
    while (cursor && cursor < end && is_xml_whitespace(*cursor))
        cursor++;
    return (cursor);
}

static int xml_add_namespace_binding(xml_node *node, const char *prefix,
        size_t prefix_length, const char *uri,
        const xml_namespace_entry **active_scope)
{
    xml_namespace_entry *entry;

    entry = new (std::nothrow) xml_namespace_entry();
    if (!entry)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    entry->prefix = ft_nullptr;
    if (prefix && prefix_length > 0)
    {
        entry->prefix = duplicate_range(prefix, prefix_length);
        if (!entry->prefix)
        {
            delete entry;
            return (-1);
        }
    }
    entry->uri = cma_strdup(uri);
    if (!entry->uri)
    {
        if (entry->prefix)
            cma_free(entry->prefix);
        delete entry;
        return (-1);
    }
    entry->parent = *active_scope;
    entry->next_local = node->namespace_bindings;
    node->namespace_bindings = entry;
    *active_scope = entry;
    return (0);
}

static const char *xml_resolve_namespace_uri(
        const xml_namespace_entry *scope_head, const char *prefix)
{
    const xml_namespace_entry *current;

    current = scope_head;
    while (current)
    {
        if (!prefix || *prefix == '\0')
        {
            if (!current->prefix || current->prefix[0] == '\0')
                return (current->uri);
        }
        else if (current->prefix && ft_strcmp(current->prefix, prefix) == 0)
            return (current->uri);
        current = current->parent;
    }
    return (ft_nullptr);
}

static int xml_parse_attributes(xml_node *node, const char *start,
        const char *end, const xml_namespace_entry *scope_head,
        const xml_namespace_entry **out_scope)
{
    const char                *cursor;
    const xml_namespace_entry *active_scope;

    cursor = start;
    active_scope = scope_head;
    while (cursor && cursor < end)
    {
        cursor = xml_skip_whitespace_range(cursor, end);
        if (!cursor || cursor >= end)
            break ;
        if (*cursor == '/')
        {
            cursor++;
            continue ;
        }
        const char *name_start;
        const char *name_end;
        char       *attribute_name;
        bool        has_value;
        char       *attribute_value;

        name_start = cursor;
        while (cursor < end && !is_xml_whitespace(*cursor)
            && *cursor != '=' && *cursor != '/' && *cursor != '>')
            cursor++;
        name_end = cursor;
        while (name_end > name_start && is_xml_whitespace(name_end[-1]))
            name_end--;
        if (name_end <= name_start)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        attribute_name = duplicate_range(name_start,
                static_cast<size_t>(name_end - name_start));
        if (!attribute_name)
            return (-1);
        cursor = xml_skip_whitespace_range(cursor, end);
        has_value = false;
        attribute_value = ft_nullptr;
        if (cursor && cursor < end && *cursor == '=')
        {
            char            quote_character;
            const char      *value_start;
            const char      *value_end;

            cursor++;
            cursor = xml_skip_whitespace_range(cursor, end);
            if (!cursor || cursor >= end)
            {
                cma_free(attribute_name);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (-1);
            }
            quote_character = '\0';
            if (*cursor == '"' || *cursor == '\'')
            {
                quote_character = *cursor;
                cursor++;
            }
            value_start = cursor;
            while (cursor < end)
            {
                if (quote_character != '\0')
                {
                    if (*cursor == quote_character)
                        break ;
                }
                else if (is_xml_whitespace(*cursor) || *cursor == '/' || *cursor == '>')
                    break ;
                cursor++;
            }
            value_end = cursor;
            if (quote_character != '\0')
            {
                if (!cursor || cursor >= end || *cursor != quote_character)
                {
                    cma_free(attribute_name);
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                    return (-1);
                }
            }
            attribute_value = duplicate_range(value_start,
                    static_cast<size_t>(value_end - value_start));
            if (!attribute_value)
            {
                cma_free(attribute_name);
                return (-1);
            }
            has_value = true;
            if (quote_character != '\0' && cursor < end && *cursor == quote_character)
                cursor++;
        }
        bool        is_namespace;
        const char *namespace_prefix_pointer;
        size_t      namespace_prefix_length;

        is_namespace = false;
        namespace_prefix_pointer = ft_nullptr;
        namespace_prefix_length = 0;
        if (attribute_name[0] == 'x' && attribute_name[1] == 'm'
            && attribute_name[2] == 'l' && attribute_name[3] == 'n'
            && attribute_name[4] == 's')
        {
            if (attribute_name[5] == '\0')
                is_namespace = true;
            else if (attribute_name[5] == ':' && attribute_name[6] != '\0')
            {
                const char *prefix_end;

                is_namespace = true;
                namespace_prefix_pointer = attribute_name + 6;
                prefix_end = namespace_prefix_pointer;
                while (*prefix_end)
                    prefix_end++;
                namespace_prefix_length = static_cast<size_t>(prefix_end
                        - namespace_prefix_pointer);
            }
            else if (attribute_name[5] == ':' && attribute_name[6] == '\0')
            {
                cma_free(attribute_name);
                if (attribute_value)
                    cma_free(attribute_value);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (-1);
            }
        }
        if (is_namespace)
        {
            if (!has_value || !attribute_value)
            {
                cma_free(attribute_name);
                if (attribute_value)
                    cma_free(attribute_value);
                ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (-1);
            }
            if (xml_add_namespace_binding(node, namespace_prefix_pointer,
                    namespace_prefix_length, attribute_value, &active_scope) != 0)
            {
                cma_free(attribute_name);
                cma_free(attribute_value);
                return (-1);
            }
        }
        node->attributes.insert(attribute_name, attribute_value);
        int attribute_error = node->attributes.last_operation_error();
        if (attribute_error != FT_ERR_SUCCESSS)
        {
            if (attribute_value)
                cma_free(attribute_value);
            cma_free(attribute_name);
            ft_errno = attribute_error;
            return (-1);
        }
    }
    *out_scope = active_scope;
    return (0);
}

xml_node::xml_node() noexcept
    : mutex(ft_nullptr), thread_safe_enabled(false), name(ft_nullptr), namespace_prefix(ft_nullptr),
    local_name(ft_nullptr), namespace_uri(ft_nullptr), namespace_bindings(ft_nullptr), text(ft_nullptr),
    children(), attributes()
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
    if (this->namespace_prefix)
        cma_free(this->namespace_prefix);
    if (this->local_name)
        cma_free(this->local_name);
    xml_namespace_entry *binding = this->namespace_bindings;
    while (binding)
    {
        xml_namespace_entry *next_binding;

        next_binding = binding->next_local;
        if (binding->prefix)
            cma_free(binding->prefix);
        if (binding->uri)
            cma_free(binding->uri);
        delete binding;
        binding = next_binding;
    }
    if (this->name)
        cma_free(this->name);
    if (this->text)
        cma_free(this->text);
    xml_node_teardown_thread_safety(this);
    return ;
}

static const char *parse_node(const char *string, xml_node **out_node,
        const xml_namespace_entry *scope_head)
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
    const char *colon_position = ft_strchr(node->name, ':');
    if (colon_position)
    {
        size_t prefix_length;

        prefix_length = static_cast<size_t>(colon_position - node->name);
        if (prefix_length == 0 || colon_position[1] == '\0')
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        node->namespace_prefix = duplicate_range(node->name, prefix_length);
        if (!node->namespace_prefix)
        {
            delete node;
            return (ft_nullptr);
        }
        node->local_name = cma_strdup(colon_position + 1);
        if (!node->local_name)
        {
            delete node;
            return (ft_nullptr);
        }
    }
    else
    {
        node->namespace_prefix = ft_nullptr;
        node->local_name = cma_strdup(node->name);
        if (!node->local_name)
        {
            delete node;
            return (ft_nullptr);
        }
    }
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
    const char *attribute_start = name_start + name_length;
    const char *attribute_end = tag_end;
    if (attribute_end > attribute_start)
    {
        const char *trim_cursor;

        trim_cursor = attribute_end;
        while (trim_cursor > attribute_start && is_xml_whitespace(trim_cursor[-1]))
            trim_cursor--;
        if (trim_cursor > attribute_start && trim_cursor[-1] == '/')
        {
            self_closing = 1;
            trim_cursor--;
            while (trim_cursor > attribute_start && is_xml_whitespace(trim_cursor[-1]))
                trim_cursor--;
        }
        attribute_end = trim_cursor;
    }
    const xml_namespace_entry *active_scope = scope_head;
    if (attribute_end > attribute_start)
    {
        if (xml_parse_attributes(node, attribute_start, attribute_end,
                scope_head, &active_scope) != 0)
        {
            delete node;
            return (ft_nullptr);
        }
    }
    if (node->namespace_prefix)
    {
        node->namespace_uri = xml_resolve_namespace_uri(active_scope,
                node->namespace_prefix);
        if (!node->namespace_uri)
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
    }
    else
        node->namespace_uri = xml_resolve_namespace_uri(active_scope, ft_nullptr);
    string++;
    if (self_closing)
    {
        *out_node = node;
        ft_errno = FT_ERR_SUCCESSS;
        return (string);
    }
    const char *text_start = string;
    if (*string == '<' && string[1] == '/');
    else if (*string == '<')
    {
        while (1)
        {
            xml_node *child = ft_nullptr;
            string = parse_node(string, &child, active_scope);
            if (!string)
            {
                delete node;
                return (ft_nullptr);
            }
            node->children.push_back(child);
            int children_error_code = node->children.get_error();
            if (children_error_code != FT_ERR_SUCCESSS)
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
    ft_errno = FT_ERR_SUCCESSS;
    return (string);
}

xml_document::xml_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr), _thread_safe_enabled(false), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
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
        lock_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE;
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
    const char *end = parse_node(xml, &node, ft_nullptr);
    if (!end)
    {
        int error_code;

        error_code = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_ARGUMENT;
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
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
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
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

int xml_document::load_from_file(const char *file_path) noexcept
{
    char *content;
    int   result;

    content = ft_nullptr;
    result = read_file_content(file_path, &content);
    if (result != FT_ERR_SUCCESSS)
    {
        thread_guard guard(this);

        if (guard.get_status() != 0)
            this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : result);
        else
            this->set_error(result);
        return (result);
    }
    result = this->load_from_string(content);
    cma_free(content);
    return (result);
}

int xml_document::load_from_backend(ft_document_source &source) noexcept
{
    ft_string content;
    int read_result;

    read_result = source.read_all(content);
    if (read_result != FT_ERR_SUCCESSS)
    {
        thread_guard guard(this);

        if (guard.get_status() != 0)
            this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : read_result);
        else
            this->set_error(read_result);
        return (read_result);
    }
    return (this->load_from_string(content.c_str()));
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

static void append_node_qualified_name(const xml_node *node, ft_vector<char> &buffer)
{
    const char *prefix;
    const char *local_name;

    if (!node)
        return ;
    prefix = node->namespace_prefix;
    if (prefix && prefix[0] != '\0')
    {
        append_string(buffer, prefix);
        buffer.push_back(':');
    }
    local_name = node->local_name;
    if (!local_name || local_name[0] == '\0')
        local_name = node->name;
    append_string(buffer, local_name);
    return ;
}

static int write_node(const xml_node *node, ft_vector<char> &buffer)
{
    bool lock_acquired;
    int lock_status;

    if (!node)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    lock_acquired = false;
    lock_status = xml_node_lock(node, &lock_acquired);
    if (lock_status != 0)
        return (-1);
    buffer.push_back('<');
    append_node_qualified_name(node, buffer);
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
    append_node_qualified_name(node, buffer);
    buffer.push_back('>');
    xml_node_unlock(node, lock_acquired);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

char *xml_document::write_to_string() const noexcept
{
    thread_guard guard(this);
    int          lock_error;

    if (guard.get_status() != 0)
    {
        lock_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE;
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    if (!this->_root)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char *result = ft_nullptr;
    int error_code = FT_ERR_SUCCESSS;
    {
        ft_vector<char> buffer;
        int write_status;
        int buffer_error;

        write_status = write_node(this->_root, buffer);
        if (write_status != 0)
            error_code = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE;
        if (error_code == FT_ERR_SUCCESSS)
        {
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESSS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESSS)
        {
            buffer.push_back('\n');
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESSS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESSS)
        {
            buffer.push_back('\0');
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESSS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESSS)
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
    if (error_code != FT_ERR_SUCCESSS)
    {
        if (result)
            cma_free(result);
        this->set_error(error_code);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
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
        if (current_error == FT_ERR_SUCCESSS)
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
            {
                error_code = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_ARGUMENT;
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
                this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : error_code);
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
                this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : error_code);
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
                this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : error_code);
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
            this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_SUCCESSS);
            return (ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_SUCCESSS);
        }
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (FT_ERR_SUCCESSS);
}

int xml_document::write_to_backend(ft_document_sink &sink) const noexcept
{
    char *content;
    size_t length;
    int write_result;
    int sink_error;

    content = this->write_to_string();
    if (!content)
        return (this->get_error());
    length = ft_strlen(content);
    write_result = sink.write_all(content, length);
    sink_error = sink.get_error();
    cma_free(content);
    if (write_result != FT_ERR_SUCCESSS)
    {
        int final_error;

        final_error = write_result;
        if (sink_error != FT_ERR_SUCCESSS)
            final_error = sink_error;
        {
            thread_guard guard(this);

            if (guard.get_status() != 0)
            {
                this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : final_error);
                return (ft_errno != FT_ERR_SUCCESSS ? ft_errno : final_error);
            }
            this->set_error(final_error);
        }
        return (final_error);
    }
    {
        thread_guard guard(this);

        if (guard.get_status() != 0)
        {
            this->set_error(ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_SUCCESSS);
            return (ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_SUCCESSS);
        }
        this->set_error(FT_ERR_SUCCESSS);
    }
    return (FT_ERR_SUCCESSS);
}

xml_node *xml_document::get_root() const noexcept
{
    thread_guard guard(this);
    int          lock_error;

    if (guard.get_status() != 0)
    {
        lock_error = ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE;
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (this->_root);
}

void xml_document::set_manual_error(int error_code) noexcept
{
    this->set_error(error_code);
    return ;
}

int xml_document::get_error() const noexcept
{
    thread_guard guard(this);

    if (guard.get_status() != 0)
        return (ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE);
    return (this->_error_code);
}

const char *xml_document::get_error_str() const noexcept
{
    thread_guard guard(this);
    const char *message;

    if (guard.get_status() != 0)
        return (ft_strerror(ft_errno != FT_ERR_SUCCESSS ? ft_errno : FT_ERR_INVALID_STATE));
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
        this->set_error(FT_ERR_SUCCESSS);
        return (false);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

int xml_document::prepare_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    void     *memory;

    if (this->_thread_safe_enabled && this->_mutex)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (!memory)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
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
    this->set_error(FT_ERR_SUCCESSS);
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
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<xml_document *>(this)->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void xml_document::unlock(bool lock_acquired) const noexcept
{
    if (!lock_acquired || !this->_thread_safe_enabled || !this->_mutex)
        return ;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        ft_errno = mutex_error;
        const_cast<xml_document *>(this)->set_error(mutex_error);
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
