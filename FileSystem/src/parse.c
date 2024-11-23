#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "../include/parse.h"
#include "../include/utils.h"

int tokenize_input(const char *input, char **result, size_t *allocated_size) {
    if (!input) {
        perror(RED "tokenize_input() : input is NULL" RESET);
        return -1; 
    }

    if (!result) {
        perror(RED "tokenize_input() : result is NULL" RESET);
        return -2; 
    }

    if (!allocated_size) {
        perror(RED "tokenize_input() : allocated_size is NULL" RESET);
        return -3; 
    }

    size_t len = strlen(input);
    if (len == 0) {
        perror(RED "tokenize_input() : input is an empty string" RESET);
        return -4; 
    }

    // Allocate memory for a modifiable copy of the input
    char *input_copy = strdup(input);
    if (!input_copy) {
        perror(RED "tokenize_input() : memory allocation failed" RESET);
        return -5; 
    }

    // Remove trailing '/' or space if present
    if (len > 0 && (input_copy[len - 1] == '/' || input_copy[len - 1] == ' ')) {
        input_copy[len - 1] = '\0';
    }

    int token_count = 0;
    char *token = strtok(input_copy, "/ "); // Multiple delimiters: '/' and space
    while (token) {
        // Reallocate memory for the result array if needed
        if (token_count >= *allocated_size) {
            *allocated_size *= 2; // Double the allocated size
            char **new_result = realloc(*result, *allocated_size * sizeof(char *));
            if (!new_result) {
                perror(RED "tokenize_input() : memory reallocation failed" RESET);
                free(input_copy);
                return -6; // Error code for memory reallocation failure
            }
            result = new_result;
        }

        // Allocate memory for the token
        result[token_count] = strdup(token);
        // if (!(*result)[token_count]) {
        //     perror(RED "tokenize_input() : memory allocation failed for token" RESET);
        //     free(input_copy);
        //     return -7; // Error code for token memory allocation failure
        // }

        token_count++;
        token = strtok(NULL, "/ ");
    }

    free(input_copy); // Free the input copy after tokenization

    if (token_count == 0) {
        perror(RED "tokenize_input() : no tokens found in input" RESET);
        return -8; // Error code for no tokens
    }

    return token_count; // Return the number of tokens on success
}
