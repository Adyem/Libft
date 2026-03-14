#include "document.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"

#include <cstdio>
#include <cstddef>
#include <new>

#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"

static thread_local int32_t g_json_document_last_error = FT_ERR_SUCCESS;
static thread_local pt_recursive_mutex *g_json_document_locked_mutex = ft_nullptr;

static void json_document_set_error(int32_t error_code)
{
    g_json_document_last_error = error_code;
    return ;
}

#define JSON_DOCUMENT_ERROR_RETURN(code, value) \
    do { json_document_set_error(code); return (value); } while (0)

#define JSON_DOCUMENT_SUCCESS_RETURN(value) \
    do { json_document_set_error(FT_ERR_SUCCESS); return (value); } while (0)

static int32_t json_document_last_error(void)
{
    return (g_json_document_last_error);
}

static char *json_document_unescape_pointer_token(const char *start, ft_size_t length) noexcept
{
    if (!start && length != FT_ERR_SUCCESS)
        JSON_DOCUMENT_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
    ft_size_t allocation_size = length + 1;
    char *token = static_cast<char *>(cma_malloc(allocation_size));
    if (!token)
        JSON_DOCUMENT_ERROR_RETURN(FT_ERR_NO_MEMORY, ft_nullptr);
    ft_size_t input_index = 0;
    ft_size_t output_index = 0;
    while (input_index < length)
    {
        char current_character = start[input_index];
        if (current_character == '~')
        {
            if (input_index + 1 >= length)
            {
                cma_free(token);
                JSON_DOCUMENT_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
            }
            char escape_character = start[input_index + 1];
            if (escape_character == '0')
                token[output_index] = '~';
            else if (escape_character == '1')
                token[output_index] = '/';
            else
            {
                cma_free(token);
                JSON_DOCUMENT_ERROR_RETURN(FT_ERR_INVALID_ARGUMENT, ft_nullptr);
            }
            input_index += 2;
            output_index += 1;
            continue;
        }
        token[output_index] = current_character;
        output_index += 1;
        input_index += 1;
    }
    token[output_index] = '\0';
    JSON_DOCUMENT_SUCCESS_RETURN(token);
}

static void json_document_finalize_guard() noexcept
{
    int32_t operation_error = json_document_last_error();

    if (g_json_document_locked_mutex != ft_nullptr)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(g_json_document_locked_mutex);
        g_json_document_locked_mutex = ft_nullptr;
    }
    if (operation_error != FT_ERR_SUCCESS)
    {
        json_document_set_error(operation_error);
        return ;
    }
    json_document_set_error(FT_ERR_SUCCESS);
    return ;
}

void json_document::set_error_unlocked(int32_t error_code) const noexcept
{
    json_document_set_error(error_code);
    this->_error_code = error_code;
    return ;
}

void json_document::set_error(int32_t error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

void json_document::clear_unlocked() noexcept
{
    json_free_groups(this->_groups);
    this->_groups = ft_nullptr;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

char *json_document::write_to_string_unlocked() const noexcept
{
    char *result;

    result = json_write_to_string(this->_groups);
    if (!result)
    {
        int32_t current_error;

        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (result);
}

json_document::json_document() noexcept
    : _groups(ft_nullptr)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

json_document::json_document(const json_document &other) noexcept
    : _groups(ft_nullptr)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(other);
    return ;
}

json_document::json_document(json_document &&other) noexcept
    : _groups(ft_nullptr)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex(ft_nullptr)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(static_cast<json_document &&>(other));
    return ;
}

json_document::~json_document() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t json_document::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "json_document::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t json_document::initialize(const json_document &other) noexcept
{
    int32_t operation_error;
    char *serialized_content;
    int32_t current_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "json_document::initialize(const json_document &) source",
            "source is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        operation_error = this->destroy();
        if (operation_error != FT_ERR_SUCCESS)
            return (operation_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    operation_error = this->initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    serialized_content = other.write_to_string();
    if (serialized_content == ft_nullptr)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        (void)this->destroy();
        return (current_error);
    }
    operation_error = this->read_from_string(serialized_content);
    cma_free(serialized_content);
    if (operation_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (operation_error);
    }
    if (other._mutex != ft_nullptr)
    {
        operation_error = this->enable_thread_safety();
        if (operation_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            return (operation_error);
        }
    }
    return (FT_ERR_SUCCESS);
}

int32_t json_document::initialize(json_document &&other) noexcept
{
    int32_t operation_error;
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "json_document::initialize(json_document &&) source",
            "source is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        operation_error = this->destroy();
        if (operation_error != FT_ERR_SUCCESS)
            return (operation_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    operation_error = this->initialize();
    if (operation_error != FT_ERR_SUCCESS)
        return (operation_error);
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (lock_error);
    }
    if (other._mutex != ft_nullptr)
    {
        operation_error = this->enable_thread_safety();
        if (operation_error != FT_ERR_SUCCESS)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
            (void)this->destroy();
            return (operation_error);
        }
    }
    this->_groups = other._groups;
    this->_error_code = other._error_code;
    other._groups = ft_nullptr;
    other._error_code = FT_ERR_SUCCESS;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    return (FT_ERR_SUCCESS);
}

