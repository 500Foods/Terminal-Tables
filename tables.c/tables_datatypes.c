/*
 * tables_datatypes.c - Implementation of data type handling for the tables utility
 * Provides validation and formatting functions for different data types.
 *
 * Supported data types:
 *   - text: Plain text values (no validation, no formatting)
 *   - int:   Integer numbers, formatted with comma separators
 *   - num:   Numeric values, formatted with comma separators
 *   - float: Floating point numbers, formatted with consistent decimal places
 *   - kcpu:  Kubernetes CPU values (cores or millicores like 100m)
 *   - kmem:  Kubernetes memory values (with units like Mi, Gi, Ki, M, G, K)
 *
 * Each data type has a validate function and a format function,
 * registered in the handlers array for dispatch by the format_display_value functions.
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
    /* Return NULL immediately if the input is NULL */
    if (str == NULL) return NULL;
    /* Attempt to duplicate the string using POSIX strdup */
    char *dup = strdup(str);
    /* Report error if memory allocation fails */
    if (dup == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for string duplication\n");
        return NULL;
    }
    return dup;
}

/*
 * Helper function to format a number with commas as thousands separators
 * Processes the integer part of the number, inserting commas every 3 digits from the right
 * Preserves the decimal part (if any) unchanged
 * Returns a newly allocated string that the caller must free
 */
char *format_with_commas(const char *num_str) {
    /* Handle NULL or empty input */
    if (num_str == NULL || strlen(num_str) == 0) return strdup("");

    /* Find decimal point if it exists to separate integer and fractional parts */
    char *decimal_point = strchr(num_str, '.');
    /* Length of just the integer portion */
    int integer_len;
    /* Pointer to the decimal portion (including the '.' character) */
    char *decimal_part = NULL;

    if (decimal_point) {
        /* Integer part length is everything before the decimal point */
        integer_len = decimal_point - num_str;
        /* decimal_part includes the '.' and everything after */
        decimal_part = decimal_point;
    } else {
        /* No decimal point - entire string is integer part */
        integer_len = strlen(num_str);
    }

    /* Calculate how many commas will be inserted in the integer part */
    int comma_count = integer_len > 3 ? (integer_len - 1) / 3 : 0;
    /* Length of the decimal portion (0 if none) */
    int decimal_len = decimal_part ? strlen(decimal_part) : 0;
    /* Total length needed for the result (integer + commas + decimal) */
    int new_len = integer_len + comma_count + decimal_len;

    /* Allocate memory for the result string (plus null terminator) */
    char *result = malloc(new_len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for number formatting\n");
        return strdup("");
    }

    /* Process integer part (add commas from right to left) */
    /* Start from the rightmost digit of the integer part */
    int src_idx = integer_len - 1;
    /* Destination index starts from the right of the integer+commas portion */
    int dst_idx = integer_len + comma_count - 1;
    /* Counter for digits placed since last comma */
    int count = 0;

    /* Copy digits from right to left, inserting commas every 3 digits */
    while (src_idx >= 0) {
        result[dst_idx--] = num_str[src_idx--];
        count++;
        /* Insert comma after every 3 digits (but not at the very beginning) */
        if (count == 3 && src_idx >= 0) {
            result[dst_idx--] = ',';
            count = 0;
        }
    }

    /* Append decimal part if it exists (preserved as-is) */
    if (decimal_part) {
        strcpy(result + integer_len + comma_count, decimal_part);
    }

    /* Null-terminate the result string */
    result[new_len] = '\0';
    return result;
}

/*
 * Validation function for text data type
 * Returns 1 if the value is valid text, 0 for NULL or "null" values
 */
int validate_text(const char *value) {
    /* NULL or "null" string means the value is invalid (blank) */
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    /* Any other string is valid text */
    return 1;
}

/*
 * Formatting function for text data type
 * Handles string limiting and clipping for text values with no transformation
 */
