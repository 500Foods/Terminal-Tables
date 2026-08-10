/*
 * tables_datatypes.h - Header file for data type handling in the tables utility
 * Defines structures and function prototypes for validating and formatting data types.
 *
 * This header defines:
 *   - DataTypeHandler: A dispatch table mapping data types to validate/format functions
 *   - Function prototypes for all data type validation and formatting functions
 *   - Helper function for comma-separated number formatting
 */

#ifndef TABLES_DATATYPES_H
#define TABLES_DATATYPES_H

#include "tables_config.h"

/* Structure to hold data type handler information
 * Each data type (text, int, num, float, kcpu, kmem) has a handler
 * that provides validation and formatting functions
 */
typedef struct {
    const char *name;               /* Data type name for display/debugging */
    int (*validate)(const char *value); /* Validation function: returns 1 if valid, 0 otherwise */
    char *(*format)(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification); /* Formatting function */
    const char *summary_types;      /* Supported summary types as space-separated string */
} DataTypeHandler;

/* Function prototypes
 * Validation functions: return 1 if value is valid for the type, 0 otherwise
 */
int validate_text(const char *value);
/* Format functions: return a newly allocated formatted string */
char *format_text(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_number(const char *value);
char *format_number(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
char *format_num(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_kcpu(const char *value);
char *format_kcpu(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_kmem(const char *value);
char *format_kmem(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);

/* Utility functions */
DataTypeHandler *get_data_type_handler(DataType type);
char *format_display_value(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
char *format_display_value_with_precision(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification, int max_decimal_places);
char *format_with_commas(const char *num_str);

#endif /* TABLES_DATATYPES_H */
