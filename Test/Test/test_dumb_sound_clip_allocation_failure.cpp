#include "../test_internal.hpp"
#include "../../DUMB/dumb_sound_clip.hpp"
#include "../../DUMB/dumb_sound.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int dumb_prepare_sparse_wav_file(char *path_buffer, size_t path_buffer_size)
{
    int file_descriptor;
    int truncate_status;

    if (path_buffer == ft_nullptr || path_buffer_size < 32)
        return (0);
    std::snprintf(path_buffer, path_buffer_size, "/tmp/libft_dumb_wav_XXXXXX");
    file_descriptor = mkstemp(path_buffer);
    if (file_descriptor < 0)
        return (0);
    truncate_status = ftruncate(file_descriptor, 16 * 1024 * 1024);
    close(file_descriptor);
    if (truncate_status != 0)
    {
        unlink(path_buffer);
        return (0);
    }
    return (1);
}

static int dumb_expect_load_wav_allocation_failure_child(void)
{
    struct rlimit limit_data;
    ft_sound_clip sound_clip_instance;
    char temporary_path[64];
    int result_code;

    limit_data.rlim_cur = 8 * 1024 * 1024;
    limit_data.rlim_max = 8 * 1024 * 1024;
    if (setrlimit(RLIMIT_AS, &limit_data) != 0)
        return (0);
    if (dumb_prepare_sparse_wav_file(temporary_path, sizeof(temporary_path)) == 0)
        return (0);
    if (sound_clip_instance.initialize() != FT_ERR_SUCCESS)
    {
        unlink(temporary_path);
        return (0);
    }
    result_code = sound_clip_instance.load_wav(temporary_path);
    (void)sound_clip_instance.destroy();
    unlink(temporary_path);
    if (result_code != ft_sound_error_platform_failure)
        return (0);
    return (1);
}

FT_TEST(test_dumb_sound_clip_load_wav_reports_failure_when_allocation_is_constrained,
    "dumb sound clip load_wav reports platform failure when file read allocation fails")
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        if (dumb_expect_load_wav_allocation_failure_child() == 1)
            _exit(0);
        _exit(1);
    }
    FT_ASSERT(child_process_id > 0);
    child_status = 0;
    FT_ASSERT(waitpid(child_process_id, &child_status, 0) > 0);
    FT_ASSERT(WIFEXITED(child_status));
    FT_ASSERT_EQ(0, WEXITSTATUS(child_status));
    return (1);
}
