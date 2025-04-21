#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #define PATH_SEP '\\'
#else
  #include <sys/stat.h>
  #include <unistd.h>
  #define PATH_SEP '/'
#endif

int dir_exists(const char *rel_path) {
    char *fixed = normalize_path(rel_path);
    if (!fixed)
		return 0;
    int exists = 0;
#if defined(_WIN32) || defined(_WIN64)
    DWORD attr = GetFileAttributesA(fixed);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        exists = 1;
#else
    struct stat st;
    if (stat(fixed, &st) == 0 && S_ISDIR(st.st_mode))
        exists = 1;
#endif
    free(fixed);
    return exists;
}
