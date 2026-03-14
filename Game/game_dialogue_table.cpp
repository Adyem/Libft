#include "../PThread/pthread_internal.hpp"
#include "game_dialogue_table.hpp"
#include "../Printf/printf.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local uint32_t game_dialogue_table::_last_error = FT_ERR_SUCCESS;

static ft_sharedptr<game_dialogue_line> game_dialogue_table_clone_line(
    const game_dialogue_line &line)
{
    game_dialogue_line *cloned_line;
    ft_vector<int32_t> copied_next_ids;

    cloned_line = new (std::nothrow) game_dialogue_line();
    if (cloned_line == ft_nullptr)
        return (ft_sharedptr<game_dialogue_line>());
    cloned_line->set_line_id(line.get_line_id());
    cloned_line->set_speaker(line.get_speaker());
    cloned_line->set_text(line.get_text());
    copied_next_ids.clear();
    {
        const int32_t *entry;
        const int32_t *entry_end;

        entry = line.get_next_line_ids().begin();
        entry_end = line.get_next_line_ids().end();
        while (entry != entry_end)
        {
            copied_next_ids.push_back(*entry);
            ++entry;
        }
    }
    cloned_line->set_next_line_ids(copied_next_ids);
    return (ft_sharedptr<game_dialogue_line>(cloned_line));
}

game_dialogue_table::game_dialogue_table() noexcept
    : _lines(), _scripts(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_dialogue_table::game_dialogue_table(const game_dialogue_table &other) noexcept
    : _lines(), _scripts(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_table::game_dialogue_table(copy)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
    }
    return ;
}

game_dialogue_table::game_dialogue_table(game_dialogue_table &&other) noexcept
    : _lines(), _scripts(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_table::game_dialogue_table(move)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

game_dialogue_table::~game_dialogue_table() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    (void)this->destroy();
    return ;
}

uint32_t game_dialogue_table::set_error(uint32_t error_code) noexcept
{
    game_dialogue_table::_last_error = error_code;
    return (error_code);
}

int32_t game_dialogue_table::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_dialogue_table::get_error");
    return (game_dialogue_table::_last_error);
}

const char *game_dialogue_table::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_dialogue_table::get_error_str");
    return (ft_strerror(this->get_error()));
}

int32_t game_dialogue_table::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_dialogue_table::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    int32_t lines_error = this->_lines.initialize();
    if (lines_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(lines_error);
        return (lines_error);
    }
    int32_t scripts_error = this->_scripts.initialize();
    if (scripts_error != FT_ERR_SUCCESS)
    {
        (void)this->_lines.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(scripts_error);
        return (scripts_error);
    }
    this->_lines.clear();
    this->_scripts.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::initialize(const game_dialogue_table &other) noexcept
{
    int32_t initialize_error;
    ft_size_t lines_count;
    ft_size_t scripts_count;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *line_entry;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *lines_end;
    const Pair<int32_t, game_dialogue_script> *script_entry;
    const Pair<int32_t, game_dialogue_script> *scripts_end;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_dialogue_table::initialize(copy)",
            "source object is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->set_error(initialize_error);
            return (initialize_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
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
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::initialize(game_dialogue_table &&other) noexcept
{
    return (this->initialize(static_cast<const game_dialogue_table &>(other)));
}

int32_t game_dialogue_table::move(game_dialogue_table &other) noexcept
{
    return (this->initialize(static_cast<game_dialogue_table &&>(other)));
}

int32_t game_dialogue_table::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_lines.clear();
    this->_scripts.clear();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_dialogue_table::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::disable_thread_safety() noexcept
{
    int32_t destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_dialogue_table::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_dialogue_table::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::lock");
    return (this->lock_internal(lock_acquired));
}

void game_dialogue_table::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_dialogue_table::register_line(const game_dialogue_line &line) noexcept
{
    ft_bool lock_acquired;
    ft_sharedptr<game_dialogue_line> stored_line;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::register_line");
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    stored_line = game_dialogue_table_clone_line(line);
    if (stored_line == ft_sharedptr<game_dialogue_line>())
    {

        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    this->_lines.insert(line.get_line_id(), stored_line);
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::register_script(const game_dialogue_script &script) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::register_script");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_scripts.insert(script.get_script_id(), script);
    (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::fetch_line(int32_t line_id,
    game_dialogue_line &out_line) const noexcept
{
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *entry;
    ft_sharedptr<game_dialogue_line> stored_line;
    ft_vector<int32_t> copied_next_ids;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::fetch_line");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    entry = this->_lines.find(line_id);
    if (entry == this->_lines.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);

        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_NOT_FOUND);
    }
    stored_line = entry->value;
    if (stored_line == ft_sharedptr<game_dialogue_line>())
    {
        this->set_error(FT_ERR_NOT_FOUND);

        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_NOT_FOUND);
    }
    out_line.set_line_id(stored_line->get_line_id());
    out_line.set_speaker(stored_line->get_speaker());
    out_line.set_text(stored_line->get_text());
    copied_next_ids.clear();
    {
        const int32_t *next_line_id;
        const int32_t *next_line_end;

        next_line_id = stored_line->get_next_line_ids().begin();
        next_line_end = stored_line->get_next_line_ids().end();
        while (next_line_id != next_line_end)
        {
            copied_next_ids.push_back(*next_line_id);
            ++next_line_id;
        }
    }
    out_line.set_next_line_ids(copied_next_ids);
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_dialogue_table::fetch_script(int32_t script_id,
    game_dialogue_script &out_script) const noexcept
{
    const Pair<int32_t, game_dialogue_script> *entry;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::fetch_script");
    ft_bool lock_acquired;
    int32_t lock_error;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    entry = this->_scripts.find(script_id);
    if (entry == this->_scripts.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);

        (void)this->unlock_internal(lock_acquired);
        return (FT_ERR_NOT_FOUND);
    }
    int32_t initialize_error = out_script.initialize(entry->value);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);

        (void)this->unlock_internal(lock_acquired);
        return (initialize_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &game_dialogue_table::get_lines() noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::get_lines");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_lines);
}

const ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &game_dialogue_table::get_lines() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::get_lines const");
    const_cast<game_dialogue_table *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_lines);
}

void game_dialogue_table::set_lines(
    const ft_map<int32_t, ft_sharedptr<game_dialogue_line> > &lines) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *entry;
    const Pair<int32_t, ft_sharedptr<game_dialogue_line> > *entry_end;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::set_lines");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
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
    this->set_error(FT_ERR_SUCCESS);

    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int32_t, game_dialogue_script> &game_dialogue_table::get_scripts() noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::get_scripts");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_scripts);
}

const ft_map<int32_t, game_dialogue_script> &game_dialogue_table::get_scripts() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::get_scripts const");
    const_cast<game_dialogue_table *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_scripts);
}

void game_dialogue_table::set_scripts(
    const ft_map<int32_t, game_dialogue_script> &scripts) noexcept
{
    ft_bool lock_acquired;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_dialogue_script> *entry;
    const Pair<int32_t, game_dialogue_script> *entry_end;

    errno_abort_if_uninitialised(this->_initialised_state, "game_dialogue_table::set_scripts");
    int32_t lock_error;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
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
        this->set_error(FT_ERR_SUCCESS);
    return ;
}