char *format_text(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Return empty string for NULL, "null", or empty values */
    if (value == NULL || strcmp(value, "null") == 0 || strlen(value) == 0) {
        return strdup("");
    }
    /* Suppress unused parameter warnings; these will be used in future implementations */
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;

    /* If string_limit is set and value exceeds it, truncate or clip */
    if (string_limit > 0 && strlen(value) > (size_t)string_limit) {
        /* Handle wrap mode: currently just truncates to string_limit */
        if (wrap_mode == WRAP_WRAP && wrap_char != NULL && strlen(wrap_char) > 0) {
            /* TODO: Implement wrapping with custom character */
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            /* Copy only string_limit characters */
            strncpy(result, value, string_limit);
            result[string_limit] = '\0';
            return result;
        } else if (wrap_mode == WRAP_WRAP) {
            /* Wrap mode without custom character: truncate */
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            strncpy(result, value, string_limit);
            result[string_limit] = '\0';
            return result;
        } else {
            /* Clip mode: truncate based on justification */
            char *result = malloc(string_limit + 1);
            if (result == NULL) return strdup("");
            if (justification == JUSTIFY_RIGHT) {
                /* Right-justified: keep the last string_limit characters */
                strncpy(result, value + strlen(value) - string_limit, string_limit);
            } else if (justification == JUSTIFY_CENTER) {
                /* Center-justified: keep the middle portion */
                int start = (strlen(value) - string_limit) / 2;
                strncpy(result, value + start, string_limit);
            } else {
                /* Left-justified: keep the first string_limit characters */
                strncpy(result, value, string_limit);
            }
            result[string_limit] = '\0';
            return result;
        }
    }

    /* No truncation needed - return value as-is */
    return strdup(value);
}

/*
 * Validation function for number data types (int, num, float)
 * Uses regex to validate that the value is a proper numeric string
 * Returns 1 if valid, 0 otherwise
 */
int validate_number(const char *value) {
    /* NULL or "null" is invalid */
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    /* "0" is valid (special case to avoid regex for simple zero) */
    if (strcmp(value, "0") == 0) {
        return 1;
    }

    /* Compile regex for number pattern: optional sign, digits, optional decimal */
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]+(\\.[0-9]+)?$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for number validation\n");
        return 0;
    }

    /* Test the value against the compiled regex */
    reti = regexec(&regex, value, 0, NULL, 0);
    /* Free the compiled regex */
    regfree(&regex);
    if (!reti) {
        /* Match: value is a valid number */
        return 1;
    }
    /* No match: value is not a valid number */
    return 0;
}

/*
 * Formatting function for number data type (int, float)
 * Applies custom format string if provided, otherwise returns raw value
 */
char *format_number(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Return empty string for NULL, "null", or zero values */
    if (value == NULL || strcmp(value, "null") == 0 || strcmp(value, "0") == 0) {
        return strdup("");
    }
    /* Suppress unused parameter warnings; these will be used in future implementations */
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;

    /* Apply custom format string if provided (e.g., "%.2f") */
    if (format != NULL && strlen(format) > 0) {
        char buffer[256];
        /* Use snprintf with the user-provided format and the numeric value */
        snprintf(buffer, sizeof(buffer), format, atof(value));
        return strdup(buffer);
    }

    /* Default: return raw value without comma separators */
    return strdup(value);
}

/*
 * Formatting function for num data type (numbers with thousands separators)
 * Similar to format_number but always applies comma formatting
 */
char *format_num(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Return empty string for NULL, "null", or zero values */
    if (value == NULL || strcmp(value, "null") == 0 || strcmp(value, "0") == 0) {
        return strdup("");
    }
    /* Suppress unused parameter warnings; these will be used in future implementations */
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;

    /* Apply custom format string if provided */
    if (format != NULL && strlen(format) > 0) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, atof(value));
        return strdup(buffer);
    }

    /* Apply thousands separators for readability */
    return format_with_commas(value);
}

