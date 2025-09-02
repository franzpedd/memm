#define _CRT_SECURE_NO_WARNINGS  // MSVC-specific for safe functions
#include "memm.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/// @brief undefine macros to use real functions
#undef malloc
#undef calloc
#undef realloc
#undef free
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////// Internal Implementation

/// @brief holds an alocation information
typedef struct memm_allocation
{
    void* ptr;
    size_t size;
    const char* file;
    int line;
    time_t timestamp;
    struct memm_allocation* next;
} memm_allocation_t;

/// @brief holds the memm state, wich keeps tracks of all memory allocated stuff
typedef struct memm
{
    memm_allocation_t* hash_table[MEMM_HASH_TABLE_SIZE];
    size_t total_allocated;     // bytes allocated
    size_t total_freed;         // bytes freed
    size_t peak_memory;         // max memory simultaneosly allocated, used 
    size_t allocation_count;    // allocations calls count
    size_t free_count;          // free calls count
} memm_t;

/// @brief global state
static memm_t g_memm = { 0 };

/// @brief hashes the function 
static size_t memm_hash_ptr(void* ptr) {
    return ((size_t)ptr) & (MEMM_HASH_TABLE_SIZE - 1);
}

/// @brief register an allocation
static void memm_register_allocation(void* ptr, size_t size, const char* file, int line)
{
    if (!ptr) return;
    
    size_t hash = memm_hash_ptr(ptr);
    memm_allocation_t* alloc = (memm_allocation_t*)malloc(sizeof(memm_allocation_t));
    
    if (!alloc) {
        #ifdef MEMM_ENABLE_LOGGING
        fprintf(stderr, "MEMM-ERROR: Failed to register allocation for %p\n", ptr);
        #endif
        return;
    }
    
    alloc->ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    alloc->timestamp = time(NULL);
    alloc->next = g_memm.hash_table[hash];
    g_memm.hash_table[hash] = alloc;
    
    g_memm.total_allocated += size;
    g_memm.allocation_count++;
    
    size_t current_usage = g_memm.total_allocated - g_memm.total_freed;
    if (current_usage > g_memm.peak_memory) {
        g_memm.peak_memory = current_usage;
    }
}

