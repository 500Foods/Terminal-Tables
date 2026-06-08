/*
 * tables_datatypes.h - Header file for data type handling in the tables utility
 * Defines structures and function prototypes for validating and formatting data types.
 */

#ifndef TABLES_DATATYPES_H
#define TABLES_DATATYPES_H

#include "tables_config.h"

/* Structure to hold data type handler information */
typedef struct {
    const char *name;               /* Data type name */
    int (*validate)(const char *value); /* Validation function */
    char *(*format)(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification); /* Formatting function */
    const char *summary_types;      /* Supported summary types as space-separated string */
} DataTypeHandler;

/* Function prototypes */
int validate_text(const char *value);
char *format_text(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_number(const char *value);
char *format_number(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
char *format_num(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_kcpu(const char *value);
char *format_kcpu(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
int validate_kmem(const char *value);
char *format_kmem(const char *value, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
DataTypeHandler *get_data_type_handler(DataType type);
char *format_display_value(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification);
char *format_display_value_with_precision(const char *value, ValueDisplay null_value, ValueDisplay zero_value, DataType data_type, const char *format, int string_limit, int wrap_mode, const char *wrap_char, int justification, int max_decimal_places);
char *format_with_commas(const char *num_str);

#endif /* TABLES_DATATYPES_H */
