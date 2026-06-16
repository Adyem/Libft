#include "test_internal.hpp"
#include "../Modules/Networking/networking.hpp"
#include "../Modules/System_utils/test_system_utils_runner.hpp"

#if defined(_WIN32)
FT_TEST(test_nw_poll_probe)
{
    int pipe_descriptors[2];
    int read_descriptors[3];
    int poll_result;
    ssize_t write_result;

    if (pipe(pipe_descriptors) != 0)
        return (0);
    write_result = write(pipe_descriptors[1], "x", 1);
    if (write_result != 1)
    {
        close(pipe_descriptors[0]);
        close(pipe_descriptors[1]);
        return (0);
    }
    read_descriptors[0] = -1;
    read_descriptors[1] = pipe_descriptors[0];
    read_descriptors[2] = -1;
    poll_result = nw_poll(read_descriptors, 3, ft_nullptr, 0, 100);
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    if (poll_result != 1)
        return (0);
    if (read_descriptors[0] != -1)
        return (0);
    if (read_descriptors[1] != pipe_descriptors[0])
        return (0);
    if (read_descriptors[2] != -1)
        return (0);
    return (1);
}
#endif