/// @brief unregister the allocation
static bool memm_unregister_allocation(void* ptr, const char* file, int line)
{
    if (!ptr) return true;
    
    size_t hash = memm_hash_ptr(ptr);
    memm_allocation_t** current = &g_memm.hash_table[hash];
    
    while (*current) {
        if ((*current)->ptr == ptr) {
            memm_allocation_t* to_free = *current;
            *current = to_free->next;
            
            g_memm.total_freed += to_free->size;
            g_memm.free_count++;
            
            free(to_free);
            return true;
        }
        current = &(*current)->next;
    }
    
    #ifdef MEMM_ENABLE_LOGGING
    fprintf(stderr, "MEMM-ERROR: Attempt to free unknown pointer %p (%s:%d)\n", ptr, file, line);
    #endif
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////// External Implementations

void memm_init()
{
    memset(&g_memm, 0, sizeof(g_memm));
    #ifdef MEMM_ENABLE_LOGGING
    printf("Memory manager initialized with %d buckets\n", MEMM_HASH_TABLE_SIZE);
    #endif
}

void memm_shutdown()
{
    // cleanup tracking structures
    for (size_t i = 0; i < MEMM_HASH_TABLE_SIZE; i++) {
        memm_allocation_t* current = g_memm.hash_table[i];
        while (current) {
            memm_allocation_t* next = current->next;
            free(current);
            current = next;
        }
        g_memm.hash_table[i] = NULL;
    }
    #ifdef MEMM_ENABLE_LOGGING
    printf("Memory manager shutdown complete\n");
    #endif
}

void* memm_malloc(size_t size, const char* file, int line)
{
    void* ptr = malloc(size);
    if (ptr) {
        memm_register_allocation(ptr, size, file, line);
    } 

    else {
        #ifdef MEMM_ENABLE_LOGGING
        fprintf(stderr, "MEMM-ERROR: malloc failed for %zu bytes (%s:%d)\n", size, file, line);
        #endif
    }
    return ptr;
}

void* memm_calloc(size_t num, size_t size, const char *file, int line)
{
    void* ptr = calloc(num, size);
    if (ptr) {
        memm_register_allocation(ptr, num * size, file, line);
    } 

    else {
        #ifdef MEMM_ENABLE_LOGGING
        fprintf(stderr, "MEMM-ERROR: calloc failed for %zu elements of %zu bytes (%s:%d)\n", num, size, file, line);
        #endif
    }
    return ptr;
}

void* memm_realloc(void *ptr, size_t size, const char *file, int line)
{
    if (ptr) {
        memm_unregister_allocation(ptr, file, line);
    }
    
    void* new_ptr = realloc(ptr, size);
    if (new_ptr) {
        memm_register_allocation(new_ptr, size, file, line);
    } 

    else if (size > 0) {
        #ifdef MEMM_ENABLE_LOGGING
        fprintf(stderr, "MEMM-ERROR: realloc failed for %zu bytes (%s:%d)\n", size, file, line);
        #endif
    }
    return new_ptr;
}

void memm_free(void *ptr, const char *file, int line)
{
    if (memm_unregister_allocation(ptr, file, line)) {
        free(ptr);
    } 

    else {
        #ifdef MEMM_ENABLE_LOGGING
        fprintf(stderr, "MEMM-WARN: free on %zu bytes on an untracked memory (%s:%d)\n", size, file, line);
        #endif
        // if not found in our tracking, still free it to avoid real leaks
        free(ptr);
    }
}

size_t memm_get_current_usage()
{
    return g_memm.total_allocated - g_memm.total_freed;
}

size_t memm_get_peak_usage()
{
    return g_memm.peak_memory;
}

size_t memm_get_allocation_count()
 {
    return g_memm.allocation_count;
}

size_t memm_get_free_count()
 {
    return g_memm.free_count;
}

int memm_get_stats_string(char *buffer, size_t buffer_size)
{
    if(!buffer || buffer_size <= 0) {
        return -1;
    }

    int written = snprintf(buffer, buffer_size,
        "=== MEMORY STATISTICS ===\n"
        "Total allocated:      %zu bytes\n"
        "Total freed:          %zu bytes\n"
        "Current usage:        %zu bytes\n"
        "Peak memory usage:    %zu bytes\n"
        "Allocation calls:     %zu\n"
        "Free calls:           %zu\n"
        "Potential leaks:      %zu objects\n"
        "Hash table size:      %d buckets\n",
        g_memm.total_allocated,
        g_memm.total_freed,
        memm_get_current_usage(),
        memm_get_peak_usage(),
        memm_get_allocation_count(),
        memm_get_free_count(),
        memm_get_allocation_count() - memm_get_free_count(),
        MEMM_HASH_TABLE_SIZE
    );

    if (written < 0) {
        buffer[0] = '\0';
        return -1;
    }
    
    if ((size_t)written >= buffer_size) {
        buffer[buffer_size - 1] = '\0';
        return buffer_size - 1;
    }
    
    return written;
}

int memm_get_allocations_string(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) {
        return -1;
    }
    
    char* cursor = buffer;
    size_t remaining = buffer_size;
    int total_written = 0;
    int written = 0;
    
    written = snprintf(cursor, remaining, "=== CURRENT ALLOCATIONS ===\n");
    if (written < 0) {
        buffer[0] = '\0';
        return -1;
    }

    if ((size_t)written >= remaining) {
        buffer[buffer_size - 1] = '\0';
        return buffer_size - 1;
    }

    cursor += written;
    remaining -= written;
    total_written += written;
    
    size_t total_count = 0;
    size_t total_bytes = 0;
    
    for (size_t i = 0; i < MEMM_HASH_TABLE_SIZE && remaining > 1; i++) {
        memm_allocation_t* current = g_memm.hash_table[i];
        while (current && remaining > 1) {
            written = snprintf(cursor, remaining, "  %p: %6zu bytes @ %s:%d\n", current->ptr, current->size, current->file, current->line);
            
            if (written < 0) break;
            if ((size_t)written >= remaining) written = remaining - 1;
            
            cursor += written;
            remaining -= written;
            total_written += written;
            total_count++;
            total_bytes += current->size;
            current = current->next;
        }
    }
    
    if (total_count == 0 && remaining > 1) {
        written = snprintf(cursor, remaining, "  No active allocations\n");
        if (written > 0) {
            if ((size_t)written >= remaining) {
                written = remaining - 1;
            }
            total_written += written;
        }
    }

    else if (remaining > 1) {
        written = snprintf(cursor, remaining, "  Total: %zu allocations, %zu bytes\n", total_count, total_bytes);
        if (written > 0) {
            if ((size_t)written >= remaining) {
                written = remaining - 1;
            }
            total_written += written;
        }
    }
    
    // ensure null termination
    if (remaining == 0 && buffer_size > 0) {
        buffer[buffer_size - 1] = '\0';
    }
    
    return total_written;
}

int memm_get_leaks_string(char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0) {
        return -1;
    }
    
    char* cursor = buffer;
    size_t remaining = buffer_size;
    int total_written = 0;
    int written = 0;
    
    written = snprintf(cursor, remaining, "=== MEMORY LEAK REPORT ===\n");
    if (written < 0) {
        buffer[0] = '\0';
        return -1;
    }

    if ((size_t)written >= remaining) {
        buffer[buffer_size - 1] = '\0';
        return buffer_size - 1;
    }

    cursor += written;
    remaining -= written;
    total_written += written;
    
    size_t leak_count = 0;
    size_t leak_bytes = 0;
    
    for (size_t i = 0; i < MEMM_HASH_TABLE_SIZE && remaining > 1; i++) {
        memm_allocation_t* current = g_memm.hash_table[i];
        while (current && remaining > 1) {
            written = snprintf(cursor, remaining, "  LEAK: %6zu bytes at %p (%s:%d)\n", current->size, current->ptr, current->file, current->line);
            
            if (written < 0) break;
            if ((size_t)written >= remaining) written = remaining - 1;
            
            cursor += written;
            remaining -= written;
            total_written += written;
            leak_count++;
            leak_bytes += current->size;
            current = current->next;
        }
    }
    
    if (leak_count == 0 && remaining > 1) {
        written = snprintf(cursor, remaining, "  No memory leaks detected!\n");
        if (written > 0) {
            if ((size_t)written >= remaining) {
                written = remaining - 1;
            }
            total_written += written;
        }
    }

    else if (remaining > 1) {
        written = snprintf(cursor, remaining, "  TOTAL LEAKS: %zu allocations, %zu bytes\n", leak_count, leak_bytes);
        if (written > 0) {
            if ((size_t)written >= remaining) {
                written = remaining - 1;
            }
            total_written += written;
        }
    }
    
    // ensure null termination
    if (remaining == 0 && buffer_size > 0) {
        buffer[buffer_size - 1] = '\0';
    }
    
    return total_written;
}
