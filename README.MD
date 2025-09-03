## about
memm is a c memory manager that helps developers to keep track of current and previously allocated memory in a easy way. It overrides standart memory allocation operations, keeping track whenever a call to malloc/calloc/realoc/free is made, recording and storing it.
At any time the user can then use the functions to check the usage and see if everything is in the expected order, it'll show the line and file from each call was made from and detect pottential leaks (memory not yet freed).

## usage
Although functions like mmem_* alloc/calloc/realoc/free exists, the library was designed to override the standart functions instead, if no memory leaks were detected while testing it is safe to go ahead and define the macro **MEMM_DONT_OVERRIDE_STD** wich will use standart functions again, removing any overhead memm may have on execution time.

* Call ```memm_init()``` and ```memm_shutdown()``` to properly initialize/shutdown the library.
* Call ```memm_get_stats_string(char*, size_t)``` to retrieve a comprehensive summary of memory management statistics as a formatted string. This function provides an overview of the memory manager's current state, including total memory operations, usage patterns, and performance metrics.
    * === MEMORY STATISTICS ===
    * Total allocated:      15400 bytes
    * Total freed:          10200 bytes
    * Current usage:        5200 bytes
    * Peak memory usage:    8900 bytes
    * Allocation calls:     25
    * Free calls:           18
    * Potential leaks:      7 objects
    * Hash table size:      1024 buckets
* Call ```memm_get_allocations_string(char*, size_t)``` to enumerates all currently active memory allocations as a formatted string. This function provides a detailed listing of every memory block that has been allocated but not yet freed, including precise location information for debugging.
    * === CURRENT ALLOCATIONS ===
    * 0x7f8aab402600:    400 bytes @ main.c:15
    * 0x7f8aab402800:   1200 bytes @ utils.c:42
    * 0x7f8aab402e00:    800 bytes @ data_processor.c:103
    * Total: 3 allocations, 2400 bytes
* Call ```memm_get_leaks_string(char*, size_t)``` to generates a memory leak report as a formatted string. This function identifies all memory allocations that remain active at the time of call (typically used during shutdown), providing detailed information to help locate and fix memory leaks.
    * === MEMORY LEAK REPORT ===
    *   LEAK:    400 bytes at 0x7f8aab402600 (main.c:15)
    *   LEAK:   1200 bytes at 0x7f8aab402800 (utils.c:42)
    *   TOTAL LEAKS: 2 allocations, 1600 bytes

Check [example.c](example.c) for a compreensive usage guide.

## defines/macros
* **MEMM_DONT_OVERRIDE_STD** : Don't overrides the standard malloc/calloc/realoc/free functions.
* **MEMM_HASH_TABLE_SIZE** : Defines how many allocations the manager can keep count. Default is 2048. Must be power of 2 for efficiency pourpuses, may be increased.
* **MEMM_ENABLE_LOGGING** : Allows to easily print status information about tracked and previously tracked memory. Also outputs erros and warnings on terminal if any occurred.
    * **MEMM_MAX_STRING_LENGTH** : Changes how many bytes the string from status information used, increase it if 2028 is not enough.

## build
Both memm.h/memm.c are designed to be included alongside the project, but using another header to define desired macros before including memm.h is a good idea.

## license
[MIT](https://choosealicense.com/licenses/mit/) license.
