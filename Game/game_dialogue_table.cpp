#include "game_dialogue_table.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static ft_sharedptr<ft_dialogue_line> game_dialogue_table_clone_line(
    const ft_dialogue_line &line)
{
    ft_dialogue_line *cloned_line;
    ft_vector<int> copied_next_ids;

    cloned_line = new (std::nothrow) ft_dialogue_line();
    if (cloned_line == ft_nullptr)
        return (ft_sharedptr<ft_dialogue_line>());
    cloned_line->set_line_id(line.get_line_id());
    cloned_line->set_speaker(line.get_speaker());
    cloned_line->set_text(line.get_text());
    copied_next_ids.clear();
    {
        const int *entry;
        const int *entry_end;

        entry = line.get_next_line_ids().begin();
        entry_end = line.get_next_line_ids().end();
        while (entry != entry_end)
        {
            copied_next_ids.push_back(*entry);
            ++entry;
        }
    }
    cloned_line->set_next_line_ids(copied_next_ids);
    return (ft_sharedptr<ft_dialogue_line>(cloned_line));
}

ft_dialogue_table::ft_dialogue_table() noexcept
    : _lines(), _scripts(), _mutex(ft_nullptr),
      _initialized_state(ft_dialogue_table::_state_uninitialized)
{
    return ;
}

ft_dialogue_table::~ft_dialogue_table() noexcept
{
    if (this->_initialized_state == ft_dialogue_table::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_dialogue_table::~ft_dialogue_table",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_dialogue_table::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dialogue_table::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dialogue_table lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_dialogue_table::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dialogue_table::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dialogue_table::initialize() noexcept
{
    if (this->_initialized_state == ft_dialogue_table::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dialogue_table::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_lines.clear();
    this->_scripts.clear();
    this->_initialized_state = ft_dialogue_table::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::initialize(const ft_dialogue_table &other) noexcept
{
    int initialize_error;
    size_t lines_count;
    size_t scripts_count;
    size_t index;
    const Pair<int, ft_sharedptr<ft_dialogue_line> > *line_entry;
    const Pair<int, ft_sharedptr<ft_dialogue_line> > *lines_end;
    const Pair<int, ft_dialogue_script> *script_entry;
    const Pair<int, ft_dialogue_script> *scripts_end;

    if (other._initialized_state != ft_dialogue_table::_state_initialized)
    {
        other.abort_lifecycle_error("ft_dialogue_table::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lines_count = other._lines.size();
    lines_end = other._lines.end();
    line_entry = lines_end - lines_count;
    index = 0;
    while (index < lines_count)
    {
        this->_lines.insert(line_entry->key, line_entry->value);
        line_entry++;
        index += 1;
    }
    scripts_count = other._scripts.size();
    scripts_end = other._scripts.end();
    script_entry = scripts_end - scripts_count;
    index = 0;
    while (index < scripts_count)
    {
        this->_scripts.insert(script_entry->key, script_entry->value);
        script_entry++;
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::initialize(ft_dialogue_table &&other) noexcept
{
    return (this->initialize(static_cast<const ft_dialogue_table &>(other)));
}

int ft_dialogue_table::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_dialogue_table::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dialogue_table::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_lines.clear();
    this->_scripts.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dialogue_table::_state_destroyed;
    return (disable_error);
}

int ft_dialogue_table::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dialogue_table::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
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

int ft_dialogue_table::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dialogue_table::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_dialogue_table::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_dialogue_table::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dialogue_table::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_dialogue_table::register_line(const ft_dialogue_line &line) noexcept
{
    bool lock_acquired;
    ft_sharedptr<ft_dialogue_line> stored_line;

    this->abort_if_not_initialized("ft_dialogue_table::register_line");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    stored_line = game_dialogue_table_clone_line(line);
    if (stored_line == ft_sharedptr<ft_dialogue_line>())
    {
        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_NO_MEMORY);
    }
    this->_lines.insert(line.get_line_id(), stored_line);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::register_script(const ft_dialogue_script &script) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_table::register_script");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_scripts.insert(script.get_script_id(), script);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::fetch_line(int line_id,
    ft_dialogue_line &out_line) const noexcept
{
    const Pair<int, ft_sharedptr<ft_dialogue_line> > *entry;
    ft_sharedptr<ft_dialogue_line> stored_line;
    ft_vector<int> copied_next_ids;

    this->abort_if_not_initialized("ft_dialogue_table::fetch_line");
    entry = this->_lines.find(line_id);
    if (entry == this->_lines.end())
        return (FT_ERR_NOT_FOUND);
    stored_line = entry->value;
    if (stored_line == ft_sharedptr<ft_dialogue_line>())
        return (FT_ERR_NOT_FOUND);
    out_line.set_line_id(stored_line->get_line_id());
    out_line.set_speaker(stored_line->get_speaker());
    out_line.set_text(stored_line->get_text());
    copied_next_ids.clear();
    {
        const int *next_line_id;
        const int *next_line_end;

        next_line_id = stored_line->get_next_line_ids().begin();
        next_line_end = stored_line->get_next_line_ids().end();
        while (next_line_id != next_line_end)
        {
            copied_next_ids.push_back(*next_line_id);
            ++next_line_id;
        }
    }
    out_line.set_next_line_ids(copied_next_ids);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_table::fetch_script(int script_id,
    ft_dialogue_script &out_script) const noexcept
{
    const Pair<int, ft_dialogue_script> *entry;

    this->abort_if_not_initialized("ft_dialogue_table::fetch_script");
    entry = this->_scripts.find(script_id);
    if (entry == this->_scripts.end())
        return (FT_ERR_NOT_FOUND);
    return (out_script.initialize(entry->value));
}

ft_map<int, ft_sharedptr<ft_dialogue_line> > &ft_dialogue_table::get_lines() noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::get_lines");
    return (this->_lines);
}

const ft_map<int, ft_sharedptr<ft_dialogue_line> > &ft_dialogue_table::get_lines() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::get_lines const");
    return (this->_lines);
}

void ft_dialogue_table::set_lines(
    const ft_map<int, ft_sharedptr<ft_dialogue_line> > &lines) noexcept
{
    bool lock_acquired;
    size_t count;
    size_t index;
    const Pair<int, ft_sharedptr<ft_dialogue_line> > *entry;
    const Pair<int, ft_sharedptr<ft_dialogue_line> > *entry_end;

    this->abort_if_not_initialized("ft_dialogue_table::set_lines");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_lines.clear();
    count = lines.size();
    entry_end = lines.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_lines.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int, ft_dialogue_script> &ft_dialogue_table::get_scripts() noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::get_scripts");
    return (this->_scripts);
}

const ft_map<int, ft_dialogue_script> &ft_dialogue_table::get_scripts() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::get_scripts const");
    return (this->_scripts);
}

void ft_dialogue_table::set_scripts(
    const ft_map<int, ft_dialogue_script> &scripts) noexcept
{
    bool lock_acquired;
    size_t count;
    size_t index;
    const Pair<int, ft_dialogue_script> *entry;
    const Pair<int, ft_dialogue_script> *entry_end;

    this->abort_if_not_initialized("ft_dialogue_table::set_scripts");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_scripts.clear();
    count = scripts.size();
    entry_end = scripts.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_scripts.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_dialogue_table::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_table::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
