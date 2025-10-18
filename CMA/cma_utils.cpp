#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <pthread.h>
#include <random>
#include <sys/mman.h>
#include <unistd.h>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Logger/logger.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"

Page *page_list = ft_nullptr;
ft_size_t    g_cma_alloc_limit = 0;
ft_size_t    g_cma_allocation_count = 0;
ft_size_t    g_cma_free_count = 0;
ft_size_t    g_cma_current_bytes = 0;
ft_size_t    g_cma_peak_bytes = 0;

struct cma_metadata_chunk
{
    unsigned char    *memory;
    ft_size_t    size;
    ft_size_t    used;
    bool        protected_state;
    cma_metadata_chunk    *next;
};

static cma_metadata_chunk    *g_cma_metadata_chunks = ft_nullptr;
static Block    *g_cma_metadata_free_list = ft_nullptr;
static ft_size_t    g_cma_metadata_stride = 0;
static ft_size_t    g_cma_metadata_page_size = 0;
static unsigned long long    g_cma_metadata_access_depth = 0;

static bool initialize_cma_allocator_mutex(pthread_mutex_t **mutex_storage)
{
    pthread_mutex_t *mutex_pointer;
    pthread_mutexattr_t attributes;
    bool attributes_initialized;
    int mutex_error;

    mutex_pointer = static_cast<pthread_mutex_t *>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (false);
    }
    attributes_initialized = false;
    mutex_error = pthread_mutexattr_init(&attributes);
    if (mutex_error == 0)
    {
        attributes_initialized = true;
        mutex_error = pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
        if (mutex_error == 0)
            mutex_error = pthread_mutex_init(mutex_pointer, &attributes);
    }
    if (attributes_initialized)
        pthread_mutexattr_destroy(&attributes);
    if (mutex_error != 0)
    {
        mutex_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
        if (mutex_error != 0)
        {
            std::free(mutex_pointer);
            ft_errno = FT_ERR_INITIALIZATION_FAILED;
            return (false);
        }
    }
    *mutex_storage = mutex_pointer;
    ft_errno = ER_SUCCESS;
    return (true);
}

static pthread_mutex_t *cma_allocator_mutex(void)
{
    static pthread_mutex_t *mutex_instance = ft_nullptr;
    static bool initialization_attempted = false;
    static bool initialized = false;

    if (!initialization_attempted)
    {
        initialization_attempted = true;
        initialized = initialize_cma_allocator_mutex(&mutex_instance);
        if (!initialized)
            mutex_instance = ft_nullptr;
    }

    if (!initialized)
        return (ft_nullptr);
    return (mutex_instance);
}


cma_allocator_guard::cma_allocator_guard()
    : _lock_acquired(false), _active(false), _error_code(ER_SUCCESS),
      _failure_logged(false)
{
    if (this->acquire_allocator_mutex() == false)
    {
        this->_active = false;
        this->set_error(ft_errno);
        return ;
    }
    this->_active = true;
    this->set_error(ER_SUCCESS);
    return ;
}

cma_allocator_guard::~cma_allocator_guard()
{
    this->unlock();
    return ;
}

bool cma_allocator_guard::is_active() const
{
    if (!this->_active && !this->_failure_logged)
    {
        void *return_address;

        return_address = __builtin_return_address(0);
        this->log_inactive_guard(return_address);
    }
    return (this->_active);
}

bool cma_allocator_guard::lock_acquired() const
{
    return (this->_lock_acquired);
}

void cma_allocator_guard::unlock()
{
    int entry_errno;
    int release_error;

    if (!this->_active)
        return ;
    entry_errno = ft_errno;
    release_error = this->release_all_mutexes();
    this->_active = false;
    this->set_error(release_error);
    if (release_error == ER_SUCCESS)
        ft_errno = entry_errno;
    return ;
}

int cma_allocator_guard::get_error() const
{
    return (this->_error_code);
}

const char *cma_allocator_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

