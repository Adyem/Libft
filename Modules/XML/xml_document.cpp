#include "xml.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno_internal.hpp"
#include <cstdio>
#include <cerrno>
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../Template/unordered_map.hpp"
#include "../Template/vector.hpp"

struct xml_namespace_entry
{
    char *prefix;
    char *uri;
    const xml_namespace_entry *parent;
    xml_namespace_entry *next_local;
};

thread_local int32_t xml_document::_last_error = FT_ERR_SUCCESS;

int32_t xml_document::set_error(int32_t error_code) noexcept
{
    xml_document::_last_error = error_code;
    return (error_code);
}

static const char *skip_whitespace(const char *string)
{
    while (string && (*string == ' ' || *string == '\n' || *string == '\t' || *string == '\r'))
        string++;
    return (string);
}

static int32_t  is_xml_whitespace(char character)
{
    if (character == ' ')
        return (1);
    if (character == '\n')
        return (1);
    if (character == '\t')
        return (1);
    if (character == '\r')
        return (1);
    return (FT_ERR_SUCCESS);
}

static char *duplicate_range(const char *start, ft_size_t length)
{
    char *copy = static_cast<char *>(cma_malloc(length + 1));
    if (!copy)
        return (ft_nullptr);
    ft_size_t index = 0;
    while (index < length)
    {
        copy[index] = start[index];
        index++;
    }
    copy[length] = '\0';
    return (copy);
}

