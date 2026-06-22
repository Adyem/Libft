#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Config/config.hpp"
#include "../../Modules/File/file_utils.hpp"

config_data *config_reload_file(const char *filename)
{
    return (config_load_file(filename));
}

int32_t config_save_file(const config_data *config, const char *filename)
{
    return (config_write_file(config, filename));
}

ft_file_watch *config_watch_file(const char *filename,
    file_watch_callback callback, void *user_data)
{
    ft_file_watch *file_watch;
    char *directory_name;
    int32_t initialize_error;
    int32_t watch_error;

    if (filename == ft_nullptr || callback == ft_nullptr)
        return (ft_nullptr);
    directory_name = file_path_dirname(filename);
    if (directory_name == ft_nullptr)
        return (ft_nullptr);
    file_watch = new (std::nothrow) ft_file_watch();
    if (file_watch == ft_nullptr)
    {
        cma_free(directory_name);
        return (ft_nullptr);
    }
    initialize_error = file_watch->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete file_watch;
        cma_free(directory_name);
        return (ft_nullptr);
    }
    watch_error = file_watch->watch_directory(directory_name, callback, user_data);
    cma_free(directory_name);
    if (watch_error != FT_ERR_SUCCESS)
    {
        (void)file_watch->destroy();
        delete file_watch;
        return (ft_nullptr);
    }
    return (file_watch);
}