int32_t json_document::destroy() noexcept
{
    int32_t lock_error;
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        this->clear_unlocked();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

uint32_t json_document::move(json_document &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(
            static_cast<json_document &&>(other))));
}

int32_t json_document::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "json_document::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t json_document::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "json_document::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool json_document::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

json_group *json_document::create_group(const char *name) noexcept
{
    int32_t lock_error;
    json_group *group;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    group = json_create_json_group(name);
    if (!group)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (group);
}

json_item *json_document::create_item(const char *key, const char *value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!key || !value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item);
}

json_item *json_document::create_item(const char *key, const ft_big_number &value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item);
}

json_item *json_document::create_item(const char *key, const int32_t value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item);
}

json_item *json_document::create_item(const char *key, const ft_bool value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = json_create_item(key, value);
    if (!item)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item);
}

void json_document::add_item(json_group *group, json_item *item) noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_add_item_to_group(group, item);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::append_group(json_group *group) noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_append_group(&this->_groups, group);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

int32_t json_document::write_to_file(const char *file_path) const noexcept
{
    int32_t lock_error;
    int32_t result;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    if (!file_path)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    result = json_write_to_file(file_path, this->_groups);
    if (result != FT_ERR_SUCCESS)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_INVALID_HANDLE;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (result);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

int32_t json_document::write_to_backend(ft_document_sink &sink) const noexcept
{
    int32_t lock_error;
    char *serialized_content;
    ft_size_t serialized_length;
    int32_t write_result;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    serialized_content = this->write_to_string_unlocked();
    if (!serialized_content)
    {
        current_error = this->_error_code;
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        json_document_finalize_guard();
        return (current_error);
    }
    serialized_length = ft_strlen(serialized_content);
    write_result = sink.write_all(serialized_content, serialized_length);
    cma_free(serialized_content);
    if (write_result != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(write_result);
        json_document_finalize_guard();
        return (write_result);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

char *json_document::write_to_string() const noexcept
{
    int32_t lock_error;
    char *result;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    result = this->write_to_string_unlocked();
    json_document_finalize_guard();
    return (result);
}

int32_t json_document::read_from_file(const char *file_path) noexcept
{
    int32_t lock_error;
    json_group *groups;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    if (!file_path)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->clear_unlocked();
    groups = json_read_from_file(file_path);
    if (!groups)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (current_error);
    }
    this->_groups = groups;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

int32_t json_document::read_from_backend(ft_document_source &source) noexcept
{
    int32_t lock_error;
    json_group *groups;
    int32_t error_code;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    this->clear_unlocked();
    groups = json_read_from_backend(source);
    if (!groups)
    {
        error_code = json_document_last_error();
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(error_code);
        json_document_finalize_guard();
        return (error_code);
    }
    this->_groups = groups;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

int32_t json_document::read_from_file_streaming(const char *file_path, ft_size_t buffer_capacity) noexcept
{
    int32_t lock_error;
    FILE *file;
    json_group *groups;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    if (!file_path || buffer_capacity == FT_ERR_SUCCESS)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    file = fopen(file_path, "rb");
    if (!file)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_IO);
        json_document_finalize_guard();
        return (FT_ERR_IO);
    }
    this->clear_unlocked();
    groups = json_read_from_file_stream(file, buffer_capacity);
    fclose(file);
    if (!groups)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (current_error);
    }
    this->_groups = groups;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

int32_t json_document::read_from_string(const char *content) noexcept
{
    int32_t lock_error;
    json_group *groups;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    if (!content)
    {
        this->clear_unlocked();
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->clear_unlocked();
    groups = json_read_from_string(content);
    if (!groups)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_INVALID_ARGUMENT;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return (current_error);
    }
    this->_groups = groups;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (FT_ERR_SUCCESS);
}

json_group *json_document::find_group(const char *name) const noexcept
{
    int32_t lock_error;
    json_group *group;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    group = json_find_group(this->_groups, name);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (group);
}

json_item *json_document::find_item(json_group *group, const char *key) const noexcept
{
    int32_t lock_error;
    json_item *item;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = json_find_item(group, key);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item);
}

json_item *json_document::find_item_by_pointer_unlocked(const char *pointer) const noexcept
{
    if (!pointer)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (pointer[0] == '\0' || pointer[0] != '/')
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    const char *cursor = pointer;
    json_group *current_group = ft_nullptr;
    ft_bool expecting_group = FT_TRUE;
    while (*cursor)
    {
        if (*cursor != '/')
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        cursor += 1;
        const char *segment_start = cursor;
        while (*cursor && *cursor != '/')
            cursor += 1;
        ft_size_t segment_length = static_cast<ft_size_t>(cursor - segment_start);
        if (segment_length == FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        char *token = json_document_unescape_pointer_token(segment_start, segment_length);
        if (!token)
        {
            this->set_error_unlocked(json_document_last_error());
            return (ft_nullptr);
        }
        if (expecting_group)
        {
            json_group *group_iterator = this->_groups;
            ft_bool found_group = FT_FALSE;
            while (group_iterator)
            {
                const char *group_name = group_iterator->name;
                if (!group_name)
                    group_name = "";
                if (ft_strcmp(group_name, token) == FT_ERR_SUCCESS)
                {
                    found_group = FT_TRUE;
                    break ;
                }
                group_iterator = group_iterator->next;
            }
            cma_free(token);
            if (!found_group)
            {
                this->set_error_unlocked(FT_ERR_NOT_FOUND);
                return (ft_nullptr);
            }
            current_group = group_iterator;
            expecting_group = FT_FALSE;
            if (*cursor == '\0')
            {
                this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                return (ft_nullptr);
            }
            continue;
        }
        json_item *item_iterator = ft_nullptr;
        if (current_group)
            item_iterator = current_group->items;
        while (item_iterator)
        {
            const char *item_key = item_iterator->key;
            if (!item_key)
                item_key = "";
            if (ft_strcmp(item_key, token) == FT_ERR_SUCCESS)
                break ;
            item_iterator = item_iterator->next;
        }
        cma_free(token);
        if (!item_iterator)
        {
            this->set_error_unlocked(FT_ERR_NOT_FOUND);
            return (ft_nullptr);
        }
        if (*cursor != '\0')
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        this->set_error_unlocked(FT_ERR_SUCCESS);
        return (item_iterator);
    }
    this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
    return (ft_nullptr);
}

json_item *json_document::find_item_by_pointer(const char *pointer) const noexcept
{
    int32_t lock_error;
    json_item *item;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = this->find_item_by_pointer_unlocked(pointer);
    json_document_finalize_guard();
    return (item);
}

const char *json_document::get_value_by_pointer(const char *pointer) const noexcept
{
    int32_t lock_error;
    json_item *item;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    item = this->find_item_by_pointer_unlocked(pointer);
    if (!item)
    {
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    if (!item->value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (item->value);
}

void json_document::remove_group(const char *name) noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!name)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_remove_group(&this->_groups, name);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::remove_item(json_group *group, const char *key) noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_remove_item(group, key);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::update_item(json_group *group, const char *key, const char *value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !key || !value)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::update_item(json_group *group, const char *key, const int32_t value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::update_item(json_group *group, const char *key, const ft_bool value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::update_item(json_group *group, const char *key, const ft_big_number &value) noexcept
{
    int32_t lock_error;
    json_item *item;
    int32_t current_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    if (!group || !key)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    item = json_find_item(group, key);
    if (!item)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        json_document_finalize_guard();
        return ;
    }
    json_update_item(group, key, value);
    if (!item->value)
    {
        current_error = json_document_last_error();
        if (current_error == FT_ERR_SUCCESS)
            current_error = FT_ERR_NO_MEMORY;
        this->set_error_unlocked(current_error);
        json_document_finalize_guard();
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return ;
}

void json_document::clear() noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return ;
    }
    this->clear_unlocked();
    json_document_finalize_guard();
    return ;
}

json_group *json_document::get_groups() const noexcept
{
    int32_t lock_error;
    json_group *groups;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_nullptr);
    }
    groups = this->_groups;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    json_document_finalize_guard();
    return (groups);
}

void json_document::set_manual_error(int32_t error_code) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "json_document::set_manual_error");
    this->set_error(error_code);
    return ;
}

int32_t json_document::get_error() const noexcept
{
    int32_t lock_error;
    int32_t error_value;

    errno_abort_if_uninitialised(this->_initialised_state,
        "json_document::get_error");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (lock_error);
    }
    error_value = this->_error_code;
    json_document_finalize_guard();
    return (error_value);
}

const char *json_document::get_error_str() const noexcept
{
    int32_t lock_error;
    const char *error_string;

    errno_abort_if_uninitialised(this->_initialised_state,
        "json_document::get_error_str");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error == FT_ERR_SUCCESS)
        g_json_document_locked_mutex = this->_mutex;
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        json_document_finalize_guard();
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    json_document_finalize_guard();
    return (error_string);
}
