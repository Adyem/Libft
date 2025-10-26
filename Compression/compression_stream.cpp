#include <zlib.h>
#include <limits>
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/unique_lock.hpp"
#include "compression.hpp"
#include "compression_stream_test_hooks.hpp"

static const std::size_t   g_compress_stream_default_buffer_size = 4096;
static const std::size_t   g_compress_stream_speed_buffer_size = 16384;
static const std::size_t   g_compress_stream_ratio_buffer_size = 32768;
static const int           g_compress_stream_min_window_bits = 8;
static const int           g_compress_stream_min_memory_level = 1;
static const int           g_compress_stream_max_memory_level = MAX_MEM_LEVEL;
static const int           g_compress_stream_default_memory_level = MAX_MEM_LEVEL - 1;

struct s_compress_stream_options_snapshot
{
    std::size_t                            input_buffer_size;
    std::size_t                            output_buffer_size;
    t_compress_stream_progress_callback    progress_callback;
    t_compress_stream_cancel_callback      cancel_callback;
    void                                   *callback_user_data;
    int                                     compression_level;
    int                                     window_bits;
    int                                     memory_level;
    int                                     strategy;
};

static void compression_stream_init_snapshot(struct s_compress_stream_options_snapshot *snapshot)
{
    if (snapshot == ft_nullptr)
        return ;
    snapshot->input_buffer_size = 0;
    snapshot->output_buffer_size = 0;
    snapshot->progress_callback = ft_nullptr;
    snapshot->cancel_callback = ft_nullptr;
    snapshot->callback_user_data = ft_nullptr;
    snapshot->compression_level = Z_DEFAULT_COMPRESSION;
    snapshot->window_bits = 0;
    snapshot->memory_level = 0;
    snapshot->strategy = Z_DEFAULT_STRATEGY;
    return ;
}

t_compress_stream_options::t_compress_stream_options(void)
{
    this->_input_buffer_size = 0;
    this->_output_buffer_size = 0;
    this->_progress_callback = ft_nullptr;
    this->_cancel_callback = ft_nullptr;
    this->_callback_user_data = ft_nullptr;
    this->_compression_level = Z_BEST_COMPRESSION;
    this->_window_bits = 0;
    this->_memory_level = 0;
    this->_strategy = Z_DEFAULT_STRATEGY;
    this->_error_code = ER_SUCCESS;
    return ;
}

t_compress_stream_options::~t_compress_stream_options(void)
{
    return ;
}

int t_compress_stream_options::reset(void)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_input_buffer_size = 0;
    this->_output_buffer_size = 0;
    this->_progress_callback = ft_nullptr;
    this->_cancel_callback = ft_nullptr;
    this->_callback_user_data = ft_nullptr;
    this->_compression_level = Z_BEST_COMPRESSION;
    this->_window_bits = 0;
    this->_memory_level = 0;
    this->_strategy = Z_DEFAULT_STRATEGY;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_input_buffer_size(std::size_t input_buffer_size)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_input_buffer_size = input_buffer_size;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_output_buffer_size(std::size_t output_buffer_size)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_output_buffer_size = output_buffer_size;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_progress_callback(t_compress_stream_progress_callback callback, void *user_data)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_progress_callback = callback;
    this->_callback_user_data = user_data;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_cancel_callback(t_compress_stream_cancel_callback callback, void *user_data)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_cancel_callback = callback;
    this->_callback_user_data = user_data;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_callbacks(t_compress_stream_progress_callback progress_callback,
        t_compress_stream_cancel_callback cancel_callback,
        void *user_data)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_progress_callback = progress_callback;
    this->_cancel_callback = cancel_callback;
    this->_callback_user_data = user_data;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_compression_level(int compression_level)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_compression_level = compression_level;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_window_bits(int window_bits)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_window_bits = window_bits;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_memory_level(int memory_level)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_memory_level = memory_level;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::set_strategy(int strategy)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    this->_strategy = strategy;
    this->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

std::size_t t_compress_stream_options::get_input_buffer_size() const
{
    int entry_errno;
    std::size_t input_buffer_size;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (0);
    }
    input_buffer_size = this->_input_buffer_size;
    ft_errno = entry_errno;
    return (input_buffer_size);
}

std::size_t t_compress_stream_options::get_output_buffer_size() const
{
    int entry_errno;
    std::size_t output_buffer_size;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (0);
    }
    output_buffer_size = this->_output_buffer_size;
    ft_errno = entry_errno;
    return (output_buffer_size);
}

