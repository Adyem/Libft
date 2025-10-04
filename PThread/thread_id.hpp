#ifndef PTHREAD_THREAD_ID_HPP
#define PTHREAD_THREAD_ID_HPP

#ifdef _WIN32
    #include <windows.h>
    using pt_thread_id_type = DWORD;
    #define THREAD_ID GetCurrentThreadId()
#else
    #include <pthread.h>
    using pt_thread_id_type = pthread_t;
    #define THREAD_ID pthread_self()
#endif

extern thread_local pt_thread_id_type pt_thread_id;
pt_thread_id_type pt_thread_self();

#endif
