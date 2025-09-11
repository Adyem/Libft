#include "system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *su_get_home_directory(void)
{
#if defined(_WIN32) || defined(_WIN64)
    char    *home;
    char    *home_drive;
    char    *home_path;

    home = ft_getenv("USERPROFILE");
    if (home != ft_nullptr)
        return (home);
    home_drive = ft_getenv("HOMEDRIVE");
    home_path = ft_getenv("HOMEPATH");
    if (home_drive == ft_nullptr || home_path == ft_nullptr)
        return (ft_nullptr);
    return (ft_strjoin_multiple(2, home_drive, home_path));
#else
    return (ft_getenv("HOME"));
#endif
}

