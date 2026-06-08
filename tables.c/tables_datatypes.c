/*
 * tables_datatypes.c - Implementation of data type handling for the tables utility
 * Provides validation and formatting functions for different data types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "tables_datatypes.h"

/*
 * Helper function to duplicate a string, returning NULL if input is NULL
 * Note: Currently unused but kept for future extensibility
 */
static char *strdup_safe(const char *str) __attribute__((unused));
static char *strdup_safe(const char *str) {
    if (str == NULL) return NULL;
    char *dup = strdup(str);
    if (dup == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for string duplication\n");
        return NULL;
    }
    return dup;
}

/*
 * Helper function to format a number with commas as thousands separators
 */
char *format_with_commas(const char *num_str) {
    if (num_str == NULL || strlen(num_str) == 0) return strdup("");
    
    // Find decimal point if it exists
    char *decimal_point = strchr(num_str, '.');
    int integer_len;
    char *decimal_part = NULL;
    
    if (decimal_point) {
        integer_len = decimal_point - num_str;
        decimal_part = decimal_point; // Include the decimal point and everything after
    } else {
        integer_len = strlen(num_str);
    }
    
    // Calculate space needed for commas in integer part only
    int comma_count = integer_len > 3 ? (integer_len - 1) / 3 : 0;
    int decimal_len = decimal_part ? strlen(decimal_part) : 0;
    int new_len = integer_len + comma_count + decimal_len;
    
    char *result = malloc(new_len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for number formatting\n");
        return strdup("");
    }
    
    // Process integer part (add commas from right to left)
    int src_idx = integer_len - 1;
    int dst_idx = integer_len + comma_count - 1;
    int count = 0;
    
    while (src_idx >= 0) {
        result[dst_idx--] = num_str[src_idx--];
        count++;
        if (count == 3 && src_idx >= 0) {
            result[dst_idx--] = ',';
            count = 0;
        }
    }
    
    // Add decimal part if it exists
    if (decimal_part) {
        strcpy(result + integer_len + comma_count, decimal_part);
    }
    
    result[new_len] = '\0';
    return result;
}

/*
 * Validation function for text data type
 */
int validate_text(const char *value) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    return 1;
}

/*
 * Formatting function for text data type
 */
char *format_text(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    if (value == NULL || strcmp(value, "null") == 0 || strlen(value) == 0) {
        return strdup("");
    }
    // Suppress unused parameter warnings; these will be used in future implementations
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    
    if (string_limit > 0 && strlen(value) > (size_t)string_limit) {
        if (wrap_mode == WRAP_WRAP && wrap_char != NULL && strlen(wrap_char) > 0) {
            // TODO: Implement wrapping with custom character
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            strncpy(result, value, string_limit);
            result[string_limit] = '\0';
            return result;
        } else if (wrap_mode == WRAP_WRAP) {
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            strncpy(result, value, string_limit);
            result[string_limit] = '\0';
            return result;
        } else {
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            if (justification == JUSTIFY_RIGHT) {
                strncpy(result, value + strlen(value) - string_limit, string_limit);
            } else if (justification == JUSTIFY_CENTER) {
                int start = (strlen(value) - string_limit) / 2;
                strncpy(result, value + start, string_limit);
            } else {
                strncpy(result, value, string_limit);
            }
            result[string_limit] = '\0';
            return result;
        }
    }
    
    return strdup(value);
}

/*
 * Validation function for number data types (int, num, float)
 */
int validate_number(const char *value) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    if (strcmp(value, "0") == 0) {
        return 1;
    }
    
    // Check if the value matches a number pattern (integer or decimal)
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]+(\\.[0-9]+)?$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for number validation\n");
        return 0;
    }
    
    reti = regexec(&regex, value, 0, NULL, 0);
    regfree(&regex);
    if (!reti) {
        return 1;
    }
    return 0;
}

/*
 * Formatting function for number data type (int, float)
 */
char *format_number(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    if (value == NULL || strcmp(value, "null") == 0 || strcmp(value, "0") == 0) {
        return strdup("");
    }
    // Suppress unused parameter warnings; these will be used in future implementations
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    
    if (format != NULL && strlen(format) > 0) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, atof(value));
        return strdup(buffer);
    }
    
    // Apply thousands separators to all numbers
    return format_with_commas(value);
}

/*
 * Formatting function for num data type (numbers with thousands separators)
 */
char *format_num(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    if (value == NULL || strcmp(value, "null") == 0 || strcmp(value, "0") == 0) {
        return strdup("");
    }
    // Suppress unused parameter warnings; these will be used in future implementations
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    
    if (format != NULL && strlen(format) > 0) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, atof(value));
        return strdup(buffer);
    }
    
    return format_with_commas(value);
}

/*
 * Validation function for kcpu data type (Kubernetes CPU values)
 */
int validate_kcpu(const char *value) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0) {
        return 1;
    }
    
    // Check for millicores format (e.g., 100m)
    regex_t regex_m;
    int reti = regcomp(&regex_m, "^[0-9]+m$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for kcpu validation (m)\n");
        return 0;
    }
    reti = regexec(&regex_m, value, 0, NULL, 0);
    regfree(&regex_m);
    if (!reti) {
        return 1;
    }
    
    // Check for numeric value (cores)
    return validate_number(value);
}

/*
 * Formatting function for kcpu data type
 */
