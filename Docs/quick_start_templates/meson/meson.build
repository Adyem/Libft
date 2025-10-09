project('libft_quick_start', 'cpp', default_options: ['cpp_std=c++17'])

option('libft_root', type: 'string', value: 'external/libft', description: 'Path to the Libft checkout')

fs = import('fs')

libft_root = join_paths(meson.project_source_root(), get_option('libft_root'))
libft_sources = []
foreach path : fs.glob(libft_root, '**/*.cpp', allow_broken_symlinks: false)
    if path.contains('/Test/')
        continue
    endif
    if path.contains('/tools/')
        continue
    endif
    libft_sources += path
endforeach

if libft_sources.length() == 0
    error('No Libft sources found. Check the libft_root option.')
endif

libft_includes = include_directories(get_option('libft_root'))
libft_lib = static_library('libft_core', libft_sources, include_directories: libft_includes)

executable('quick_start', 'src/main.cpp', include_directories: libft_includes, link_with: libft_lib)
