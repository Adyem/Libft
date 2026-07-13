#include "../Modules/System_utils/test_system_utils_runner.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
# include <windows.h>
#endif

namespace
{
    typedef std::filesystem::path test_path;

    static bool test_string_has_prefix(const std::string &value,
            const char *prefix)
    {
        const std::string prefix_string(prefix);
        return (value.compare(0, prefix_string.size(), prefix_string) == 0);
    }

    static bool test_string_has_suffix(const std::string &value,
            const char *suffix)
    {
        const std::string suffix_string(suffix);
        if (value.size() < suffix_string.size())
        {
            return (false);
        }
        return (value.compare(value.size() - suffix_string.size(),
                suffix_string.size(), suffix_string) == 0);
    }

    static void test_remove_path(const test_path &path)
    {
        std::error_code error_code;
        std::filesystem::remove_all(path, error_code);
        return ;
    }

    static bool test_is_generated_object_directory(const std::string &name)
    {
        return (test_string_has_prefix(name, "objs_test")
            || test_string_has_prefix(name, "temp_objs_test"));
    }

    static void test_remove_generated_object_directories(
            const test_path &root_path)
    {
        std::error_code error_code;
        std::filesystem::recursive_directory_iterator iterator(root_path,
            std::filesystem::directory_options::skip_permission_denied,
            error_code);
        const std::filesystem::recursive_directory_iterator end_iterator;
        while (iterator != end_iterator)
        {
            const std::filesystem::directory_entry entry = *iterator;
            std::error_code entry_error_code;
            if (entry.is_directory(entry_error_code)
                && test_is_generated_object_directory(
                    entry.path().filename().string()))
            {
                test_remove_path(entry.path());
                iterator.disable_recursion_pending();
            }
            iterator.increment(error_code);
        }
        return ;
    }

    static void test_remove_generated_archives(const test_path &root_path)
    {
        std::error_code error_code;
        std::filesystem::recursive_directory_iterator iterator(root_path,
            std::filesystem::directory_options::skip_permission_denied,
            error_code);
        const std::filesystem::recursive_directory_iterator end_iterator;
        while (iterator != end_iterator)
        {
            const std::filesystem::directory_entry entry = *iterator;
            std::error_code entry_error_code;
            if (entry.is_regular_file(entry_error_code)
                && test_string_has_suffix(entry.path().filename().string(),
                    "_test.a"))
            {
                test_remove_path(entry.path());
            }
            iterator.increment(error_code);
        }
        return ;
    }

    static void test_remove_test_logs(const test_path &test_root_path)
    {
        std::error_code error_code;
        std::filesystem::directory_iterator iterator(test_root_path,
            std::filesystem::directory_options::skip_permission_denied,
            error_code);
        const std::filesystem::directory_iterator end_iterator;
        while (iterator != end_iterator)
        {
            const std::filesystem::directory_entry entry = *iterator;
            const std::string name = entry.path().filename().string();
            std::error_code entry_error_code;
            if (entry.is_regular_file(entry_error_code)
                && (test_string_has_prefix(name, ".libft_")
                    || name == "full_test_run.log"))
            {
                test_remove_path(entry.path());
            }
            iterator.increment(error_code);
        }
        return ;
    }

    static test_path test_find_libft_root(const test_path &executable_path)
    {
        std::error_code error_code;
        const test_path current_path = std::filesystem::current_path(
            error_code);
        if (!error_code && std::filesystem::is_directory(current_path / "Test")
            && std::filesystem::is_directory(current_path / "Modules"))
        {
            return (current_path);
        }

        const test_path executable_parent = executable_path.parent_path();
        if (!executable_parent.empty()
            && executable_parent.filename() == "Test")
        {
            return (executable_parent.parent_path());
        }
        if (current_path.filename() == "Test")
        {
            return (current_path.parent_path());
        }
        return (current_path);
    }

    static void test_cleanup_generated_artifacts(const test_path &root_path)
    {
        const test_path test_root_path = root_path / "Test";
        test_remove_path(test_root_path / "Full_Libft_test.a");
        test_remove_path(test_root_path / "Full_Libft_test_debug.a");
        test_remove_path(test_root_path / "libft_tests");
        test_remove_path(test_root_path / "libft_tests.exe");
        test_remove_path(test_root_path / "libft_tests_debug");
        test_remove_path(test_root_path / "libft_tests_debug.exe");
        test_remove_path(test_root_path / "libft_efficiency_tests");
        test_remove_path(test_root_path / "libft_efficiency_tests.exe");
        test_remove_path(root_path / "test_failures.log");
        test_remove_path(root_path / "test_file_io.txt");
        test_remove_path(root_path / "test_cmp_system_io.txt");
        test_remove_path(root_path / "test_su_file_stream.txt");
        test_remove_path(test_root_path / ".libft_progress");
        test_remove_path(test_root_path / ".libft_progress.lock");
        test_remove_path(test_root_path / ".libft_test_output_lock");
        test_remove_generated_object_directories(test_root_path);
        test_remove_generated_object_directories(root_path / "Modules");
        test_remove_generated_archives(root_path / "Modules");
        test_remove_test_logs(test_root_path);
        return ;
    }

    static void test_schedule_executable_cleanup(const test_path &path,
            const test_path &test_root_path)
    {
#ifdef _WIN32
        std::error_code error_code;
        if (std::filesystem::remove(path, error_code))
        {
            return ;
        }
        const test_path cleanup_script_path = test_root_path
            / ".libft_test_cleanup.cmd";
        std::ofstream cleanup_script(cleanup_script_path);
        if (!cleanup_script)
        {
            return ;
        }
        const std::string executable_string = path.string();
        cleanup_script << "@echo off\r\n";
        cleanup_script << ":retry\r\n";
        cleanup_script << "del /f /q \"" << executable_string
            << "\" >nul 2>&1\r\n";
        cleanup_script << "if exist \"" << executable_string << "\" (\r\n";
        cleanup_script << "  ping 127.0.0.1 -n 2 >nul\r\n";
        cleanup_script << "  goto retry\r\n";
        cleanup_script << ")\r\n";
        cleanup_script << "del /f /q \"%~f0\" >nul 2>&1\r\n";
        cleanup_script.close();
        const std::string command = "cmd.exe /d /c call \""
            + cleanup_script_path.string() + "\"";
        std::vector<char> command_buffer(command.begin(), command.end());
        STARTUPINFOA startup_info;
        PROCESS_INFORMATION process_information;

        std::memset(&startup_info, 0, sizeof(startup_info));
        std::memset(&process_information, 0, sizeof(process_information));
        startup_info.cb = sizeof(startup_info);
        command_buffer.push_back('\0');
        if (CreateProcessA(nullptr, command_buffer.data(), nullptr, nullptr,
                FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup_info,
                &process_information) != 0)
        {
            CloseHandle(process_information.hThread);
            CloseHandle(process_information.hProcess);
        }
#else
        std::error_code error_code;
        std::filesystem::remove(path, error_code);
#endif
        return ;
    }
}

int main(int argc, char **argv)
{
    std::error_code error_code;
    test_path executable_path;
    if (argc > 0 && argv != NULL && argv[0] != NULL)
    {
        executable_path = std::filesystem::absolute(argv[0], error_code);
    }
    const test_path root_path = test_find_libft_root(executable_path);
    const int test_status = ft_run_registered_tests();
    test_cleanup_generated_artifacts(root_path);
    test_schedule_executable_cleanup(executable_path, root_path / "Test");
    return (test_status);
}
