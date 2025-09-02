#define _CRT_SECURE_NO_WARNINGS  // MSVC-specific for safe functions
#include <stdio.h>
#include <string.h>

#define MEMM_ENABLE_LOGGING
#define MEMM_MAX_STRING_LENGTH 2048
#include "memm.h"

void test_function(void)
{
    printf("Testing memory allocation...\n");
    
    int* numbers = (int*)malloc(100 * sizeof(int));
    char* text = (char*)calloc(256, sizeof(char));
    double* values = (double*)malloc(50 * sizeof(double));
    
    for (int i = 0; i < 100; i++) {
        numbers[i] = i * i;
    }
    
    free(numbers);
    free(text);
    //free(values); // values is intentionally not freed
}

int main() {
    memm_init();
    
    printf("Memory Manager Test Program\n");
    printf("===========================\n");
    
    test_function();
    
    // method 1: using the buffer approach
    char buffer[MEMM_MAX_STRING_LENGTH];
    
    if (memm_get_stats_string(buffer, sizeof(buffer)) > 0) {
        printf("%s\n", buffer);
    }
    
    if (memm_get_allocations_string(buffer, sizeof(buffer)) > 0) {
        printf("%s\n", buffer);
    }

    if (memm_get_leaks_string(buffer, sizeof(buffer)) > 0) {
        printf("%s\n", buffer);
    }
    
    // method 2: using the convenience macros, only works when MEMM_ENABLE_LOGGING is defined (and a good enough MEMM_MAX_STRING_LENGTH size)
    memm_print_stats();
    memm_print_allocations();
    memm_print_leaks();
    
    // method 3: writing to a file
    FILE* log_file = fopen("example_log.txt", "w");
    if (log_file) {
        char file_buffer[MEMM_MAX_STRING_LENGTH];
        
        if (memm_get_stats_string(file_buffer, sizeof(file_buffer)) > 0) {
            fprintf(log_file, "%s\n", file_buffer);
        }
        
        if (memm_get_allocations_string(file_buffer, sizeof(file_buffer)) > 0) {
            fprintf(log_file, "%s\n", file_buffer);
        }
    
        if (memm_get_leaks_string(file_buffer, sizeof(file_buffer)) > 0) {
            fprintf(log_file, "%s\n", file_buffer);
        }
        
        fclose(log_file);
    }
    
    memm_shutdown();
    return 0;
}