t_compress_stream_progress_callback t_compress_stream_options::get_progress_callback() const
{
    int entry_errno;
    t_compress_stream_progress_callback progress_callback;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (ft_nullptr);
    }
    progress_callback = this->_progress_callback;
    ft_errno = entry_errno;
    return (progress_callback);
}

t_compress_stream_cancel_callback t_compress_stream_options::get_cancel_callback() const
{
    int entry_errno;
    t_compress_stream_cancel_callback cancel_callback;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (ft_nullptr);
    }
    cancel_callback = this->_cancel_callback;
    ft_errno = entry_errno;
    return (cancel_callback);
}

void    *t_compress_stream_options::get_callback_user_data() const
{
    int entry_errno;
    void *user_data;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (ft_nullptr);
    }
    user_data = this->_callback_user_data;
    ft_errno = entry_errno;
    return (user_data);
}

int t_compress_stream_options::get_compression_level() const
{
    int entry_errno;
    int compression_level;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (Z_BEST_COMPRESSION);
    }
    compression_level = this->_compression_level;
    ft_errno = entry_errno;
    return (compression_level);
}

int t_compress_stream_options::get_window_bits() const
{
    int entry_errno;
    int window_bits;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (0);
    }
    window_bits = this->_window_bits;
    ft_errno = entry_errno;
    return (window_bits);
}

int t_compress_stream_options::get_memory_level() const
{
    int entry_errno;
    int memory_level;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (0);
    }
    memory_level = this->_memory_level;
    ft_errno = entry_errno;
    return (memory_level);
}

int t_compress_stream_options::get_strategy() const
{
    int entry_errno;
    int strategy;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (Z_DEFAULT_STRATEGY);
    }
    strategy = this->_strategy;
    ft_errno = entry_errno;
    return (strategy);
}

int t_compress_stream_options::snapshot(struct s_compress_stream_options_snapshot *snapshot) const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    if (snapshot == ft_nullptr)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    snapshot->input_buffer_size = this->_input_buffer_size;
    snapshot->output_buffer_size = this->_output_buffer_size;
    snapshot->progress_callback = this->_progress_callback;
    snapshot->cancel_callback = this->_cancel_callback;
    snapshot->callback_user_data = this->_callback_user_data;
    snapshot->compression_level = this->_compression_level;
    snapshot->window_bits = this->_window_bits;
    snapshot->memory_level = this->_memory_level;
    snapshot->strategy = this->_strategy;
    const_cast<t_compress_stream_options *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

int t_compress_stream_options::get_error() const
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (mutex_guard.get_error());
    }
    error_code = this->_error_code;
    ft_errno = entry_errno;
    return (error_code);
}

const char  *t_compress_stream_options::get_error_str() const
{
    int entry_errno;
    int error_code;
    const char *error_string;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);

    entry_errno = ft_errno;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        const_cast<t_compress_stream_options *>(this)->set_error(mutex_guard.get_error());
        return (ft_strerror(mutex_guard.get_error()));
    }
    error_code = this->_error_code;
    error_string = ft_strerror(error_code);
    ft_errno = entry_errno;
    return (error_string);
}

void    t_compress_stream_options::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void    ft_compress_stream_apply_speed_preset(t_compress_stream_options *options)
{
    int memory_level;

    if (options == ft_nullptr)
        return ;
    if (options->set_input_buffer_size(g_compress_stream_speed_buffer_size) != ER_SUCCESS)
        return ;
    if (options->set_output_buffer_size(g_compress_stream_speed_buffer_size) != ER_SUCCESS)
        return ;
    if (options->set_compression_level(Z_BEST_SPEED) != ER_SUCCESS)
        return ;
    if (options->set_window_bits(MAX_WBITS - 2) != ER_SUCCESS)
        return ;
    memory_level = g_compress_stream_default_memory_level - 1;
    if (memory_level < g_compress_stream_min_memory_level)
        memory_level = g_compress_stream_min_memory_level;
    if (options->set_memory_level(memory_level) != ER_SUCCESS)
        return ;
    if (options->set_strategy(Z_DEFAULT_STRATEGY) != ER_SUCCESS)
        return ;
    return ;
}