static int32_t translate_vector_error(int32_t error_code)
{
    if (error_code == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
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

static int32_t xml_add_namespace_binding(xml_node *node, const char *prefix,
        ft_size_t prefix_length, const char *uri,
        const xml_namespace_entry **active_scope)
{
    xml_namespace_entry *entry;

    entry = new (std::nothrow) xml_namespace_entry();
    if (!entry)
    {
        return (FT_ERR_INTERNAL);
    }
    entry->prefix = ft_nullptr;
    if (prefix && prefix_length > 0)
    {
        entry->prefix = duplicate_range(prefix, prefix_length);
        if (!entry->prefix)
        {
            delete entry;
            return (FT_ERR_INTERNAL);
        }
    }
    entry->uri = duplicate_range(uri, static_cast<ft_size_t>(ft_strlen(uri)));
    if (!entry->uri)
    {
        if (entry->prefix)
            cma_free(entry->prefix);
        delete entry;
        return (FT_ERR_INTERNAL);
    }
    entry->parent = *active_scope;
    entry->next_local = node->namespace_bindings;
    node->namespace_bindings = entry;
    *active_scope = entry;
    return (FT_ERR_SUCCESS);
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

static int32_t xml_parse_attributes(xml_node *node, const char *start,
        const char *end, const xml_namespace_entry *scope_head,
        const xml_namespace_entry **out_scope, int32_t *error_code_out)
{
    const char                *cursor;
    const xml_namespace_entry *active_scope;

    cursor = start;
    active_scope = scope_head;
    if (error_code_out)
        *error_code_out = FT_ERR_SUCCESS;
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
        ft_bool        has_value;
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
            if (error_code_out)
                *error_code_out = FT_ERR_INVALID_ARGUMENT;
            return (FT_ERR_INTERNAL);
        }
        attribute_name = duplicate_range(name_start,
                static_cast<ft_size_t>(name_end - name_start));
        if (!attribute_name)
        {
            if (error_code_out)
                *error_code_out = FT_ERR_NO_MEMORY;
            return (FT_ERR_INTERNAL);
        }
        cursor = xml_skip_whitespace_range(cursor, end);
        has_value = FT_FALSE;
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
                if (error_code_out)
                    *error_code_out = FT_ERR_INVALID_ARGUMENT;
                return (FT_ERR_INTERNAL);
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
                    if (error_code_out)
                        *error_code_out = FT_ERR_INVALID_ARGUMENT;
                    return (FT_ERR_INTERNAL);
                }
            }
            attribute_value = duplicate_range(value_start,
                    static_cast<ft_size_t>(value_end - value_start));
            if (!attribute_value)
            {
                cma_free(attribute_name);
                if (error_code_out)
                    *error_code_out = FT_ERR_NO_MEMORY;
                return (FT_ERR_INTERNAL);
            }
            has_value = FT_TRUE;
            if (quote_character != '\0' && cursor < end && *cursor == quote_character)
                cursor++;
        }
        ft_bool        is_namespace;
        const char *namespace_prefix_pointer;
        ft_size_t      namespace_prefix_length;

        is_namespace = FT_FALSE;
        namespace_prefix_pointer = ft_nullptr;
        namespace_prefix_length = 0;
        if (attribute_name[0] == 'x' && attribute_name[1] == 'm'
            && attribute_name[2] == 'l' && attribute_name[3] == 'n'
            && attribute_name[4] == 's')
        {
            if (attribute_name[5] == '\0')
                is_namespace = FT_TRUE;
            else if (attribute_name[5] == ':' && attribute_name[6] != '\0')
            {
                const char *prefix_end;

                is_namespace = FT_TRUE;
                namespace_prefix_pointer = attribute_name + 6;
                prefix_end = namespace_prefix_pointer;
                while (*prefix_end)
                    prefix_end++;
                namespace_prefix_length = static_cast<ft_size_t>(prefix_end
                        - namespace_prefix_pointer);
            }
            else if (attribute_name[5] == ':' && attribute_name[6] == '\0')
            {
                cma_free(attribute_name);
                if (attribute_value)
                    cma_free(attribute_value);
                if (error_code_out)
                    *error_code_out = FT_ERR_INVALID_ARGUMENT;
                return (FT_ERR_INTERNAL);
            }
        }
        if (is_namespace)
        {
            if (!has_value || !attribute_value)
            {
                cma_free(attribute_name);
                if (attribute_value)
                    cma_free(attribute_value);
                if (error_code_out)
                    *error_code_out = FT_ERR_INVALID_ARGUMENT;
                return (FT_ERR_INTERNAL);
            }
            if (xml_add_namespace_binding(node, namespace_prefix_pointer,
                    namespace_prefix_length, attribute_value, &active_scope) != 0)
            {
                cma_free(attribute_name);
                cma_free(attribute_value);
                if (error_code_out)
                    *error_code_out = FT_ERR_NO_MEMORY;
                return (FT_ERR_INTERNAL);
            }
        }
        node->attributes.insert(attribute_name, attribute_value);
        int32_t attribute_error = node->attributes.get_error();
        if (attribute_error != FT_ERR_SUCCESS)
        {
            if (attribute_value)
                cma_free(attribute_value);
            cma_free(attribute_name);
            if (error_code_out)
                *error_code_out = attribute_error;
            return (FT_ERR_INTERNAL);
        }
    }
    if (out_scope)
        *out_scope = active_scope;
    if (error_code_out)
        *error_code_out = FT_ERR_SUCCESS;
    return (FT_ERR_SUCCESS);
}

xml_node::xml_node() noexcept
    : mutex(ft_nullptr), name(ft_nullptr), namespace_prefix(ft_nullptr),
    local_name(ft_nullptr), namespace_uri(ft_nullptr), namespace_bindings(ft_nullptr), text(ft_nullptr),
    children(), attributes()
{
    return ;
}

