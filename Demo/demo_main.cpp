#include "demo_game.hpp"

#include <cstdio>

int main(void)
{
    int32_t error_code;

    std::fprintf(stdout, "Launching DUMB maze 3D. Close the window to exit.\n");
    error_code = demo_run();
    if (error_code != FT_ERR_SUCCESS)
    {
        std::fprintf(stderr, "DUMB maze 3D failed with error code %d\n", error_code);
        return (1);
    }
    return (0);
}