/*
 * Validation function for kcpu data type (Kubernetes CPU values)
 * Accepts values like "100m" (millicores) or plain numbers (cores)
 * Returns 1 if valid, 0 otherwise
 */
int validate_kcpu(const char *value) {
    /* NULL or "null" is invalid */
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    /* "0" and "0m" are valid zero values */
    if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0) {
        return 1;
    }

    /* Check for millicores format (e.g., 100m) using regex */
    regex_t regex_m;
    int reti = regcomp(&regex_m, "^[0-9]+m$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for kcpu validation (m)\n");
        return 0;
    }
    /* Test against millicores regex */
    reti = regexec(&regex_m, value, 0, NULL, 0);
    regfree(&regex_m);
    if (!reti) {
        /* Matches millicores format */
        return 1;
    }

    /* Fall back to plain number validation for core values */
    return validate_number(value);
}

/*
 * Formatting function for kcpu data type
 * Converts CPU values to millicores format with comma separators
 * Handles both "100m" (millicores) and "1.5" (cores) formats
 */
char *format_kcpu(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Return empty string for NULL or "null" values */
    if (value == NULL || strcmp(value, "null") == 0) {
        return strdup("");
    }
    /* Zero values display as "0m" */
    if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0) {
        return strdup("0m");
    }
    /* Suppress unused parameter warnings; these will be used in future implementations */
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;

    /* Handle millicores values (e.g., "100m") */
    if (strstr(value, "m") != NULL) {
        /* Duplicate value to safely modify */
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kcpu\n");
            return strdup("");
        }
        /* Remove trailing 'm' suffix */
        num_part[strlen(num_part) - 1] = '\0';
        /* Apply comma formatting to the numeric portion */
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kcpu\n");
            return strdup("");
        }
        /* Build result: formatted number + "m" suffix */
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
        /* Handle core values (e.g., "1.5") by converting to millicores */
        double cores = atof(value);
        /* Convert cores to millicores (1 core = 1000 millicores) */
        long millicores = (long)(cores * 1000);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%ld", millicores);
        /* Apply comma formatting */
        char *formatted = format_with_commas(buffer);
        if (formatted == NULL) return strdup("");
        /* Build result with "m" suffix */
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

    /* Return value as-is if it doesn't match known formats */
    return strdup(value);
}

/*
 * Validation function for kmem data type (Kubernetes memory values)
 * Accepts values like "128M", "1G", "512Ki", "1Gi"
 * Returns 1 if valid, 0 otherwise
 */
int validate_kmem(const char *value) {
    /* NULL or "null" is invalid */
    if (value == NULL || strcmp(value, "null") == 0) {
        return 0;
    }
    /* "0" is valid (no unit suffix for zero is acceptable) */
    if (strcmp(value, "0") == 0) {
        return 1;
    }

    /* Compile regex for memory formats: plain units (M/G/K) or binary units (Mi/Gi/Ki) */
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]+[KMG]$|^[0-9]+(Mi|Gi|Ki)$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Error: Could not compile regex for kmem validation\n");
        return 0;
    }
    /* Test value against the compiled regex */
    reti = regexec(&regex, value, 0, NULL, 0);
    regfree(&regex);
    if (!reti) {
        /* Matches a valid memory format */
        return 1;
    }
    /* No match: invalid memory format */
    return 0;
}

/*
 * Formatting function for kmem data type
 * Normalizes Kubernetes memory values to display with comma formatting
 * Handles unit suffixes: Mi, M, Gi, G, Ki, K
 */
char *format_kmem(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Return empty string for NULL or "null" values */
    if (value == NULL || strcmp(value, "null") == 0) {
        return strdup("");
    }
    /* Suppress unused parameter warnings; these will be used in future implementations */
    (void)format;
    (void)string_limit;
    (void)wrap_mode;
    (void)wrap_char;
    (void)justification;
    /* Zero-value memory displays as "0M" */
    if (strstr(value, "0M") != NULL || strstr(value, "0G") != NULL || strstr(value, "0K") != NULL ||
        strstr(value, "0Mi") != NULL || strstr(value, "0Gi") != NULL || strstr(value, "0Ki") != NULL) {
        return strdup("0M");
    }

    /* Handle Mi (mebibytes) suffix */
    if (strstr(value, "Mi") != NULL) {
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        /* Remove the 2-character "Mi" suffix */
        num_part[strlen(num_part) - 2] = '\0';
        /* Apply comma formatting */
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        /* Build result with "M" suffix */
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
        /* Handle Gi (gibibytes) suffix */
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        /* Remove the 2-character "Gi" suffix */
        num_part[strlen(num_part) - 2] = '\0';
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        /* Build result with "G" suffix */
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
        /* Handle Ki (kibibytes) suffix */
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        /* Remove the 2-character "Ki" suffix */
        num_part[strlen(num_part) - 2] = '\0';
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        /* Build result with "K" suffix */
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
        /* Handle single-character unit suffixes (M, G, K) */
        char *num_part = strdup(value);
        if (num_part == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for num_part in format_kmem\n");
            return strdup("");
        }
        /* Extract the unit character from the end of the string */
        char unit = num_part[strlen(num_part) - 1];
        /* Remove the single-character unit suffix */
        num_part[strlen(num_part) - 1] = '\0';
        char *formatted = format_with_commas(num_part);
        free(num_part);
        if (formatted == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted in format_kmem\n");
            return strdup("");
        }
        /* Build result with the original unit character preserved */
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

    /* Return value as-is if it doesn't match known formats */
    return strdup(value);
}

/*
 * Data type handlers array
 * Maps each DataType enum value to its validation and formatting functions.
 * Order must match the DataType enum in tables_config.h:
 *   DATA_TEXT=0, DATA_INT=1, DATA_NUM=2, DATA_FLOAT=3, DATA_KCPU=4, DATA_KMEM=5
 */
static DataTypeHandler handlers[] = {
    /* Text type: basic validation and formatting */
    {"text", validate_text, format_text, "count unique"},
    /* Integer type: number validation, comma formatting, full summary support */
    {"int", validate_number, format_number, "sum min max avg count unique"},
    /* Numeric type: number validation, comma formatting, full summary support */
    {"num", validate_number, format_num, "sum min max avg count unique"},
    /* Float type: number validation, comma formatting with decimals, full summary support */
    {"float", validate_number, format_number, "sum min max avg count unique"},
    /* Kubernetes CPU type: millicores validation, "m" suffix formatting, full summary support */
    {"kcpu", validate_kcpu, format_kcpu, "sum min max avg count unique"},
    /* Kubernetes memory type: memory unit validation, unit-aware formatting, full summary support */
    {"kmem", validate_kmem, format_kmem, "sum min max avg count unique"}
};

/*
 * Get the data type handler for a given data type
 * Returns a pointer to the DataTypeHandler struct for the specified type,
 * or the text handler as a default for out-of-range values
 */
DataTypeHandler *get_data_type_handler(DataType type) {
    /* Ensure the type is within the valid range of the handlers array */
    if (type >= DATA_TEXT && type <= DATA_KMEM) {
        return &handlers[type];
    }
    /* Default to text handler for unknown types */
    return &handlers[DATA_TEXT];
}

/*
 * Format a value for display, considering null and zero value display options
 * This function dispatches to the appropriate data type handler for formatting,
 * and handles special cases for null/zero values based on configuration.
 *
 * Parameters:
 *   value:        The raw string value to format
 *   null_value:   How to display NULL values (blank, zero, or "Missing")
 *   zero_value:   How to display zero values (blank, zero, or "Missing")
 *   data_type:    The data type enum determining which formatter to use
 *   format:       Custom format string (e.g., "%.2f")
 *   string_limit: Maximum string length before truncation
 *   wrap_mode:    How to handle text exceeding string_limit
 *   wrap_char:    Character for delimiter-based wrapping
 *   justification: Text alignment for clipped content
 *
 * Returns a newly allocated string that the caller must free
 */
char *format_display_value(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification) {
    /* Get the handler for this data type */
    DataTypeHandler *handler = get_data_type_handler(data_type);
    /* Validate the value using the handler's validation function */
    int is_valid = handler->validate(value);
    /* Buffer for the formatted display value */
    char *display_value = NULL;

    /* Handle null/invalid values based on null_value display option */
    if (!is_valid || value == NULL || strcmp(value, "null") == 0) {
        switch (null_value) {
            case VALUE_ZERO:
                /* Display "0" for null values */
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                /* Display "Missing" text for null values */
                display_value = strdup("Missing");
                break;
            default:
                /* Display blank for null values */
                display_value = strdup("");
        }
    } else if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0 || strcmp(value, "0M") == 0 || strcmp(value, "0G") == 0 || strcmp(value, "0K") == 0) {
        /* Handle zero values based on zero_value display option */
        switch (zero_value) {
            case VALUE_ZERO:
                /* Display "0" for zero values */
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                /* Display "Missing" text for zero values */
                display_value = strdup("Missing");
                break;
            default:
                /* Display blank for zero values */
                display_value = strdup("");
        }
    } else {
        /* Valid, non-null, non-zero value: use the type-specific formatter */
        display_value = handler->format(value, format, string_limit, wrap_mode, wrap_char, justification);
    }

    return display_value;
}