void cma_allocator_guard::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void cma_allocator_guard::log_inactive_guard(void *return_address) const
{
    pt_thread_id_type thread_identifier;
    int previous_errno;

    if (this->_failure_logged)
        return ;
    this->_failure_logged = true;
    previous_errno = ft_errno;
    thread_identifier = pt_thread_self();
    ft_log_error("cma_allocator_guard inactive thread=%p return_address=%p",
        reinterpret_cast<void *>(thread_identifier), return_address);
    ft_errno = previous_errno;
    return ;
}

bool cma_allocator_guard::acquire_allocator_mutex()
{
    pthread_mutex_t *mutex_pointer;

    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->_lock_acquired = false;
        this->set_error(ft_errno);
        return (false);
    }
    return (this->acquire_mutex(mutex_pointer));
}

bool cma_allocator_guard::acquire_mutex(pthread_mutex_t *mutex_pointer)
{
    pt_thread_id_type thread_identifier;
    bool local_lock_acquired;
    cma_guard_vector<s_mutex_entry> previous_mutexes;
    pt_mutex_vector current_mutexes;
    pt_mutex_vector tracked_mutexes;
    ft_size_t index;

    thread_identifier = pt_thread_self();
    while (true)
    {
        current_mutexes = pt_lock_tracking::get_owned_mutexes(thread_identifier);
        if (ft_errno != ER_SUCCESS)
        {
            this->_lock_acquired = false;
            this->set_error(ft_errno);
            return (false);
        }
        tracked_mutexes = this->owned_mutex_pointers();
        index = 0;
        while (index < tracked_mutexes.size())
        {
            if (!this->mutex_vector_contains(current_mutexes, tracked_mutexes[index]))
                current_mutexes.push_back(tracked_mutexes[index]);
            index += 1;
        }
        if (!pt_lock_tracking::notify_wait(thread_identifier, mutex_pointer, current_mutexes))
        {
            int release_error;
            int snapshot_error;

            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            previous_mutexes = this->snapshot_owned_mutexes();
            snapshot_error = previous_mutexes.get_error();
            if (snapshot_error != ER_SUCCESS)
            {
                release_error = this->release_all_mutexes();
                if (release_error != ER_SUCCESS)
                    this->set_error(release_error);
                else
                    this->set_error(snapshot_error);
                return (false);
            }
            release_error = this->release_all_mutexes();
            if (release_error != ER_SUCCESS)
            {
                this->set_error(release_error);
                return (false);
            }
            this->sleep_random_backoff();
            if (!this->reacquire_mutexes(previous_mutexes))
            {
                int reacquire_error;

                reacquire_error = ft_errno;
                release_error = this->release_all_mutexes();
                if (release_error != ER_SUCCESS)
                    this->set_error(release_error);
                else
                    this->set_error(reacquire_error);
                return (false);
            }
            continue ;
        }
        local_lock_acquired = false;
        if (cma_lock_allocator(&local_lock_acquired) != 0)
        {
            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            this->_lock_acquired = false;
            this->set_error(ft_errno);
            return (false);
        }
        if (!local_lock_acquired)
        {
            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            this->_lock_acquired = false;
            this->set_error(FT_ERR_INVALID_STATE);
            return (false);
        }
        this->_lock_acquired = true;
        this->track_mutex_acquisition(mutex_pointer, local_lock_acquired);
        pt_lock_tracking::notify_acquired(thread_identifier, mutex_pointer);
        this->set_error(ER_SUCCESS);
        return (true);
    }
}

int cma_allocator_guard::release_all_mutexes()
{
    pt_thread_id_type thread_identifier;
    ft_size_t count;
    int release_error;

    thread_identifier = pt_thread_self();
    release_error = ER_SUCCESS;
    count = this->_owned_mutexes.size();
    while (count > 0)
    {
        pthread_mutex_t *mutex_pointer;
        bool lock_state;

        count -= 1;
        mutex_pointer = this->_owned_mutexes[count].mutex_pointer;
        lock_state = this->_owned_mutexes[count].lock_acquired;
        cma_unlock_allocator(lock_state);
        if (ft_errno != ER_SUCCESS && release_error == ER_SUCCESS)
            release_error = ft_errno;
        pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
        if (ft_errno != ER_SUCCESS && release_error == ER_SUCCESS)
            release_error = ft_errno;
        this->_owned_mutexes.pop_back();
    }
    this->_lock_acquired = false;
    if (release_error != ER_SUCCESS)
        ft_errno = release_error;
    else
        ft_errno = ER_SUCCESS;
    return (release_error);
}

