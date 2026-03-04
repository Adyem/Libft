#include "../PThread/pthread_internal.hpp"
#include "ft_dialogue_script.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_dialogue_script::_last_error = FT_ERR_SUCCESS;

static void game_dialogue_copy_next_ids(const ft_vector<int> &source,
    ft_vector<int> &destination)
{
    const int *entry;
    const int *entry_end;

    destination.clear();
    entry = source.begin();
    entry_end = source.end();
    while (entry != entry_end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

static ft_sharedptr<ft_dialogue_line> game_dialogue_clone_line_ptr(
    const ft_sharedptr<ft_dialogue_line> &line)
{
    ft_dialogue_line *cloned_line;
    ft_vector<int> copied_next_ids;

    if (line == ft_sharedptr<ft_dialogue_line>())
        return (ft_sharedptr<ft_dialogue_line>());
    if (copied_next_ids.initialize() != FT_ERR_SUCCESS)
        return (ft_sharedptr<ft_dialogue_line>());
    cloned_line = new (std::nothrow) ft_dialogue_line();
    if (cloned_line == ft_nullptr)
        return (ft_sharedptr<ft_dialogue_line>());
    cloned_line->set_line_id(line->get_line_id());
    cloned_line->set_speaker(line->get_speaker());
    cloned_line->set_text(line->get_text());
    game_dialogue_copy_next_ids(line->get_next_line_ids(), copied_next_ids);
    cloned_line->set_next_line_ids(copied_next_ids);
    return (ft_sharedptr<ft_dialogue_line>(cloned_line));
}

static void game_dialogue_copy_line_vector(
    const ft_vector<ft_sharedptr<ft_dialogue_line> > &source,
    ft_vector<ft_sharedptr<ft_dialogue_line> > &destination)
{
    const ft_sharedptr<ft_dialogue_line> *entry;
    const ft_sharedptr<ft_dialogue_line> *entry_end;

    destination.clear();
    entry = source.begin();
    entry_end = source.end();
    while (entry != entry_end)
    {
        destination.push_back(game_dialogue_clone_line_ptr(*entry));
        ++entry;
    }
    return ;
}

static void game_dialogue_copy_plain_line_vector(const ft_vector<ft_dialogue_line> &source,
    ft_vector<ft_sharedptr<ft_dialogue_line> > &destination)
{
    const ft_dialogue_line *entry;
    const ft_dialogue_line *entry_end;
    ft_dialogue_line *cloned_line;
    ft_vector<int> copied_next_ids;

    if (copied_next_ids.initialize() != FT_ERR_SUCCESS)
        return ;
    destination.clear();
    entry = source.begin();
    entry_end = source.end();
    while (entry != entry_end)
    {
        cloned_line = new (std::nothrow) ft_dialogue_line();
        if (cloned_line == ft_nullptr)
        {
            destination.push_back(ft_sharedptr<ft_dialogue_line>());
            ++entry;
            continue ;
        }
        cloned_line->set_line_id(entry->get_line_id());
        cloned_line->set_speaker(entry->get_speaker());
        cloned_line->set_text(entry->get_text());
        game_dialogue_copy_next_ids(entry->get_next_line_ids(), copied_next_ids);
        cloned_line->set_next_line_ids(copied_next_ids);
        destination.push_back(ft_sharedptr<ft_dialogue_line>(cloned_line));
        ++entry;
    }
    return ;
}

void ft_dialogue_script::set_error(int error_code) const noexcept
{
    ft_dialogue_script::_last_error = error_code;
    return ;
}

ft_dialogue_script::ft_dialogue_script() noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(),
      _mutex(ft_nullptr),
      _initialized_state(ft_dialogue_script::_state_uninitialized)
{
    return ;
}

ft_dialogue_script::~ft_dialogue_script() noexcept
{
    if (this->_initialized_state == ft_dialogue_script::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_dialogue_script::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_dialogue_script lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_dialogue_script::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_dialogue_script::initialize() noexcept
{
    int lines_error;
    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
    {
        this->abort_lifecycle_error("ft_dialogue_script::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_script_id = 0;
    int title_error = this->_title.initialize();
    if (title_error != FT_ERR_SUCCESS)
        return (title_error);
    int summary_error = this->_summary.initialize();
    if (summary_error != FT_ERR_SUCCESS)
    {
        (void)this->_title.destroy();
        this->_initialized_state = ft_dialogue_script::_state_destroyed;
        return (summary_error);
    }
    lines_error = this->_lines.initialize();
    if (lines_error != FT_ERR_SUCCESS)
    {
        (void)this->_title.destroy();
        (void)this->_summary.destroy();
        this->_initialized_state = ft_dialogue_script::_state_destroyed;
        return (lines_error);
    }
    this->_start_line_id = 0;
    this->_initialized_state = ft_dialogue_script::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_script::initialize(const ft_dialogue_script &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_dialogue_script::_state_initialized)
    {
        other.abort_lifecycle_error("ft_dialogue_script::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_script::initialize(ft_dialogue_script &&other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_dialogue_script::_state_initialized)
    {
        other.abort_lifecycle_error("ft_dialogue_script::initialize(move)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_script::initialize(int script_id, const ft_string &title,
    const ft_string &summary, int start_line_id,
    const ft_vector<ft_dialogue_line> &lines) noexcept
{
    int initialize_error;

    if (this->_initialized_state == ft_dialogue_script::_state_initialized)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (initialize_error);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_script_id = script_id;
    this->_title = title;
    this->_summary = summary;
    this->_start_line_id = start_line_id;
    game_dialogue_copy_plain_line_vector(lines, this->_lines);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_script::destroy() noexcept
{
    int lines_error;
    int title_error;
    int summary_error;
    int first_error;
    int disable_error;

    if (this->_initialized_state != ft_dialogue_script::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    first_error = FT_ERR_SUCCESS;
    lines_error = this->_lines.destroy();
    if (first_error == FT_ERR_SUCCESS && lines_error != FT_ERR_SUCCESS)
        first_error = lines_error;
    title_error = this->_title.destroy();
    if (first_error == FT_ERR_SUCCESS && title_error != FT_ERR_SUCCESS)
        first_error = title_error;
    summary_error = this->_summary.destroy();
    if (first_error == FT_ERR_SUCCESS && summary_error != FT_ERR_SUCCESS)
        first_error = summary_error;
    this->_script_id = 0;
    this->_start_line_id = 0;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_dialogue_script::_state_destroyed;
    if (first_error != FT_ERR_SUCCESS)
        return (first_error);
    return (disable_error);
}

int ft_dialogue_script::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_dialogue_script::enable_thread_safety");
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

int ft_dialogue_script::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_dialogue_script::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_dialogue_script::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_script::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_dialogue_script::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_dialogue_script::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::unlock");
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

int ft_dialogue_script::get_script_id() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_script_id");
    return (this->_script_id);
}

void ft_dialogue_script::set_script_id(int script_id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_script::set_script_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_script_id = script_id;
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

const ft_string &ft_dialogue_script::get_title() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_title");
    return (this->_title);
}

void ft_dialogue_script::set_title(const ft_string &title) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_script::set_title");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_title = title;
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

const ft_string &ft_dialogue_script::get_summary() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_summary");
    return (this->_summary);
}

void ft_dialogue_script::set_summary(const ft_string &summary) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_script::set_summary");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_summary = summary;
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

int ft_dialogue_script::get_start_line_id() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_start_line_id");
    return (this->_start_line_id);
}

void ft_dialogue_script::set_start_line_id(int start_line_id) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_script::set_start_line_id");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_start_line_id = start_line_id;
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

const ft_vector<ft_sharedptr<ft_dialogue_line> > &ft_dialogue_script::get_lines() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_lines const");
    return (this->_lines);
}

ft_vector<ft_sharedptr<ft_dialogue_line> > &ft_dialogue_script::get_lines() noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_lines");
    return (this->_lines);
}

void ft_dialogue_script::set_lines(
    const ft_vector<ft_sharedptr<ft_dialogue_line> > &lines) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_dialogue_script::set_lines");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    game_dialogue_copy_line_vector(lines, this->_lines);
    int unlock_error;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_dialogue_script::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_dialogue_script::get_mutex_for_validation");
    return (this->_mutex);
}
#endif

int ft_dialogue_script::get_error() const noexcept
{
    return (ft_dialogue_script::_last_error);
}

const char *ft_dialogue_script::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