/*
 * Format a value for display with decimal precision, considering null and zero value display options
 * Similar to format_display_value but with an additional max_decimal_places parameter
 * for float types to ensure consistent decimal place formatting across all values in a column.
 *
 * The max_decimal_places parameter ensures that all float values in a column are formatted
 * with the same number of decimal places (determined by the column's maximum).
 */
char *format_display_value_with_precision(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification, int max_decimal_places) {
    /* Get the handler for this data type */
    DataTypeHandler *handler = get_data_type_handler(data_type);
    /* Validate the value */
    int is_valid = handler->validate(value);
    /* Buffer for the formatted display value */
    char *display_value = NULL;

    /* Handle null/invalid values based on null_value display option */
    if (!is_valid || value == NULL || strcmp(value, "null") == 0) {
        switch (null_value) {
            case VALUE_ZERO:
                /* Display "0" for null values */
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                /* Display "Missing" text for null values */
                display_value = strdup("Missing");
                break;
            default:
                /* Display blank for null values */
                display_value = strdup("");
        }
    } else if (strcmp(value, "0") == 0 || strcmp(value, "0m") == 0 || strcmp(value, "0M") == 0 || strcmp(value, "0G") == 0 || strcmp(value, "0K") == 0) {
        /* Handle zero values based on zero_value display option */
        switch (zero_value) {
            case VALUE_ZERO:
                /* Display "0" for zero values */
                display_value = strdup("0");
                break;
            case VALUE_MISSING:
                /* Display "Missing" text for zero values */
                display_value = strdup("Missing");
                break;
            default:
                /* Display blank for zero values */
                display_value = strdup("");
        }
    } else {
        /* For float data type with specified decimal precision, use fixed decimal formatting */
        if (data_type == DATA_FLOAT && max_decimal_places > 0) {
            /* Build format string for the specified number of decimal places */
            char format_str[16];
            snprintf(format_str, sizeof(format_str), "%%.%df", max_decimal_places);
            char buffer[256];
            /* Format the value with the specified decimal precision */
            snprintf(buffer, sizeof(buffer), format_str, atof(value));
            /* Apply thousands separators to the formatted float string */
            display_value = format_with_commas(buffer);
        } else {
            /* Use the type-specific formatter for non-float or unspecified precision */
            display_value = handler->format(value, format, string_limit, wrap_mode, wrap_char, justification);
        }
    }

    return display_value;
}
