#include "../PThread/pthread_internal.hpp"
#include "ft_dialogue_line.hpp"
#include <new>

static void game_dialogue_copy_int_vector(const ft_vector<int> &source,
    ft_vector<int> &destination)
{
    ft_vector<int>::const_iterator entry;
    ft_vector<int>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

thread_local int ft_dialogue_line::_last_error = FT_ERR_SUCCESS;

int ft_dialogue_line::lock_internal(bool *lock_acquired) const noexcept
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

void ft_dialogue_line::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void ft_dialogue_line::set_error(int error_code) const noexcept
{
    ft_dialogue_line::_last_error = error_code;
    return ;
}

ft_dialogue_line::ft_dialogue_line() noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_dialogue_line::~ft_dialogue_line() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_dialogue_line::initialize(int line_id, const ft_string &speaker,
    const ft_string &text, const ft_vector<int> &next_line_ids) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_line_id = line_id;
    this->_speaker = speaker;
    this->_text = text;
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_dialogue_line::get_line_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int result;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    result = this->_line_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

void ft_dialogue_line::set_line_id(int line_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_line_id = line_id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_dialogue_line::get_speaker() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_string *result;

    result = &this->_speaker;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void ft_dialogue_line::set_speaker(const ft_string &speaker) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_speaker = speaker;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_dialogue_line::get_text() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_string *result;

    result = &this->_text;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void ft_dialogue_line::set_text(const ft_string &text) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_text = text;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_vector<int> &ft_dialogue_line::get_next_line_ids() const noexcept
{
    bool lock_acquired;
    int lock_error;
    const ft_vector<int> *result;

    result = &this->_next_line_ids;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

ft_vector<int> &ft_dialogue_line::get_next_line_ids() noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_vector<int> *result;

    result = &this->_next_line_ids;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*result);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*result);
}

void ft_dialogue_line::set_next_line_ids(const ft_vector<int> &next_line_ids) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_dialogue_line::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

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

int ft_dialogue_line::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_dialogue_line::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_dialogue_line::get_error() const noexcept
{
    return (ft_dialogue_line::_last_error);
}

const char *ft_dialogue_line::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_dialogue_line::get_mutex_for_validation() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif
