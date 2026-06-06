#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include <cstdlib>
#include <cstdio>
#include <string>

#ifndef LIBFT_TEST_BUILD
#endif

struct s_runtime_file_guard
{
    char source_path[4096];
    char executable_path[8192];
    ft_bool source_path_active;
    ft_bool executable_path_active;

    s_runtime_file_guard(void)
    {
        this->source_path[0] = '\0';
        this->executable_path[0] = '\0';
        this->source_path_active = FT_FALSE;
        this->executable_path_active = FT_FALSE;
        return ;
    }

    ~s_runtime_file_guard(void)
    {
        this->destroy();
        return ;
    }

    void destroy(void)
    {
        if (this->source_path_active == FT_TRUE)
            (void)unlink(this->source_path);
        if (this->executable_path_active == FT_TRUE)
            (void)unlink(this->executable_path);
        this->source_path_active = FT_FALSE;
        this->executable_path_active = FT_FALSE;
        return ;
    }
};

static std::string runtime_project_root(void)
{
    char current_directory[4096];
    std::string candidate_directory;
    std::size_t slash_position;

    if (getcwd(current_directory, sizeof(current_directory)) == ft_nullptr)
        return (std::string());
    candidate_directory = current_directory;
    while (candidate_directory.empty() == FT_FALSE)
    {
        if (access((candidate_directory + "/Full_Libft_test.a").c_str(), F_OK) == 0
            && access((candidate_directory + "/Modules").c_str(), F_OK) == 0)
            return (candidate_directory);
        if (candidate_directory == "/")
            break ;
        slash_position = candidate_directory.find_last_of('/');
        if (slash_position == std::string::npos)
            break ;
        if (slash_position == 0)
            candidate_directory = "/";
        else
            candidate_directory.erase(slash_position);
    }
    return (std::string());
}

static int32_t runtime_write_source_file(const char *source_path,
    const char *source_code)
{
    FILE *source_file;
    ft_size_t source_length;
    std::size_t write_count;

    source_file = std::fopen(source_path, "w");
    if (source_file == ft_nullptr)
        return (0);
    source_length = ft_strlen_size_t(source_code);
    write_count = std::fwrite(source_code, 1,
            static_cast<std::size_t>(source_length), source_file);
    if (write_count != static_cast<std::size_t>(source_length))
    {
        (void)std::fclose(source_file);
        return (0);
    }
    if (std::fclose(source_file) != 0)
        return (0);
    return (1);
}

static int32_t runtime_run_command(const std::string &command)
{
    int system_status;

    std::fprintf(stderr, "[secure_wipe] run: %s\n", command.c_str());
    system_status = std::system(command.c_str());
    std::fprintf(stderr, "[secure_wipe] status: %d\n", system_status);
    if (system_status != 0)
        return (0);
    return (1);
}

static const char *runtime_child_source(void)
{
    return (
R"CPP(#include "Modules/CMA/CMA.hpp"
#include "Modules/SCMA/SCMA.hpp"
#include "Modules/SCMA/scma_internal.hpp"
#include <cstddef>
#include <cstdlib>
#include <cstring>

struct s_backend_state
{
    void *allocation_pointer;
    ft_size_t allocation_size;
    ft_bool observed_zero_before_free;
};

static void initialize_backend_state(s_backend_state *state)
{
    state->allocation_pointer = ft_nullptr;
    state->allocation_size = 0;
    state->observed_zero_before_free = FT_FALSE;
    return ;
}

static void *backend_allocate(ft_size_t size, void *user_data)
{
    s_backend_state *state;
    void *allocation_pointer;

    state = static_cast<s_backend_state *>(user_data);
    allocation_pointer = std::malloc(static_cast<std::size_t>(size));
    if (allocation_pointer == ft_nullptr)
        return (ft_nullptr);
    state->allocation_pointer = allocation_pointer;
    state->allocation_size = size;
    state->observed_zero_before_free = FT_FALSE;
    std::memset(allocation_pointer, 0x5A, static_cast<std::size_t>(size));
    return (allocation_pointer);
}

static void *backend_reallocate(void *memory_pointer, ft_size_t size,
    void *user_data)
{
    s_backend_state *state;
    void *new_pointer;

    state = static_cast<s_backend_state *>(user_data);
    if (memory_pointer == ft_nullptr)
        return (backend_allocate(size, user_data));
    if (size == 0)
    {
        state->allocation_pointer = ft_nullptr;
        state->allocation_size = 0;
        std::free(memory_pointer);
        return (ft_nullptr);
    }
    new_pointer = std::realloc(memory_pointer, static_cast<std::size_t>(size));
    if (new_pointer == ft_nullptr)
        return (ft_nullptr);
    state->allocation_pointer = new_pointer;
    state->allocation_size = size;
    return (new_pointer);
}

static void backend_deallocate(void *memory_pointer, void *user_data)
{
    s_backend_state *state;
    ft_size_t byte_index;
    unsigned char *byte_pointer;

    state = static_cast<s_backend_state *>(user_data);
    if (memory_pointer == ft_nullptr)
        return ;
    byte_pointer = static_cast<unsigned char *>(memory_pointer);
    byte_index = 0;
    state->observed_zero_before_free = FT_TRUE;
    while (byte_index < state->allocation_size)
    {
        if (byte_pointer[byte_index] != 0)
        {
            state->observed_zero_before_free = FT_FALSE;
            break ;
        }
        byte_index++;
    }
    state->allocation_pointer = ft_nullptr;
    state->allocation_size = 0;
    std::free(memory_pointer);
    return ;
}

static ft_size_t backend_get_allocation_size(const void *memory_pointer,
    void *user_data)
{
    s_backend_state *state;

    state = static_cast<s_backend_state *>(user_data);
    if (memory_pointer != state->allocation_pointer)
        return (0);
    return (state->allocation_size);
}

static ft_bool backend_owns_allocation(const void *memory_pointer,
    void *user_data)
{
    s_backend_state *state;

    state = static_cast<s_backend_state *>(user_data);
    if (memory_pointer != state->allocation_pointer)
        return (FT_FALSE);
    return (FT_TRUE);
}

int main(void)
{
    s_backend_state backend_state;
    cma_backend_hooks backend_hooks;
    scma_handle_accessor<int> accessor;
    scma_handle handle;
    unsigned char *heap_data;
    scma_block *blocks_data;
    ft_size_t heap_capacity;
    ft_size_t block_capacity;
    ft_size_t byte_index;

    initialize_backend_state(&backend_state);
    if (scma_initialize(128) != FT_ERR_SUCCESS)
        return (1);
    handle = scma_allocate(sizeof(int));
    if (scma_handle_is_valid(handle) != FT_TRUE)
        return (2);
    if (accessor.initialize(handle) != FT_ERR_SUCCESS)
        return (3);
    if (accessor.destroy() != FT_ERR_SUCCESS)
        return (4);
    if (accessor._handle.index != 0 || accessor._handle.generation != 0)
        return (5);
    if (scma_free(handle) != FT_ERR_SUCCESS)
        return (6);
    scma_shutdown();

    if (scma_initialize(128) != FT_ERR_SUCCESS)
        return (7);
    handle = scma_allocate(sizeof(int) * 2);
    if (scma_handle_is_valid(handle) != FT_TRUE)
        return (8);
    heap_data = scma_heap_data_ref();
    blocks_data = scma_blocks_data_ref();
    heap_capacity = scma_heap_capacity_ref();
    block_capacity = scma_block_capacity_ref();
    byte_index = 0;
    while (byte_index < heap_capacity)
    {
        heap_data[byte_index] = 0xA5;
        byte_index++;
    }
    byte_index = 0;
    while (byte_index < block_capacity * sizeof(scma_block))
    {
        reinterpret_cast<unsigned char *>(blocks_data)[byte_index] = 0xA5;
        byte_index++;
    }
    scma_test_secure_wipe_runtime();
    byte_index = 0;
    while (byte_index < heap_capacity)
    {
        if (heap_data[byte_index] != 0)
            return (9);
        byte_index++;
    }
    byte_index = 0;
    while (byte_index < block_capacity * sizeof(scma_block))
    {
        if (reinterpret_cast<unsigned char *>(blocks_data)[byte_index] != 0)
            return (10);
        byte_index++;
    }
    scma_shutdown();

    backend_hooks.allocate = &backend_allocate;
    backend_hooks.reallocate = &backend_reallocate;
    backend_hooks.deallocate = &backend_deallocate;
    backend_hooks.aligned_allocate = ft_nullptr;
    backend_hooks.get_allocation_size = &backend_get_allocation_size;
    backend_hooks.owns_allocation = &backend_owns_allocation;
    backend_hooks.user_data = &backend_state;
    if (cma_set_backend(&backend_hooks) != FT_ERR_SUCCESS)
        return (11);
    if (cma_backend_is_enabled() != FT_TRUE)
        return (12);
    backend_state.observed_zero_before_free = FT_FALSE;
    backend_state.allocation_pointer = ft_nullptr;
    backend_state.allocation_size = 0;
    {
        void *memory_pointer;

        memory_pointer = cma_malloc(64);
        if (memory_pointer == ft_nullptr)
            return (13);
        cma_bzero_and_free(memory_pointer);
        if (backend_state.observed_zero_before_free != FT_TRUE)
            return (14);
    }
    if (cma_clear_backend() != FT_ERR_SUCCESS)
        return (15);
    return (0);
}
)CPP");
}

static int32_t runtime_compile_and_run_helper(void)
{
    s_runtime_file_guard file_guard;
    std::string project_root;
    const char *compiler;
    std::string compile_command;
    std::string run_command;
    char source_template[] = "/tmp/libft_cma_scma_secure_wipe_XXXXXX";
    int32_t source_descriptor;

    project_root = runtime_project_root();
    if (project_root.empty())
        return (0);
    source_descriptor = mkstemp(source_template);
    if (source_descriptor < 0)
        return (0);
    (void)close(source_descriptor);
    (void)std::snprintf(file_guard.source_path,
        sizeof(file_guard.source_path), "%s", source_template);
    file_guard.source_path_active = FT_TRUE;
    (void)std::snprintf(file_guard.executable_path,
        sizeof(file_guard.executable_path), "%s.bin", file_guard.source_path);
    file_guard.executable_path_active = FT_TRUE;
    if (runtime_write_source_file(file_guard.source_path,
            runtime_child_source()) == 0)
        return (0);
    compiler = std::getenv("CXX");
    if (compiler == ft_nullptr || compiler[0] == '\0')
        compiler = "c++";
    compile_command = compiler;
    compile_command += " -x c++ -O3 -Wall -Wextra -Werror -std=c++17 -pthread";
    compile_command += " -Wno-missing-declarations -DLIBFT_TEST_BUILD";
    compile_command += " -DLIBFT_INTERNAL_HEADERS -I";
    compile_command += project_root;
    compile_command += " -I";
    compile_command += project_root;
    compile_command += "/Modules ";
    compile_command += file_guard.source_path;
    compile_command += " -x none ";
    compile_command += project_root;
    compile_command += "/Full_Libft_test.a";
#ifdef __APPLE__
    compile_command += " -pthread -lz";
#else
    compile_command += " -pthread -Wl,--allow-multiple-definition -rdynamic";
    compile_command += " -lz -ldl";
#endif
    compile_command += " -o ";
    compile_command += file_guard.executable_path;
    if (runtime_run_command(compile_command) == 0)
        return (0);
    (void)unlink(file_guard.source_path);
    file_guard.source_path_active = FT_FALSE;
    run_command = file_guard.executable_path;
    if (runtime_run_command(run_command) == 0)
        return (0);
    file_guard.destroy();
    return (1);
}

FT_TEST(test_cma_scma_secure_wipe_runtime_compilation_and_cleanup)
{
    FT_ASSERT_EQ(1, runtime_compile_and_run_helper());
    return (1);
}