void    ft_compress_stream_apply_ratio_preset(t_compress_stream_options *options)
{
    int memory_level;

    if (options == ft_nullptr)
        return ;
    if (options->set_input_buffer_size(g_compress_stream_ratio_buffer_size) != ER_SUCCESS)
        return ;
    if (options->set_output_buffer_size(g_compress_stream_ratio_buffer_size) != ER_SUCCESS)
        return ;
    if (options->set_compression_level(Z_BEST_COMPRESSION) != ER_SUCCESS)
        return ;
    if (options->set_window_bits(MAX_WBITS) != ER_SUCCESS)
        return ;
    memory_level = g_compress_stream_default_memory_level + 1;
    if (memory_level > g_compress_stream_max_memory_level)
        memory_level = g_compress_stream_max_memory_level;
    if (options->set_memory_level(memory_level) != ER_SUCCESS)
        return ;
    if (options->set_strategy(Z_DEFAULT_STRATEGY) != ER_SUCCESS)
        return ;
    return ;
}

static int  compression_stream_validate_tuning(const t_compress_stream_options *options)
{
    s_compress_stream_options_snapshot snapshot;

    compression_stream_init_snapshot(&snapshot);
    if (options == ft_nullptr)
        return (0);
    if (options->snapshot(&snapshot) != ER_SUCCESS)
        return (1);
    if (snapshot.compression_level < Z_DEFAULT_COMPRESSION || snapshot.compression_level > Z_BEST_COMPRESSION)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (snapshot.window_bits != 0
        && (snapshot.window_bits < g_compress_stream_min_window_bits || snapshot.window_bits > MAX_WBITS))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (snapshot.memory_level != 0
        && (snapshot.memory_level < g_compress_stream_min_memory_level
            || snapshot.memory_level > g_compress_stream_max_memory_level))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (snapshot.strategy != Z_DEFAULT_STRATEGY
        && snapshot.strategy != Z_FILTERED
        && snapshot.strategy != Z_HUFFMAN_ONLY
        && snapshot.strategy != Z_RLE
        && snapshot.strategy != Z_FIXED)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    return (0);
}

static t_compress_stream_deflate_init_hook compression_stream_get_deflate_init_hook(void);

static int  compression_stream_begin_deflate(z_stream *stream, const t_compress_stream_options *options)
{
    int compression_level;
    int window_bits;
    int memory_level;
    int strategy;

    compression_level = Z_BEST_COMPRESSION;
    if (options != ft_nullptr)
    {
        s_compress_stream_options_snapshot snapshot;

        compression_stream_init_snapshot(&snapshot);
        if (options->snapshot(&snapshot) != ER_SUCCESS)
            return (FT_ERR_INVALID_ARGUMENT);
        compression_level = snapshot.compression_level;
        window_bits = snapshot.window_bits;
        memory_level = snapshot.memory_level;
        strategy = snapshot.strategy;
        if (window_bits != 0 || memory_level != 0 || strategy != Z_DEFAULT_STRATEGY)
        {
            if (window_bits == 0)
                window_bits = MAX_WBITS;
            if (memory_level == 0)
                memory_level = g_compress_stream_default_memory_level;
            return (deflateInit2(stream, compression_level, Z_DEFLATED, window_bits, memory_level, strategy));
        }
    }
    return (compression_stream_get_deflate_init_hook()(stream, compression_level));
}

static int  map_zlib_error(int zlib_status)
{
    if (zlib_status == Z_MEM_ERROR)
        return (FT_ERR_NO_MEMORY);
    if (zlib_status == Z_BUF_ERROR)
        return (FT_ERR_FULL);
    if (zlib_status == Z_NEED_DICT)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_DATA_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_STREAM_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    if (zlib_status == Z_VERSION_ERROR)
        return (FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_INVALID_ARGUMENT);
}

static void compression_stream_release_buffers(unsigned char *input_buffer, unsigned char *output_buffer)
{
    int saved_errno;

    saved_errno = ft_errno;
    if (input_buffer)
        cma_free(input_buffer);
    if (output_buffer)
        cma_free(output_buffer);
    ft_errno = saved_errno;
    return ;
}