bool cma_allocator_guard::reacquire_mutexes(const cma_guard_vector<s_mutex_entry> &previous_mutexes)
{
    ft_size_t index;

    index = 0;
    while (index < previous_mutexes.size())
    {
        if (!this->acquire_mutex(previous_mutexes[index].mutex_pointer))
            return (false);
        index += 1;
    }
    return (true);
}

void cma_allocator_guard::track_mutex_acquisition(pthread_mutex_t *mutex_pointer, bool lock_acquired)
{
    ft_size_t index;
    s_mutex_entry entry;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        if (this->_owned_mutexes[index].mutex_pointer == mutex_pointer)
        {
            this->_owned_mutexes[index].lock_acquired = lock_acquired;
            return ;
        }
        index += 1;
    }
    entry.mutex_pointer = mutex_pointer;
    entry.lock_acquired = lock_acquired;
    this->_owned_mutexes.push_back(entry);
    if (this->_owned_mutexes.get_error() != ER_SUCCESS)
    {
        this->_lock_acquired = false;
        this->set_error(this->_owned_mutexes.get_error());
    }
    return ;
}

pt_mutex_vector cma_allocator_guard::owned_mutex_pointers() const
{
    pt_mutex_vector mutex_pointers;
    ft_size_t index;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        mutex_pointers.push_back(this->_owned_mutexes[index].mutex_pointer);
        index += 1;
    }
    return (mutex_pointers);
}

cma_guard_vector<cma_allocator_guard::s_mutex_entry> cma_allocator_guard::snapshot_owned_mutexes() const
{
    cma_guard_vector<s_mutex_entry> snapshot;
    ft_size_t index;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        snapshot.push_back(this->_owned_mutexes[index]);
        if (snapshot.get_error() != ER_SUCCESS)
            return (snapshot);
        index += 1;
    }
    return (snapshot);
}

bool cma_allocator_guard::mutex_vector_contains(const pt_mutex_vector &mutexes, pthread_mutex_t *mutex_pointer) const
{
    ft_size_t index;

    index = 0;
    while (index < mutexes.size())
    {
        if (mutexes[index] == mutex_pointer)
            return (true);
        index += 1;
    }
    return (false);
}

void cma_allocator_guard::sleep_random_backoff() const
{
    static thread_local bool generator_initialized = false;
    static thread_local std::minstd_rand generator;
    std::uniform_int_distribution<int> distribution(1, 10);
    std::random_device device;
    int delay_ms;

    if (!generator_initialized)
    {
        generator.seed(device());
        generator_initialized = true;
    }
    delay_ms = distribution(generator);
    pt_thread_sleep(static_cast<unsigned int>(delay_ms));
    return ;
}

static bool cma_metadata_guard_increment(void);
static bool cma_metadata_guard_decrement(void);

static ft_size_t cma_metadata_compute_stride(void)
{
    ft_size_t    stride;

    if (g_cma_metadata_stride != 0)
        return (g_cma_metadata_stride);
    stride = align16(static_cast<ft_size_t>(sizeof(Block)));
    g_cma_metadata_stride = stride;
    return (stride);
}

static ft_size_t cma_metadata_compute_page_size(void)
{
    long    system_page_size;

    if (g_cma_metadata_page_size != 0)
        return (g_cma_metadata_page_size);
    system_page_size = sysconf(_SC_PAGESIZE);
    if (system_page_size <= 0)
        g_cma_metadata_page_size = static_cast<ft_size_t>(4096);
    else
        g_cma_metadata_page_size = static_cast<ft_size_t>(system_page_size);
    return (g_cma_metadata_page_size);
}

