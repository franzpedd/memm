#ifndef MEMM_HEADER_INCLUDED
#define MEMM_HEADER_INCLUDED

#include <stddef.h>
#include <stdbool.h>

/// @brief sets how many allocations the system can hold information about
#ifndef MEMM_HASH_TABLE_SIZE
    #define MEMM_HASH_TABLE_SIZE 2048
#endif

/// @brief compile-time validation that size is power of 2
#if (MEMM_HASH_TABLE_SIZE & (MEMM_HASH_TABLE_SIZE - 1)) != 0
    #error "MEMM_HASH_TABLE_SIZE must be a power of 2 for hashing efficiency"
#endif

/// @brief compilation options
#if defined(MEMM_BUILD_SHARED) // shared library
    #if defined(_WIN32) || defined(_WIN64)
        #if defined(MEMM_EXPORTS)
            #define MEMM_API __declspec(dllexport)
        #else
            #define MEMM_API __declspec(dllimport)
        #endif
    #elif defined(__linux__) && !defined(__ANDROID__)
        #define MEMM_API __attribute__((visibility("default")))
    #else
        #define MEMM_API
    #endif
#else
    #define MEMM_API // static library
#endif

#ifdef __cplusplus
extern "C" {
#endif

///@brief initializes the memory manager
MEMM_API void memm_init();

/// @brief shutdows the memory manager
MEMM_API void memm_shutdown();

/// @brief allocates memory
MEMM_API void* memm_malloc(size_t size, const char* file, int line);

/// @brief zeroed-allocates memory
MEMM_API void* memm_calloc(size_t num, size_t size, const char* file, int line);

/// @brief realocates/changes size of a previously allocated memory block
MEMM_API void* memm_realloc(void* ptr, size_t size, const char* file, int line);

/// @brief deallocates memory
MEMM_API void memm_free(void* ptr, const char* file, int line);

/// @brief returns how much of the memory is being currently used
MEMM_API size_t memm_get_current_usage();

/// @brief returns the peak usage, wich is how much bytes were simultaneously allocated
MEMM_API size_t memm_get_peak_usage();

/// @brief returns how many allocation calls were issued
MEMM_API size_t memm_get_allocation_count();

/// @brief returns how may free calls were issued
MEMM_API size_t memm_get_free_count();

/// @brief fills-out a buffer with statistics about the memory manager
MEMM_API int memm_get_stats_string(char* buffer, size_t buffer_size);

/// @brief fills-out a buffer with information about current tracked and no-long tracked allocations
MEMM_API int memm_get_allocations_string(char* buffer, size_t buffer_size);

/// @brief fills-out a buffer with information about pottentially memory leaks
MEMM_API int memm_get_leaks_string(char* buffer, size_t buffer_size);

/// @brief helper macros for stats, must have logging enable
#ifdef MEMM_ENABLE_LOGGING

/// @brief sets how many bytes the helper-macros string have at max, increase it if 2048 is not enough
#ifndef MEMM_MAX_STRING_LENGTH
    #define MEMM_MAX_STRING_LENGTH 2048
#endif

#define memm_print_stats() do { \
    char _buf[MEMM_MAX_STRING_LENGTH]; \
    if (memm_get_stats_string(_buf, sizeof(_buf)) > 0) printf("%s", _buf); \
} while (0)

#define memm_print_allocations() do { \
    char _buf[MEMM_MAX_STRING_LENGTH]; \
    if (memm_get_allocations_string(_buf, sizeof(_buf)) > 0) printf("%s", _buf); \
} while (0)

#define memm_print_leaks() do { \
    char _buf[MEMM_MAX_STRING_LENGTH]; \
    if (memm_get_leaks_string(_buf, sizeof(_buf)) > 0) printf("%s", _buf); \
} while (0)
 #endif

/// @brief re-defines memory functions to use the memm, keeping track of memory allocations
#ifndef MEMM_DONT_OVERRIDE_STD
    #include <stdlib.h>
    #undef malloc
    #undef calloc
    #undef realloc
    #undef free
    #define malloc(size) memm_malloc(size, __FILE__, __LINE__)
    #define calloc(num, size) memm_calloc(num, size, __FILE__, __LINE__)
    #define realloc(ptr, size) memm_realloc(ptr, size, __FILE__, __LINE__)
    #define free(ptr) memm_free(ptr, __FILE__, __LINE__)
#endif

#ifdef __cplusplus
}
#endif

#endif // MEMM_HEADER_INCLUDED