static int  compression_stream_allocate_buffers(const t_compress_stream_options *options,
        unsigned char **input_buffer,
        std::size_t *input_buffer_size,
        unsigned char **output_buffer,
        std::size_t *output_buffer_size)
{
    std::size_t    resolved_input_size;
    std::size_t    resolved_output_size;

    if (input_buffer == ft_nullptr || input_buffer_size == ft_nullptr
        || output_buffer == ft_nullptr || output_buffer_size == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (options)
    {
        s_compress_stream_options_snapshot snapshot;

        compression_stream_init_snapshot(&snapshot);
        if (options->snapshot(&snapshot) != ER_SUCCESS)
            return (1);
        resolved_input_size = snapshot.input_buffer_size;
        resolved_output_size = snapshot.output_buffer_size;
        if (resolved_input_size == 0 || resolved_output_size == 0)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    else
    {
        resolved_input_size = g_compress_stream_default_buffer_size;
        resolved_output_size = g_compress_stream_default_buffer_size;
    }
    if (resolved_input_size > std::numeric_limits<unsigned int>::max()
        || resolved_output_size > std::numeric_limits<unsigned int>::max())
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    *input_buffer = static_cast<unsigned char *>(cma_malloc(resolved_input_size));
    if (!*input_buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (1);
    }
    *output_buffer = static_cast<unsigned char *>(cma_malloc(resolved_output_size));
    if (!*output_buffer)
    {
        cma_free(*input_buffer);
        *input_buffer = ft_nullptr;
        ft_errno = FT_ERR_NO_MEMORY;
        return (1);
    }
    *input_buffer_size = resolved_input_size;
    *output_buffer_size = resolved_output_size;
    return (0);
}

static int compress_stream_default_deflate_init(z_stream *stream, int compression_level)
{
    return (deflateInit(stream, compression_level));
}

static int compress_stream_default_deflate(z_stream *stream, int flush_mode)
{
    return (deflate(stream, flush_mode));
}

static int decompress_stream_default_inflate_init(z_stream *stream)
{
    return (inflateInit(stream));
}

static int decompress_stream_default_inflate(z_stream *stream, int flush_mode)
{
    return (inflate(stream, flush_mode));
}

static t_compress_stream_deflate_init_hook    g_compress_stream_deflate_init_hook = compress_stream_default_deflate_init;
static t_compress_stream_deflate_hook         g_compress_stream_deflate_hook = compress_stream_default_deflate;
static t_decompress_stream_inflate_init_hook  g_decompress_stream_inflate_init_hook = decompress_stream_default_inflate_init;
static t_decompress_stream_inflate_hook       g_decompress_stream_inflate_hook = decompress_stream_default_inflate;

static t_compress_stream_deflate_init_hook compression_stream_get_deflate_init_hook(void)
{
    return (g_compress_stream_deflate_init_hook);
}

static int  compression_stream_dispatch_progress(const t_compress_stream_options *options, const t_compress_stream_progress *progress)
{
    int callback_result;
    s_compress_stream_options_snapshot snapshot;

    compression_stream_init_snapshot(&snapshot);
    if (options == ft_nullptr || progress == ft_nullptr)
        return (0);
    if (options->snapshot(&snapshot) != ER_SUCCESS)
        return (1);
    if (snapshot.progress_callback == ft_nullptr)
        return (0);
    callback_result = snapshot.progress_callback(progress, snapshot.callback_user_data);
    if (callback_result != 0)
    {
        ft_errno = FT_ERR_TERMINATED;
        return (1);
    }
    return (0);
}

static int  compression_stream_check_cancel(const t_compress_stream_options *options, const t_compress_stream_progress *progress)
{
    int callback_result;
    s_compress_stream_options_snapshot snapshot;

    compression_stream_init_snapshot(&snapshot);
    if (options == ft_nullptr || progress == ft_nullptr)
        return (0);
    if (options->snapshot(&snapshot) != ER_SUCCESS)
        return (1);
    if (snapshot.cancel_callback == ft_nullptr)
        return (0);
    callback_result = snapshot.cancel_callback(progress, snapshot.callback_user_data);
    if (callback_result != 0)
    {
        ft_errno = FT_ERR_TERMINATED;
        return (1);
    }
    return (0);
}

void ft_compress_stream_set_deflate_init_hook(t_compress_stream_deflate_init_hook hook)
{
    if (hook)
        g_compress_stream_deflate_init_hook = hook;
    else
        g_compress_stream_deflate_init_hook = compress_stream_default_deflate_init;
    return ;
}

void ft_compress_stream_set_deflate_hook(t_compress_stream_deflate_hook hook)
{
    if (hook)
        g_compress_stream_deflate_hook = hook;
    else
        g_compress_stream_deflate_hook = compress_stream_default_deflate;
    return ;
}

void ft_decompress_stream_set_inflate_init_hook(t_decompress_stream_inflate_init_hook hook)
{
    if (hook)
        g_decompress_stream_inflate_init_hook = hook;
    else
        g_decompress_stream_inflate_init_hook = decompress_stream_default_inflate_init;
    return ;
}

void ft_decompress_stream_set_inflate_hook(t_decompress_stream_inflate_hook hook)
{
    if (hook)
        g_decompress_stream_inflate_hook = hook;
    else
        g_decompress_stream_inflate_hook = decompress_stream_default_inflate;
    return ;
}

int ft_compress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options)
{
    z_stream        stream;
    unsigned char   *input_buffer;
    unsigned char   *output_buffer;
    std::size_t     input_buffer_size;
    std::size_t     output_buffer_size;
    int             flush_mode;
    int             deflate_status;
    ssize_t         read_bytes;
    t_compress_stream_progress    progress;

    if (input_fd < 0 || output_fd < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (compression_stream_validate_tuning(options) != 0)
        return (1);
    input_buffer = ft_nullptr;
    output_buffer = ft_nullptr;
    input_buffer_size = 0;
    output_buffer_size = 0;
    if (compression_stream_allocate_buffers(options, &input_buffer, &input_buffer_size,
            &output_buffer, &output_buffer_size) != 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    deflate_status = compression_stream_begin_deflate(&stream, options);
    if (deflate_status != Z_OK)
    {
        int error_code;

        error_code = map_zlib_error(deflate_status);
        compression_stream_release_buffers(input_buffer, output_buffer);
        ft_errno = error_code;
        return (1);
    }
    flush_mode = Z_NO_FLUSH;
    ft_bzero(&progress, sizeof(progress));
    while (flush_mode != Z_FINISH)
    {
        if (compression_stream_check_cancel(options, &progress) != 0)
        {
            deflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_TERMINATED;
            return (1);
        }
        read_bytes = su_read(input_fd, input_buffer, input_buffer_size);
        if (read_bytes < 0)
        {
            deflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_IO;
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (read_bytes > 0)
        {
            progress.total_bytes_read += static_cast<std::size_t>(read_bytes);
            if (compression_stream_dispatch_progress(options, &progress) != 0)
            {
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_TERMINATED;
                return (1);
            }
        }
        if (compression_stream_check_cancel(options, &progress) != 0)
        {
            deflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_TERMINATED;
            return (1);
        }
        if (static_cast<std::size_t>(read_bytes) < input_buffer_size)
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = static_cast<unsigned int>(output_buffer_size);
            deflate_status = g_compress_stream_deflate_hook(&stream, flush_mode);
            if (deflate_status == Z_STREAM_ERROR || deflate_status == Z_BUF_ERROR)
            {
                int error_code;

                error_code = map_zlib_error(deflate_status);
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = error_code;
                return (1);
            }
            std::size_t produced_bytes;

            produced_bytes = output_buffer_size - static_cast<std::size_t>(stream.avail_out);
            if (su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_IO;
                return (1);
            }
            if (produced_bytes != 0)
            {
                progress.total_bytes_written += produced_bytes;
                if (compression_stream_dispatch_progress(options, &progress) != 0)
                {
                    deflateEnd(&stream);
                    compression_stream_release_buffers(input_buffer, output_buffer);
                    ft_errno = FT_ERR_TERMINATED;
                    return (1);
                }
            }
            if (compression_stream_check_cancel(options, &progress) != 0)
            {
                deflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_TERMINATED;
                return (1);
            }
        }
        while (stream.avail_out == 0);
    }
    deflateEnd(&stream);
    compression_stream_release_buffers(input_buffer, output_buffer);
    ft_errno = ER_SUCCESS;
    return (0);
}

int ft_decompress_stream_with_options(int input_fd, int output_fd, const t_compress_stream_options *options)
{
    z_stream        stream;
    unsigned char   *input_buffer;
    unsigned char   *output_buffer;
    std::size_t     input_buffer_size;
    std::size_t     output_buffer_size;
    int             inflate_status;
    ssize_t         read_bytes;
    int             flush_mode;
    int             stream_finished;
    int             read_any_input;
    t_compress_stream_progress    progress;

    if (input_fd < 0 || output_fd < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (1);
    }
    if (compression_stream_validate_tuning(options) != 0)
        return (1);
    input_buffer = ft_nullptr;
    output_buffer = ft_nullptr;
    input_buffer_size = 0;
    output_buffer_size = 0;
    if (compression_stream_allocate_buffers(options, &input_buffer, &input_buffer_size,
            &output_buffer, &output_buffer_size) != 0)
        return (1);
    ft_bzero(&stream, sizeof(stream));
    inflate_status = g_decompress_stream_inflate_init_hook(&stream);
    if (inflate_status != Z_OK)
    {
        compression_stream_release_buffers(input_buffer, output_buffer);
        ft_errno = map_zlib_error(inflate_status);
        return (1);
    }
    stream_finished = 0;
    read_any_input = 0;
    flush_mode = Z_NO_FLUSH;
    ft_bzero(&progress, sizeof(progress));
    while (1)
    {
        if (compression_stream_check_cancel(options, &progress) != 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_TERMINATED;
            return (1);
        }
        read_bytes = su_read(input_fd, input_buffer, input_buffer_size);
        if (read_bytes < 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_IO;
            return (1);
        }
        stream.next_in = input_buffer;
        stream.avail_in = static_cast<unsigned int>(read_bytes);
        if (read_bytes > 0)
            read_any_input = 1;
        if (read_bytes > 0)
        {
            progress.total_bytes_read += static_cast<std::size_t>(read_bytes);
            if (compression_stream_dispatch_progress(options, &progress) != 0)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_TERMINATED;
                return (1);
            }
        }
        if (compression_stream_check_cancel(options, &progress) != 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_TERMINATED;
            return (1);
        }
        if (read_bytes == 0)
            flush_mode = Z_FINISH;
        else
            flush_mode = Z_NO_FLUSH;
        do
        {
            stream.next_out = output_buffer;
            stream.avail_out = static_cast<unsigned int>(output_buffer_size);
            inflate_status = g_decompress_stream_inflate_hook(&stream, flush_mode);
            if (inflate_status == Z_STREAM_END)
                stream_finished = 1;
            if (inflate_status == Z_BUF_ERROR && stream_finished == 0)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                if (read_any_input == 0)
                    ft_errno = FT_ERR_FULL;
                else
                    ft_errno = FT_ERR_INVALID_ARGUMENT;
                return (1);
            }
            if (inflate_status == Z_NEED_DICT
                || inflate_status == Z_DATA_ERROR
                || inflate_status == Z_MEM_ERROR
                || inflate_status == Z_STREAM_ERROR
                || inflate_status == Z_VERSION_ERROR)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = map_zlib_error(inflate_status);
                return (1);
            }
            std::size_t produced_bytes;

            produced_bytes = output_buffer_size - static_cast<std::size_t>(stream.avail_out);
            if (produced_bytes != 0
                && su_write(output_fd, output_buffer, produced_bytes) != static_cast<ssize_t>(produced_bytes))
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_IO;
                return (1);
            }
            if (produced_bytes != 0)
            {
                progress.total_bytes_written += produced_bytes;
                if (compression_stream_dispatch_progress(options, &progress) != 0)
                {
                    inflateEnd(&stream);
                    compression_stream_release_buffers(input_buffer, output_buffer);
                    ft_errno = FT_ERR_TERMINATED;
                    return (1);
                }
            }
            if (compression_stream_check_cancel(options, &progress) != 0)
            {
                inflateEnd(&stream);
                compression_stream_release_buffers(input_buffer, output_buffer);
                ft_errno = FT_ERR_TERMINATED;
                return (1);
            }
        }
        while (stream.avail_out == 0 && stream_finished == 0 && stream.avail_in != 0);
        if (stream_finished != 0 && stream.avail_in != 0)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
        if (stream_finished != 0)
        {
            if (flush_mode == Z_FINISH)
                break ;
            continue ;
        }
        if (flush_mode == Z_FINISH)
        {
            inflateEnd(&stream);
            compression_stream_release_buffers(input_buffer, output_buffer);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (1);
        }
    }
    inflateEnd(&stream);
    compression_stream_release_buffers(input_buffer, output_buffer);
    ft_errno = ER_SUCCESS;
    return (0);
}

int ft_compress_stream(int input_fd, int output_fd)
{
    return (ft_compress_stream_with_options(input_fd, output_fd, ft_nullptr));
}

int ft_decompress_stream(int input_fd, int output_fd)
{
    return (ft_decompress_stream_with_options(input_fd, output_fd, ft_nullptr));
}