char *format_kcpu(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return strdup("");
    }
    if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0) {
        return strdup("0m");
    }
    // Suppress unused parameter warnings; these will be used in future implementations
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    
    if (strstr(value, "m") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kcpu\n");
            return strdup("");
        }
        num_part[strlen(num_part) - 1] = '\0'; // Remove 'm'
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kcpu\n");
            return strdup("");
        }
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kcpu\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%sm", formatted);
        free(formatted);
        return result;
    } else if (validate_number(value)) {
        double cores = atof(value);
        long millicores = (long)(cores * 1000);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%ld", millicores);
        char *formatted = format_with_commas(buffer);
        if (formatted == NULL) return strdup("");
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kcpu\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%sm", formatted);
        free(formatted);
        return result;
    }
    
    return strdup(value);
}

/*
 * Validation function for kmem data type (Kubernetes memory values)
 */
int validate_kmem(const char *value) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    if (strcmp(value, "0") == 0) {
        return 1;
    }
    
    // Check for memory formats (e.g., 128M, 1G, 512Ki)
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]+[KMG]$|^[0-9]+(Mi|Gi|Ki)$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for kmem validation\n");
        return 0;
    }
    reti = regexec(&regex, value, 0, NULL, 0);
    regfree(&regex);
    if (!reti) {
        return 1;
    }
    return 0;
}

/*
 * Formatting function for kmem data type
 */
char *format_kmem(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    if (value == NULL || strcmp(value, "null") == 0) {
        return strdup("");
    }
    // Suppress unused parameter warnings; these will be used in future implementations
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    if (strstr(value, "0M") != NULL || strstr(value, "0G") != NULL || strstr(value, "0K") != NULL ||
        strstr(value, "0Mi") != NULL || strstr(value, "0Gi") != NULL || strstr(value, "0Ki") != NULL) {
        return strdup("0M");
    }
    
    if (strstr(value, "Mi") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        num_part[strlen(num_part) - 2] = '\0'; // Remove 'Mi'
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kmem\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%sM", formatted);
        free(formatted);
        return result;
    } else if (strstr(value, "Gi") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        num_part[strlen(num_part) - 2] = '\0'; // Remove 'Gi'
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kmem\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%sG", formatted);
        free(formatted);
        return result;
    } else if (strstr(value, "Ki") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        num_part[strlen(num_part) - 2] = '\0'; // Remove 'Ki'
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kmem\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%sK", formatted);
        free(formatted);
        return result;
    } else if (strstr(value, "M") != NULL || strstr(value, "G") != NULL || strstr(value, "K") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        char unit = num_part[strlen(num_part) - 1];
        num_part[strlen(num_part) - 1] = '\0'; // Remove unit
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        char *result = malloc(strlen(formatted) + 2);
        if (result == NULL) {
            free(formatted);
            fprintf(stderr, "Error: Memory allocation failed in format_kmem\n");
            return strdup("");
        }
        snprintf(result, strlen(formatted) + 2, "%s%c", formatted, unit);
        free(formatted);
        return result;
    }
    
    return strdup(value);
}

/* Data type handlers array */
static DataTypeHandler handlers[] = {
    {"text", validate_text, format_text, "count unique"},
    {"int", validate_number, format_number, "sum min max avg count unique"},
    {"num", validate_number, format_num, "sum min max avg count unique"},
    {"float", validate_number, format_number, "sum min max avg count unique"},
    {"kcpu", validate_kcpu, format_kcpu, "sum min max avg count unique"},
    {"kmem", validate_kmem, format_kmem, "sum min max avg count unique"}
};

/*
 * Get the data type handler for a given data type
 */
DataTypeHandler *get_data_type_handler(DataType type) {
    if (type >= DATA_TEXT && type <= DATA_KMEM) {
        return &handlers[type];
    }
    return &handlers[DATA_TEXT]; // Default to text
}

/*
 * Format a value for display, considering null and zero value display options
 */
char *format_display_value(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    DataTypeHandler *handler = get_data_type_handler(data_type);
    int is_valid = handler->validate(value);
    char *display_value = NULL;
    
    if (!is_valid || value == NULL || strcmp(value, "null") == 0) {
        switch (null_value) {
            case VALUE_ZERO:
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                display_value = strdup("Missing");
                break;
            default:
                display_value = strdup("");
        }
    } else if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0 || strcmp(value, "0M") == 0 || strcmp(value, "0G") == 0 || strcmp(value, "0K") == 0) {
        switch (zero_value) {
            case VALUE_ZERO:
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                display_value = strdup("Missing");
                break;
            default:
                display_value = strdup("");
        }
    } else {
        display_value = handler->format(value, format, string_limit, wrap_mode, wrap_char, justification);
    }
    
    return display_value;
}

/*
 * Format a value for display with decimal precision, considering null and zero value display options
 */
char *format_display_value_with_precision(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification, int max_decimal_places) {
    DataTypeHandler *handler = get_data_type_handler(data_type);
    int is_valid = handler->validate(value);
    char *display_value = NULL;
    
    if (!is_valid || value == NULL || strcmp(value, "null") == 0) {
        switch (null_value) {
            case VALUE_ZERO:
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                display_value = strdup("Missing");
                break;
            default:
                display_value = strdup("");
        }
    } else if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0 || strcmp(value, "0M") == 0 || strcmp(value, "0G") == 0 || strcmp(value, "0K") == 0) {
        switch (zero_value) {
            case VALUE_ZERO:
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                display_value = strdup("Missing");
                break;
            default:
                display_value = strdup("");
        }
    } else {
        // For float data type, format with consistent decimal places
        if (data_type == DATA_FLOAT && max_decimal_places > 0) {
            char format_str[16];
            snprintf(format_str, sizeof(format_str), "%%.%df", max_decimal_places);
            char buffer[256];
            snprintf(buffer, sizeof(buffer), format_str, atof(value));
            // Apply thousands separators to the formatted float
            display_value = format_with_commas(buffer);
        } else {
            display_value = handler->format(value, format, string_limit, wrap_mode, wrap_char, justification);
        }
    }
    
    return display_value;
}
