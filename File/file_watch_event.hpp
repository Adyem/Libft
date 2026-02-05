#ifndef FILE_FILE_WATCH_EVENT_HPP
#define FILE_FILE_WATCH_EVENT_HPP

enum file_watch_event_type
{
    FILE_WATCH_EVENT_CREATE = 1,
    FILE_WATCH_EVENT_MODIFY,
    FILE_WATCH_EVENT_DELETE
};

struct cmp_file_watch_event
{
    file_watch_event_type    event_type;
    bool                     has_name;
    char                     name[512];
};

#endif