static int  cma_metadata_add_chunk(void)
{
    cma_metadata_chunk    *chunk;
    ft_size_t    page_size;
    ft_size_t    chunk_size;
    void        *memory;

    chunk = static_cast<cma_metadata_chunk *>(std::malloc(
            sizeof(cma_metadata_chunk)));
    if (chunk == ft_nullptr)
        return (-1);
    page_size = cma_metadata_compute_page_size();
    chunk_size = page_size * 4;
    memory = mmap(ft_nullptr, chunk_size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
    {
        std::free(chunk);
        return (-1);
    }
    chunk->memory = static_cast<unsigned char *>(memory);
    chunk->size = chunk_size;
    chunk->used = 0;
    chunk->protected_state = false;
    chunk->next = g_cma_metadata_chunks;
    g_cma_metadata_chunks = chunk;
    return (0);
}

#if CMA_ENABLE_METADATA_PROTECTION

static int  cma_metadata_apply_protection(int protection)
{
    cma_metadata_chunk    *chunk;

    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        if (chunk->memory != ft_nullptr && chunk->size != 0)
        {
            bool desired_protected;
            bool needs_update;

            desired_protected = false;
            if (protection == PROT_NONE)
                desired_protected = true;
            needs_update = true;
            if (chunk->protected_state == desired_protected)
                needs_update = false;
            if (needs_update)
            {
                if (mprotect(chunk->memory, chunk->size, protection) != 0)
                {
                    ft_errno = FT_ERR_INTERNAL;
                    return (-1);
                }
                if (desired_protected)
                    chunk->protected_state = true;
                else
                    chunk->protected_state = false;
            }
        }
        chunk = chunk->next;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif

#if CMA_ENABLE_METADATA_PROTECTION

int cma_metadata_make_writable(void)
{
    if (g_cma_metadata_access_depth != 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (cma_metadata_add_chunk() != 0)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (-1);
        }
    }
    if (cma_metadata_apply_protection(PROT_READ | PROT_WRITE) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_metadata_make_inaccessible(void)
{
    if (g_cma_metadata_chunks == ft_nullptr)
        return ;
    cma_metadata_apply_protection(PROT_NONE);
    return ;
}





static bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (true);
}

static bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    g_cma_metadata_access_depth--;
    if (g_cma_metadata_access_depth == 0)
        cma_metadata_make_inaccessible();
    return (true);
}

#else

int cma_metadata_make_writable(void)
{
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_metadata_make_inaccessible(void)
{
    return ;
}

static bool cma_metadata_guard_increment(void)
{
    g_cma_metadata_access_depth++;
    return (true);
}

static bool cma_metadata_guard_decrement(void)
{
    if (g_cma_metadata_access_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (false);
    }
    g_cma_metadata_access_depth--;
    return (true);
}

#endif

Block    *cma_metadata_allocate_block(void)
{
    cma_metadata_chunk    *chunk;
    Block                *block;
    ft_size_t            stride;

    if (g_cma_metadata_free_list != ft_nullptr)
    {
        block = g_cma_metadata_free_list;
        g_cma_metadata_free_list = block->next;
        std::memset(block, 0, sizeof(Block));
        return (block);
    }
    if (g_cma_metadata_chunks == ft_nullptr)
    {
        if (cma_metadata_add_chunk() != 0)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    stride = cma_metadata_compute_stride();
    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        if (chunk->used + stride <= chunk->size)
        {
            block = reinterpret_cast<Block *>(chunk->memory + chunk->used);
            chunk->used += stride;
            std::memset(block, 0, sizeof(Block));
            ft_errno = ER_SUCCESS;
            return (block);
        }
        if (chunk->next == ft_nullptr)
        {
            if (cma_metadata_add_chunk() != 0)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                return (ft_nullptr);
            }
            chunk = g_cma_metadata_chunks;
        }
        else
            chunk = chunk->next;
    }
    ft_errno = FT_ERR_NO_MEMORY;
    return (ft_nullptr);
}

void    cma_metadata_release_block(Block *block)
{
    if (block == ft_nullptr)
        return ;
    block->next = g_cma_metadata_free_list;
    block->prev = ft_nullptr;
    block->size = 0;
    block->payload = ft_nullptr;
    g_cma_metadata_free_list = block;
    return ;
}

void    cma_metadata_reset(void)
{
    cma_metadata_chunk    *chunk;

    chunk = g_cma_metadata_chunks;
    while (chunk)
    {
        cma_metadata_chunk    *next_chunk;

        next_chunk = chunk->next;
        if (chunk->memory != ft_nullptr && chunk->size != 0)
            munmap(chunk->memory, chunk->size);
        std::free(chunk);
        chunk = next_chunk;
    }
    g_cma_metadata_chunks = ft_nullptr;
    g_cma_metadata_free_list = ft_nullptr;
    g_cma_metadata_stride = 0;
    g_cma_metadata_page_size = 0;
    return ;
}







int cma_lock_allocator(bool *lock_acquired)
{
    bool guard_incremented;
    int mutex_error;
    pthread_mutex_t *mutex_pointer;

    if (!lock_acquired)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *lock_acquired = false;
    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return (-1);
    }
    mutex_error = pthread_mutex_lock(mutex_pointer);
    if (mutex_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (cma_metadata_make_writable() != 0)
    {
        pthread_mutex_unlock(mutex_pointer);
        return (-1);
    }
    guard_incremented = cma_metadata_guard_increment();
    if (!guard_incremented)
    {
        pthread_mutex_unlock(mutex_pointer);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_unlock_allocator(bool lock_acquired)
{
    bool guard_decremented;
    int mutex_error;
    pthread_mutex_t *mutex_pointer;

    if (!lock_acquired)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    guard_decremented = cma_metadata_guard_decrement();
    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return ;
    }
    mutex_error = pthread_mutex_unlock(mutex_pointer);
    if (!guard_decremented)
        return ;
    if (mutex_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}





static ft_size_t determine_page_size(ft_size_t size)
{
    if (size < SMALL_SIZE)
        return (SMALL_ALLOC);
    else if (size < MEDIUM_SIZE)
        return (MEDIUM_ALLOC);
    return (size);
}

static void determine_page_use(Page *page)
{
    if (page->heap == false)
        page->alloc_size_type = 0;
    else if (page->size == SMALL_ALLOC)
        page->alloc_size_type = 0;
    else if (page->size == MEDIUM_ALLOC)
        page->alloc_size_type = 1;
    else
        page->alloc_size_type = 2;
    return ;
}

static int8_t determine_which_block_to_use(ft_size_t size)
{
    if (size < SMALL_SIZE)
        return (0);
    else if (size < MEDIUM_SIZE)
        return (1);
    return (2);
}

static void *create_stack_block(void)
{
    static char memory_block[PAGE_SIZE];

    return (memory_block);
}




static void report_corrupted_block(Block *block, const char *context,
        void *user_pointer)
{
    (void)block;
    (void)user_pointer;
    (void)context;
    ft_errno = FT_ERR_INVALID_STATE;
    su_sigabrt();
    return ;
}

static int are_blocks_adjacent(Block *left_block, Block *right_block)
{
    unsigned char   *expected_address;
    unsigned char   *actual_address;

    if (left_block == ft_nullptr || right_block == ft_nullptr)
        return (0);
    if (left_block->payload == ft_nullptr || right_block->payload == ft_nullptr)
        return (0);
    expected_address = left_block->payload + left_block->size;
    actual_address = right_block->payload;
    if (expected_address == actual_address)
        return (1);
    return (0);
}

static void verify_forward_link(Block *block, Block *next_block)
{
    if (next_block->prev != block)
        report_corrupted_block(next_block, "merge_block inconsistent next link",
            ft_nullptr);
    if (are_blocks_adjacent(block, next_block) == 0)
        report_corrupted_block(next_block, "merge_block detached next neighbor",
            ft_nullptr);
    return ;
}

static void verify_backward_link(Block *block, Block *previous_block)
{
    if (previous_block->next != block)
        report_corrupted_block(previous_block,
            "merge_block inconsistent prev link", ft_nullptr);
    if (are_blocks_adjacent(previous_block, block) == 0)
        report_corrupted_block(previous_block,
            "merge_block detached prev neighbor", ft_nullptr);
    return ;
}

void cma_validate_block(Block *block, const char *context, void *user_pointer)
{
    const char    *location;
    bool            sentinel_free;
    bool            sentinel_allocated;

    location = context;
    if (block == ft_nullptr)
    {
        if (location == ft_nullptr)
            location = "unknown";
        su_sigabrt();
    }
    sentinel_free = (block->magic == MAGIC_NUMBER_FREE);
    sentinel_allocated = (block->magic == MAGIC_NUMBER_ALLOCATED);
    if (!sentinel_free && !sentinel_allocated)
        report_corrupted_block(block, location, user_pointer);
    if (sentinel_free && block->free == false)
        cma_mark_block_free(block);
    if (sentinel_allocated && block->free == true)
        cma_mark_block_allocated(block);
    if (block->payload == ft_nullptr)
        report_corrupted_block(block, location, user_pointer);
    return ;
}

static ft_size_t    minimum_split_payload(void)
{
    ft_size_t    minimum_payload;

    minimum_payload = align16(1);
    if (minimum_payload < static_cast<ft_size_t>(16))
        minimum_payload = static_cast<ft_size_t>(16);
    return (minimum_payload);
}









Block* split_block(Block* block, ft_size_t size)
{
    Block       *new_block;
    ft_size_t    available_size;
    ft_size_t    remaining_size;
    ft_size_t    minimum_payload;

    cma_validate_block(block, "split_block", ft_nullptr);
    available_size = block->size;
    if (size >= available_size)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    remaining_size = available_size - size;
    minimum_payload = minimum_split_payload();
    if (remaining_size <= minimum_payload)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    new_block = cma_metadata_allocate_block();
    if (new_block == ft_nullptr)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    new_block->size = remaining_size;
    new_block->payload = block->payload + size;
    cma_mark_block_free(new_block);
    new_block->next = block->next;
    new_block->prev = block;
    if (new_block->next)
    {
        cma_validate_block(new_block->next, "split_block relink next", ft_nullptr);
        new_block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
    if (cma_block_is_free(block))
        cma_mark_block_free(block);
    else
        cma_mark_block_allocated(block);
    return (block);
}

Page *create_page(ft_size_t size)
{
    ft_size_t page_size = determine_page_size(size);
    bool use_heap = true;

    if (page_list == ft_nullptr)
    {
        page_size = PAGE_SIZE;
        use_heap = false;
    }
    else
    {
        if (size > determine_page_size(size))
            page_size = size;
    }
    void* ptr;
    if (use_heap)
    {
        ptr = std::malloc(static_cast<size_t>(page_size));
        if (!ptr)
            return (ft_nullptr);
    }
    else
    {
        ptr = create_stack_block();
        if (!ptr)
            return (ft_nullptr);
    }
    Page* page = static_cast<Page*>(std::malloc(sizeof(Page)));
    if (!page)
    {
        if (use_heap)
            std::free(ptr);
        return (ft_nullptr);
    }
    page->heap = use_heap;
    page->start = ptr;
    page->size = page_size;
    page->next = ft_nullptr;
    page->prev = ft_nullptr;
    page->blocks = cma_metadata_allocate_block();
    if (page->blocks == ft_nullptr)
    {
        if (use_heap)
            std::free(ptr);
        std::free(page);
        return (ft_nullptr);
    }
    page->blocks->size = page_size;
    page->blocks->payload = static_cast<unsigned char *>(ptr);
    cma_mark_block_free(page->blocks);
    page->blocks->next = ft_nullptr;
    page->blocks->prev = ft_nullptr;
    cma_validate_block(page->blocks, "create_page", ft_nullptr);
    determine_page_use(page);
    if (!page_list) {
        page_list = page;
    }
    else
    {
        page->next = page_list;
        page_list->prev = page;
        page_list = page;
    }
    return (page);
}

Block *find_free_block(ft_size_t size)
{
    Page* cur_page = page_list;
    int8_t alloc_size_type = determine_which_block_to_use(size);
    while (cur_page)
    {
        if (cur_page->alloc_size_type != alloc_size_type)
        {
            cur_page = cur_page->next;
            continue ;
        }
        Block* cur_block = cur_page->blocks;
        while (cur_block)
        {
            cma_validate_block(cur_block, "find_free_block", ft_nullptr);
            if (cma_block_is_free(cur_block) && cur_block->size >= size)
                return (cur_block);
            cur_block = cur_block->next;
        }
        cur_page = cur_page->next;
    }
    return (ft_nullptr);
}

Block    *cma_find_block_for_pointer(const void *memory_pointer)
{
    Page    *current_page;

    if (memory_pointer == ft_nullptr)
        return (ft_nullptr);
    current_page = page_list;
    while (current_page)
    {
        Block    *current_block;

        current_block = current_page->blocks;
        while (current_block)
        {
            if (current_block->payload == memory_pointer)
                return (current_block);
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (ft_nullptr);
}

Block *merge_block(Block *block)
{
    Block       *current;
    Block       *next_block;
    Block       *previous_block;

    cma_validate_block(block, "merge_block", ft_nullptr);
    current = block;
    previous_block = current->prev;
    while (previous_block && cma_block_is_free(previous_block))
    {
        cma_validate_block(previous_block, "merge_block prev", ft_nullptr);
        verify_backward_link(current, previous_block);
#ifdef DEBUG
#endif
        previous_block->size += current->size;
#ifdef DEBUG
#endif
        previous_block->next = current->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink prev", ft_nullptr);
            current->next->prev = previous_block;
        }
        current->next = ft_nullptr;
        current->prev = ft_nullptr;
        cma_mark_block_free(current);
        cma_metadata_release_block(current);
        current = previous_block;
        previous_block = current->prev;
    }
    next_block = current->next;
    while (next_block && cma_block_is_free(next_block))
    {
        cma_validate_block(next_block, "merge_block next", ft_nullptr);
        verify_forward_link(current, next_block);
#ifdef DEBUG
#endif
        current->size += next_block->size;
#ifdef DEBUG
#endif
        current->next = next_block->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink next", ft_nullptr);
            current->next->prev = current;
        }
        next_block->next = ft_nullptr;
        next_block->prev = ft_nullptr;
        cma_mark_block_free(next_block);
        cma_metadata_release_block(next_block);
        next_block = current->next;
    }
    cma_mark_block_free(current);
#ifdef DEBUG
#endif
    return (current);
}

Page *find_page_of_block(Block *block)
{
    Page *page = page_list;
    while (page)
    {
        unsigned char    *start;
        unsigned char    *end;
        unsigned char    *payload;

        start = static_cast<unsigned char *>(page->start);
        end = start + static_cast<size_t>(page->size);
        payload = block->payload;
        if (payload >= start && payload < end)
            return (page);
        page = page->next;
    }
    return (ft_nullptr);
}

void free_page_if_empty(Page *page)
{
    if (!page || page->heap == false)
        return ;
    if (page->blocks && cma_block_is_free(page->blocks) &&
        page->blocks->next == ft_nullptr &&
        page->blocks->prev == ft_nullptr)
    {
        if (page->prev)
            page->prev->next = page->next;
        if (page->next)
            page->next->prev = page->prev;
        if (page_list == page)
            page_list = page->next;
        std::free(page->start);
        cma_metadata_release_block(page->blocks);
        std::free(page);
    }
    return ;
}









void cma_get_extended_stats(ft_size_t *allocation_count,
        ft_size_t *free_count,
        ft_size_t *current_bytes,
        ft_size_t *peak_bytes)
{
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    if (allocation_count != ft_nullptr)
        *allocation_count = g_cma_allocation_count;
    if (free_count != ft_nullptr)
        *free_count = g_cma_free_count;
    if (current_bytes != ft_nullptr)
        *current_bytes = g_cma_current_bytes;
    if (peak_bytes != ft_nullptr)
        *peak_bytes = g_cma_peak_bytes;
    return ;
}

void cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count)
{
    cma_get_extended_stats(allocation_count, free_count, ft_nullptr, ft_nullptr);
    return ;
}
