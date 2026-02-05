#ifndef COMPATEBILITY_FILE_WATCH_HPP
#define COMPATEBILITY_FILE_WATCH_HPP

#include "../File/file_watch_event.hpp"

struct cmp_file_watch_context;

cmp_file_watch_context *cmp_file_watch_create(void);
void cmp_file_watch_destroy(cmp_file_watch_context *context);
int cmp_file_watch_start(cmp_file_watch_context *context, const char *path);
void cmp_file_watch_stop(cmp_file_watch_context *context);
bool cmp_file_watch_wait_event(cmp_file_watch_context *context,
    cmp_file_watch_event *event);
int cmp_file_watch_last_error(const cmp_file_watch_context *context);

#endif