xml_node::~xml_node() noexcept
{
    if (this->children.is_initialised() == FT_CLASS_STATE_INITIALISED)
    {
        ft_size_t child_index = 0;
        ft_size_t child_size = this->children.size();

        while (child_index < child_size)
        {
            xml_node *child = this->children[child_index];
            if (child)
                delete child;
            child_index++;
        }
    }
    if (this->attributes.is_initialised() == FT_CLASS_STATE_INITIALISED)
    {
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
        const xml_namespace_entry *scope_head, int32_t *error_code_out)
{
    if (error_code_out)
        *error_code_out = FT_ERR_SUCCESS;
    string = skip_whitespace(string);
    if (!string || *string != '<')
    {
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    const char *name_start = string;
    while (*string && *string != '>' && *string != ' ' && *string != '/')
        string++;
    ft_size_t name_length = static_cast<ft_size_t>(string - name_start);
    char *name = duplicate_range(name_start, name_length);
    if (!name)
    {
        if (error_code_out)
            *error_code_out = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    xml_node *node = new (std::nothrow) xml_node();
    if (!node)
    {
        cma_free(name);
        if (error_code_out)
            *error_code_out = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    {
        int32_t children_initialize_error;
        int32_t attributes_initialize_error;

        children_initialize_error = node->children.initialize();
        if (children_initialize_error != FT_ERR_SUCCESS)
        {
            cma_free(name);
            delete node;
            if (error_code_out)
                *error_code_out = children_initialize_error;
            return (ft_nullptr);
        }
        attributes_initialize_error = node->attributes.initialize();
        if (attributes_initialize_error != FT_ERR_SUCCESS)
        {
            cma_free(name);
            delete node;
            if (error_code_out)
                *error_code_out = attributes_initialize_error;
            return (ft_nullptr);
        }
    }
    node->name = name;
    const char *colon_position = ft_strchr(node->name, ':');
    if (colon_position)
    {
        ft_size_t prefix_length = static_cast<ft_size_t>(colon_position - node->name);
        if (prefix_length == 0 || colon_position[1] == '\0')
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        node->namespace_prefix = duplicate_range(node->name, prefix_length);
        if (!node->namespace_prefix)
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        node->local_name = duplicate_range(colon_position + 1,
                static_cast<ft_size_t>(ft_strlen(colon_position + 1)));
        if (!node->local_name)
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    else
    {
        node->namespace_prefix = ft_nullptr;
        node->local_name = duplicate_range(node->name,
                static_cast<ft_size_t>(ft_strlen(node->name)));
        if (!node->local_name)
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    int32_t prepare_result = xml_node_prepare_thread_safety(node);
    if (prepare_result != 0)
    {
        delete node;
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    char quote_character = '\0';

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
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    const char *tag_end = string;
    int32_t self_closing = 0;
    const char *attribute_start = name_start + name_length;
    const char *attribute_end = tag_end;
    if (attribute_end > attribute_start)
    {
        const char *trim_cursor = attribute_end;

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
        int32_t attribute_error = FT_ERR_SUCCESS;

        if (xml_parse_attributes(node, attribute_start, attribute_end,
                scope_head, &active_scope, &attribute_error) != 0)
        {
            delete node;
            if (error_code_out)
            {
                int32_t reported_error;

                if (attribute_error != FT_ERR_SUCCESS)
                    reported_error = attribute_error;
                else
                    reported_error = FT_ERR_INVALID_ARGUMENT;
                *error_code_out = reported_error;
            }
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
            if (error_code_out)
                *error_code_out = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
    }
    else
        node->namespace_uri = xml_resolve_namespace_uri(active_scope, ft_nullptr);
    string++;
    if (self_closing)
    {
        *out_node = node;
        if (error_code_out)
            *error_code_out = FT_ERR_SUCCESS;
        return (string);
    }
    const char *text_start = string;
    if (*string == '<' && string[1] == '/')
        ;
    else if (*string == '<')
    {
        while (1)
        {
            xml_node *child = ft_nullptr;
            int32_t child_error = FT_ERR_SUCCESS;

            string = parse_node(string, &child, active_scope, &child_error);
            if (!string)
            {
                delete node;
                if (error_code_out)
                {
                    int32_t reported_error;

                    if (child_error != FT_ERR_SUCCESS)
                        reported_error = child_error;
                    else
                        reported_error = FT_ERR_INVALID_ARGUMENT;
                    *error_code_out = reported_error;
                }
                return (ft_nullptr);
            }
            node->children.push_back(child);
            int32_t children_error_code = node->children.get_error();
            if (children_error_code != FT_ERR_SUCCESS)
            {
                delete child;
                delete node;
                if (error_code_out)
                    *error_code_out = translate_vector_error(children_error_code);
                return (ft_nullptr);
            }
            string = skip_whitespace(string);
            if (*string == '<' && string[1] == '/')
                break ;
        }
    }
    else
    {
        while (*string && !(*string == '<' && string[1] == '/'))
            string++;
        ft_size_t text_length = static_cast<ft_size_t>(string - text_start);
        node->text = duplicate_range(text_start, text_length);
        if (!node->text && text_length > 0)
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    if (*string != '<' || string[1] != '/')
    {
        delete node;
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string += 2;
    string = skip_whitespace(string);
    const char *closing_name_start = string;
    while (*string && *string != '>' && !is_xml_whitespace(*string))
        string++;
    ft_size_t closing_name_length = static_cast<ft_size_t>(string - closing_name_start);
    ft_size_t opening_name_length = ft_strlen(node->name);
    if (closing_name_length != opening_name_length)
    {
        delete node;
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_size_t name_index = 0;
    while (name_index < closing_name_length)
    {
        if (closing_name_start[name_index] != node->name[name_index])
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        name_index++;
    }
    while (*string && *string != '>')
    {
        if (!is_xml_whitespace(*string))
        {
            delete node;
            if (error_code_out)
                *error_code_out = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        string++;
    }
    if (*string != '>')
    {
        delete node;
        if (error_code_out)
            *error_code_out = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    *out_node = node;
    if (error_code_out)
        *error_code_out = FT_ERR_SUCCESS;
    return (string);
}


xml_document::xml_document() noexcept
    : _root(ft_nullptr), _mutex(ft_nullptr),
    _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

xml_document::~xml_document() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t xml_document::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "xml_document::initialize", "called while object is already initialised");
    this->_root = ft_nullptr;
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t xml_document::initialize(const xml_document &other) noexcept
{
    char *serialized_content;
    int32_t load_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "xml_document::initialize(copy)", "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    serialized_content = other.write_to_string();
    if (serialized_content == ft_nullptr)
        return (FT_ERR_INTERNAL);
    load_error = this->load_from_string(serialized_content);
    cma_free(serialized_content);
    return (load_error);
}

int32_t xml_document::initialize(xml_document &&other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "xml_document::initialize(move)", "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    return (this->move(other));
}

int32_t xml_document::move(xml_document &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "xml_document::move", "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_root = ft_nullptr;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_root = other._root;
    this->_mutex = other._mutex;
    other._root = ft_nullptr;
    other._mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t xml_document::destroy() noexcept
{
    int32_t disable_error;
    int32_t lock_error;
    ft_bool lock_acquired;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESS && this->_root != ft_nullptr)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    if (lock_acquired)
        this->unlock(FT_TRUE);
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

void xml_document::record_operation_error(int32_t error_code) const noexcept
{
    (void)xml_document::set_error(static_cast<uint32_t>(error_code));
    return ;
}

int32_t xml_document::load_from_string(const char *xml) noexcept
{
    int32_t lock_error;
    int32_t parse_error;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::load_from_string");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->record_operation_error(lock_error);
        return (lock_error);
    }
    if (!xml)
    {
        this->unlock(lock_acquired);
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (this->_root)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    xml_node *node = ft_nullptr;
    parse_error = FT_ERR_SUCCESS;
    const char *end = parse_node(xml, &node, ft_nullptr, &parse_error);
    if (!end)
    {
        int32_t error_code;

        error_code = parse_error;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->unlock(lock_acquired);
        this->record_operation_error(error_code);
        return (error_code);
    }
    const char *remaining = skip_whitespace(end);
    if (remaining && *remaining != '\0')
    {
        delete node;
        this->unlock(lock_acquired);
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_root = node;
    this->unlock(lock_acquired);
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

static int32_t read_file_content(const char *file_path, char **out_content)
{
    errno = 0;
    FILE *file = std::fopen(file_path, "rb");
    if (!file)
    {
        int32_t error_code;

        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (error_code);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        int32_t error_code;

        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        return (error_code);
    }
    int64_t size = std::ftell(file);
    if (size <= 0)
    {
        std::fclose(file);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_SET) != 0)
    {
        int32_t error_code;

        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        return (error_code);
    }
    char *buffer = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(size) + 1));
    if (!buffer)
    {
        std::fclose(file);
        return (FT_ERR_NO_MEMORY);
    }
    errno = 0;
    ft_size_t read_size = std::fread(buffer, 1, static_cast<ft_size_t>(size), file);
    std::fclose(file);
    if (read_size != static_cast<ft_size_t>(size))
    {
        cma_free(buffer);
        int32_t error_code;

        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (error_code);
    }
    buffer[size] = '\0';
    *out_content = buffer;
    return (FT_ERR_SUCCESS);
}

int32_t xml_document::load_from_file(const char *file_path) noexcept
{
    char *content;
    int32_t   result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::load_from_file");
    content = ft_nullptr;
    result = read_file_content(file_path, &content);
    if (result != FT_ERR_SUCCESS)
    {
        this->record_operation_error(result);
        return (result);
    }
    result = this->load_from_string(content);
    cma_free(content);
    if (result == FT_ERR_SUCCESS)
        this->record_operation_error(FT_ERR_SUCCESS);
    return (result);
}

int32_t xml_document::load_from_backend(ft_document_source &source) noexcept
{
    ft_string content;
    int32_t read_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::load_from_backend");
    read_result = content.initialize();
    if (read_result != FT_ERR_SUCCESS)
    {
        this->record_operation_error(read_result);
        return (read_result);
    }
    read_result = source.read_all(content);
    if (read_result != FT_ERR_SUCCESS)
    {
        this->record_operation_error(read_result);
        return (read_result);
    }
    return (this->load_from_string(content.c_str()));
}

static void append_string(ft_vector<char> &buffer, const char *string)
{
    if (!string)
        return ;
    ft_size_t index = 0;
    ft_size_t length = ft_strlen(string);
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

static int32_t write_node(const xml_node *node, ft_vector<char> &buffer)
{
    if (!node)
        return (FT_ERR_INVALID_ARGUMENT);
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_status = xml_node_lock(node, &lock_acquired);

    if (lock_status != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    buffer.push_back('<');
    append_node_qualified_name(node, buffer);
    buffer.push_back('>');
    if (node->text)
        append_string(buffer, node->text);
    ft_size_t index = 0;
    ft_size_t size = node->children.size();
    while (index < size)
    {
        int32_t child_status = write_node(node->children[index], buffer);

        if (child_status != FT_ERR_SUCCESS)
        {
            xml_node_unlock(node, lock_acquired);
            return (child_status);
        }
        index++;
    }
    buffer.push_back('<');
    buffer.push_back('/');
    append_node_qualified_name(node, buffer);
    buffer.push_back('>');
    xml_node_unlock(node, lock_acquired);
    return (FT_ERR_SUCCESS);
}

char *xml_document::write_to_string() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::write_to_string");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->record_operation_error(lock_error);
        return (ft_nullptr);
    }
    if (!this->_root)
    {
        this->unlock(lock_acquired);
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char *result = ft_nullptr;
    int32_t error_code = FT_ERR_SUCCESS;
    {
        ft_vector<char> buffer;
        int32_t initialize_status;
        int32_t write_status;
        int32_t buffer_error;

        initialize_status = buffer.initialize();
        if (initialize_status != FT_ERR_SUCCESS)
            error_code = initialize_status;
        if (error_code == FT_ERR_SUCCESS)
            write_status = write_node(this->_root, buffer);
        else
            write_status = FT_ERR_SUCCESS;
        if (write_status != FT_ERR_SUCCESS)
            error_code = write_status;
        if (error_code == FT_ERR_SUCCESS)
        {
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESS)
        {
            buffer.push_back('\n');
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESS)
        {
            buffer.push_back('\0');
            buffer_error = buffer.get_error();
            if (buffer_error != FT_ERR_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == FT_ERR_SUCCESS)
        {
            ft_size_t length = buffer.size();
            result = static_cast<char *>(cma_malloc(length));
            if (!result)
                error_code = FT_ERR_NO_MEMORY;
            else
            {
                ft_size_t index = 0;
                while (index < length)
                {
                    result[index] = buffer[index];
                    index++;
                }
            }
        }
    }
    if (error_code != FT_ERR_SUCCESS)
    {
        if (result)
            cma_free(result);
        this->unlock(lock_acquired);
        this->record_operation_error(error_code);
        return (ft_nullptr);
    }
    this->unlock(lock_acquired);
    this->record_operation_error(FT_ERR_SUCCESS);
    return (result);
}

int32_t xml_document::write_to_file(const char *file_path) const noexcept
{
    char *content;
    int32_t   error_code;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::write_to_file");
    content = this->write_to_string();
    if (!content)
    {
        error_code = this->get_error();
        this->record_operation_error(error_code);
        return (error_code);
    }
    errno = 0;
    FILE *file = std::fopen(file_path, "wb");
    if (!file)
    {
        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->record_operation_error(error_code);
        cma_free(content);
        return (error_code);
    }
    ft_size_t length = ft_strlen(content);
    errno = 0;
    ft_size_t written = std::fwrite(content, 1, length, file);
    if (written != length)
    {
        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        this->record_operation_error(error_code);
        cma_free(content);
        return (error_code);
    }
    errno = 0;
    if (std::fclose(file) != 0)
    {
        if (errno != 0)
            error_code = cmp_map_system_error_to_ft(errno);
        else
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->record_operation_error(error_code);
        cma_free(content);
        return (error_code);
    }
    cma_free(content);
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t xml_document::write_to_backend(ft_document_sink &sink) const noexcept
{
    char *content;
    ft_size_t length;
    int32_t write_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::write_to_backend");
    content = this->write_to_string();
    if (!content)
        return (this->get_error());
    length = ft_strlen(content);
    write_result = sink.write_all(content, length);
    cma_free(content);
    int32_t final_error = write_result;
    if (final_error != FT_ERR_SUCCESS)
    {
        this->record_operation_error(final_error);
        return (final_error);
    }
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

xml_node *xml_document::get_root() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::get_root");
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->record_operation_error(lock_error);
        return (ft_nullptr);
    }
    this->unlock(lock_acquired);
    this->record_operation_error(FT_ERR_SUCCESS);
    return (this->_root);
}

void xml_document::set_manual_error(int32_t error_code) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::set_manual_error");
    this->record_operation_error(error_code);
    return ;
}

int32_t xml_document::get_error() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "xml_document::get_error");
    if (this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (xml_document::_last_error);
    lock_acquired = FT_FALSE;
    lock_error = this->lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->unlock(lock_acquired);
    return (xml_document::_last_error);
}

const char *xml_document::get_error_str() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "xml_document::get_error_str");
    int32_t error_code = this->get_error();
    const char *message = ft_strerror(error_code);

    if (message == ft_nullptr)
        message = "unknown error";
    return (message);
}

int32_t xml_document::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::enable_thread_safety");
    if (this->_mutex)
    {
        this->record_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (!mutex_pointer)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->record_operation_error(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t xml_document::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::disable_thread_safety");
    if (!this->_mutex)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->record_operation_error(destroy_error);
    return (destroy_error);
}

ft_bool xml_document::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t xml_document::lock(ft_bool *lock_acquired) const noexcept
{
    ft_bool has_mutex;
    int32_t mutex_result;
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::lock");
    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    has_mutex = (this->_mutex != ft_nullptr);
    mutex_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    int32_t reported_error;

    reported_error = mutex_result;
    if (reported_error != FT_ERR_SUCCESS)
    {
        this->record_operation_error(reported_error);
        return (reported_error);
    }
    if (lock_acquired && has_mutex)
        *lock_acquired = FT_TRUE;
    this->record_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void xml_document::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "xml_document::unlock");
    if (!lock_acquired)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->record_operation_error(FT_ERR_SUCCESS);
    return ;
}
