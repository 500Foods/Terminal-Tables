/*
 * tables_config.c - Implementation of configuration parsing for the tables utility
 * Parses layout JSON files and manages configuration structures.
 *
 * This file handles:
 *   - Reading and parsing layout JSON files
 *   - Converting JSON fields to typed config structures
 *   - Memory management for the TableConfig structure
 *   - Validation of input file accessibility
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>
#include "tables_config.h"

/*
 * Helper function to duplicate a string, returning NULL if input is NULL
 * Wraps strdup() with a NULL check and error reporting on allocation failure
 */
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
 * Helper function to parse justification string to enum
 * Returns JUSTIFY_LEFT as the default for unrecognized or NULL values
 */
static Justification parse_justification(const char *str) {
    /* Default to left alignment for NULL input */
    if (str == NULL) return JUSTIFY_LEFT;
    /* Case-insensitive comparison for "right" */
    if (strcasecmp(str, "right") == 0) return JUSTIFY_RIGHT;
    /* Case-insensitive comparison for "center" */
    if (strcasecmp(str, "center") == 0) return JUSTIFY_CENTER;
    /* Default to left alignment for any other string */
    return JUSTIFY_LEFT;
}

/*
 * Helper function to parse data type string to enum
 * Returns DATA_TEXT as the default for unrecognized or NULL values
 */
static DataType parse_data_type(const char *str) {
    /* Default to text type for NULL input */
    if (str == NULL) return DATA_TEXT;
    /* Integer data type: whole numbers */
    if (strcasecmp(str, "int") == 0) return DATA_INT;
    /* Numeric data type: numbers with thousands separators */
    if (strcasecmp(str, "num") == 0) return DATA_NUM;
    /* Float data type: floating point numbers */
    if (strcasecmp(str, "float") == 0) return DATA_FLOAT;
    /* Kubernetes CPU data type: cores or millicores (e.g., 100m) */
    if (strcasecmp(str, "kcpu") == 0) return DATA_KCPU;
    /* Kubernetes memory data type: with units like Mi, Gi, Ki */
    if (strcasecmp(str, "kmem") == 0) return DATA_KMEM;
    /* Default to text for unrecognized types */
    return DATA_TEXT;
}

/*
 * Helper function to parse value display string to enum
 * Controls how null and zero values are rendered in table cells
 */
static ValueDisplay parse_value_display(const char *str) {
    /* Default: show blank for null values */
    if (str == NULL) return VALUE_BLANK;
    /* "0" means display zero in place of null */
    if (strcasecmp(str, "0") == 0) return VALUE_ZERO;
    /* "missing" means display "Missing" text */
    if (strcasecmp(str, "missing") == 0) return VALUE_MISSING;
    /* Default: show blank */
    return VALUE_BLANK;
}

/*
 * Helper function to parse summary type string to enum
 * Controls what summary calculation to perform for a column
 */
static SummaryType parse_summary_type(const char *str) {
    /* Default: no summary calculation */
    if (str == NULL) return SUMMARY_NONE;
    /* Sum of all values in the column */
    if (strcasecmp(str, "sum") == 0) return SUMMARY_SUM;
    /* Minimum value in the column */
    if (strcasecmp(str, "min") == 0) return SUMMARY_MIN;
    /* Maximum value in the column */
    if (strcasecmp(str, "max") == 0) return SUMMARY_MAX;
    /* Average of all values in the column */
    if (strcasecmp(str, "avg") == 0) return SUMMARY_AVG;
    /* Count of non-null values */
    if (strcasecmp(str, "count") == 0) return SUMMARY_COUNT;
    /* Count of unique values */
    if (strcasecmp(str, "unique") == 0) return SUMMARY_UNIQUE;
    /* Count of blank values */
    if (strcasecmp(str, "blanks") == 0) return SUMMARY_BLANKS;
    /* Count of non-blank values */
    if (strcasecmp(str, "nonblanks") == 0) return SUMMARY_NONBLANKS;
    /* Default: no summary */
    return SUMMARY_NONE;
}

/*
 * Helper function to parse wrap mode string to enum
 * Controls how text is handled when it exceeds column width
 */
static WrapMode parse_wrap_mode(const char *str) {
    /* Default: clip text that exceeds width */
    if (str == NULL) return WRAP_CLIP;
    /* "wrap" means wrap long text to multiple lines */
    if (strcasecmp(str, "wrap") == 0) return WRAP_WRAP;
    /* Default: clip */
    return WRAP_CLIP;
}

/*
 * Helper function to parse position string to enum
 * Controls positioning of title and footer elements
 */
static Position parse_position(const char *str) {
    /* Default: no positioning (title extends beyond table) */
    if (str == NULL) return POSITION_NONE;
    /* Left-aligned positioning */
    if (strcasecmp(str, "left") == 0) return POSITION_LEFT;
    /* Right-aligned positioning */
    if (strcasecmp(str, "right") == 0) return POSITION_RIGHT;
    /* Center-aligned positioning */
    if (strcasecmp(str, "center") == 0) return POSITION_CENTER;
    /* Full-width positioning: span entire table width */
    if (strcasecmp(str, "full") == 0) return POSITION_FULL;
    /* Default: no positioning */
    return POSITION_NONE;
}

/*
 * Validate input files exist and are non-empty
 * Returns 0 on success, 1 if either file cannot be opened
 */
int validate_input_files(const char *layout_file, const char *data_file) {
    FILE *fp;

    /* Attempt to open the layout file for reading */
    fp = fopen(layout_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open layout file %s\n", layout_file);
        return 1;
    }
    /* File opened successfully, close it */
    fclose(fp);

    /* Attempt to open the data file for reading */
    fp = fopen(data_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open data file %s\n", data_file);
        return 1;
    }
    /* File opened successfully, close it */
    fclose(fp);

    /* Both files are accessible */
    return 0;
}

/*
 * Parse layout JSON file into TableConfig structure
 * Reads the JSON file, parses all configuration fields, and populates
 * the TableConfig structure with columns, sorts, title, footer, theme, etc.
 * Returns 0 on success, 1 on error
 */
int parse_layout_file(const char *filename, TableConfig *config) {
    /* Jansson library JSON root and error tracking */
    json_t *root;
    json_error_t error;
    /* File handle for reading the layout file */
    FILE *fp;
    /* Buffer for reading file content dynamically */
    char *buffer = NULL;
    /* Current allocated size of the buffer */
    size_t buffer_size = 0;
    /* Total bytes read from the file */
    size_t total_read = 0;
    /* Chunk size for incremental file reading */
    size_t chunk_size = 1024;
    /* Reference to global debug flag */
    extern int debug_mode;

    if (debug_mode) {
        fprintf(stderr, "Debug: Starting to parse layout file %s\n", filename);
    }

    /* Open the layout file for reading */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open layout file %s\n", filename);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Layout file %s opened successfully\n", filename);
    }

    /* Read file content into buffer in chunks for memory efficiency */
    buffer = malloc(chunk_size + 1); /* Extra byte for null terminator */
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for buffer\n");
        fclose(fp);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Allocated initial buffer of size %zu for layout file\n", chunk_size + 1);
    }
    /* Track the current buffer capacity */
    buffer_size = chunk_size;

    /* Read file in a loop until EOF is reached */
    while (1) {
        /* Read a chunk of data into the buffer at the current position */
        size_t bytes_read = fread(buffer + total_read, 1, chunk_size, fp);
        /* Accumulate total bytes read */
        total_read += bytes_read;
        if (debug_mode) {
            fprintf(stderr, "Debug: Read %zu bytes, total read now %zu\n", bytes_read, total_read);
        }
        /* Check if we've reached the end of the file */
        if (bytes_read < chunk_size) {
            if (feof(fp)) {
                if (debug_mode) {
                    fprintf(stderr, "Debug: End of file reached\n");
                }
                break;
            }
            /* Check for read errors */
            if (ferror(fp)) {
                fprintf(stderr, "Error: Reading layout file %s\n", filename);
                free(buffer);
                fclose(fp);
                return 1;
            }
        }
        /* Buffer is full, expand it to accommodate more data */
        buffer_size += chunk_size;
        char *new_buffer = realloc(buffer, buffer_size + 1); /* Extra byte for null terminator */
        if (new_buffer == NULL) {
            fprintf(stderr, "Error: Memory reallocation failed for buffer\n");
            free(buffer);
            fclose(fp);
            return 1;
        }
        if (debug_mode) {
            fprintf(stderr, "Debug: Reallocated buffer to size %zu for layout file\n", buffer_size + 1);
        }
        /* Update buffer pointer to the newly allocated memory */
        buffer = new_buffer;
    }
    /* Close the file now that all data has been read */
    fclose(fp);

    /* Null-terminate the buffer for JSON parsing */
    buffer[total_read] = '\0';
    if (debug_mode) {
        fprintf(stderr, "Debug: Read %zu bytes from layout file, buffer null-terminated\n", total_read);
    }

    /* Parse the JSON buffer using the Jansson library */
    if (debug_mode) {
        fprintf(stderr, "Debug: Starting JSON parsing for layout file\n");
    }
    root = json_loads(buffer, 0, &error);
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON parsing completed, freeing buffer\n");
    }
    /* Free the read buffer as JSON parsing is done */
    free(buffer);
    if (root == NULL) {
        fprintf(stderr, "Error: JSON parsing failed for %s: %s\n", filename, error.text);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON layout parsed successfully from %s\n", filename);
    }

    /* Initialize the config structure to zero before populating fields */
    memset(config, 0, sizeof(TableConfig));

    /* Parse theme name from JSON, defaulting to "Red" */
    json_t *theme_val = json_object_get(root, "theme");
    config->theme_name = strdup_safe(json_string_value(theme_val) ? json_string_value(theme_val) : "Red");
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed theme_name as '%s'\n", config->theme_name ? config->theme_name : "NULL");
    }

    /* Parse title text (optional) */
    json_t *title_val = json_object_get(root, "title");
    config->title = strdup_safe(json_string_value(title_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed title as '%s'\n", config->title ? config->title : "NULL");
    }
    /* Parse title position (left, center, right, full, none) */
    json_t *title_pos_val = json_object_get(root, "title_position");
    config->title_pos = parse_position(json_string_value(title_pos_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed title_position as %d\n", config->title_pos);
    }

    /* Parse footer text (optional) */
    json_t *footer_val = json_object_get(root, "footer");
    config->footer = strdup_safe(json_string_value(footer_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed footer as '%s'\n", config->footer ? config->footer : "NULL");
    }
    /* Parse footer position */
    json_t *footer_pos_val = json_object_get(root, "footer_position");
    config->footer_pos = parse_position(json_string_value(footer_pos_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed footer_position as %d\n", config->footer_pos);
    }

    /* Parse columns array - the core of the table layout */
    json_t *columns_array = json_object_get(root, "columns");
    if (!json_is_array(columns_array) || json_array_size(columns_array) == 0) {
        fprintf(stderr, "Error: No columns defined in layout JSON\n");
        json_decref(root);
        free_table_config(config);
        return 1;
    }

    /* Set column count, with overflow protection */
    config->column_count = json_array_size(columns_array);
    if (config->column_count > MAX_COLUMNS) {
        fprintf(stderr, "Warning: Too many columns, truncating to %d\n", MAX_COLUMNS);
        config->column_count = MAX_COLUMNS;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Configured %d columns for layout\n", config->column_count);
    }

    /* Allocate memory for the column configuration array */
    config->columns = malloc(config->column_count * sizeof(ColumnConfig));
    if (config->columns == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for columns\n");
        json_decref(root);
        free_table_config(config);
        return 1;
    }

    /* Iterate over each column and parse its configuration */
    for (int i = 0; i < config->column_count; i++) {
        json_t *col_obj = json_array_get(columns_array, i);
        if (!json_is_object(col_obj)) continue;

        ColumnConfig *col = &config->columns[i];
        /* Zero-initialize the column config to ensure all fields start clean */
        memset(col, 0, sizeof(ColumnConfig));

        /* Parse column header text - required field */
        json_t *header_val = json_object_get(col_obj, "header");
        col->header = strdup_safe(json_string_value(header_val));
        if (col->header == NULL || strlen(col->header) == 0) {
            fprintf(stderr, "Error: Column %d has no header\n", i);
            json_decref(root);
            free_table_config(config);
            return 1;
        }

        /* Parse JSON key for data lookup */
        json_t *key_val = json_object_get(col_obj, "key");
        const char *key_str = json_string_value(key_val);
        if (key_str == NULL || strlen(key_str) == 0) {
            /* Derive key from header if not explicitly specified */
            char *derived_key = strdup(col->header);
            if (derived_key == NULL) {
                fprintf(stderr, "Error: Memory allocation failed for derived key\n");
                json_decref(root);
                free_table_config(config);
                return 1;
            }
            /* Transform: lowercase and replace non-alphanumeric with underscores */
            for (char *p = derived_key; *p; p++) {
                if (!isalnum(*p)) *p = '_';
                else *p = tolower(*p);
            }
            col->key = derived_key;
        } else {
            /* Use the explicitly specified key */
            col->key = strdup_safe(key_str);
        }

        /* Parse text justification (left, right, center) */
        json_t *justify_val = json_object_get(col_obj, "justification");
        col->justify = parse_justification(json_string_value(justify_val));

        /* Parse data type (text, int, num, float, kcpu, kmem) */
        json_t *datatype_val = json_object_get(col_obj, "datatype");
        col->data_type = parse_data_type(json_string_value(datatype_val));

        /* Parse null value display option */
        json_t *null_val = json_object_get(col_obj, "null_value");
        col->null_val = parse_value_display(json_string_value(null_val));

        /* Parse zero value display option */
        json_t *zero_val = json_object_get(col_obj, "zero_value");
        col->zero_val = parse_value_display(json_string_value(zero_val));

        /* Parse custom format string for value display */
        json_t *format_val = json_object_get(col_obj, "format");
        col->format = strdup_safe(json_string_value(format_val));

        /* Parse summary type for column-level statistical calculations */
        json_t *summary_val = json_object_get(col_obj, "summary");
        col->summary = parse_summary_type(json_string_value(summary_val));

        /* Parse break_on_change flag for visual separators on value change */
        json_t *break_val = json_object_get(col_obj, "break");
        col->break_on_change = json_is_true(break_val);

        /* Parse string_limit for truncating cell content */
        json_t *string_limit_val = json_object_get(col_obj, "string_limit");
        col->string_limit = json_is_number(string_limit_val) ? json_integer_value(string_limit_val) : 0;

        /* Parse wrap mode for handling text that exceeds column width */
        json_t *wrap_mode_val = json_object_get(col_obj, "wrap_mode");
        col->wrap_mode = parse_wrap_mode(json_string_value(wrap_mode_val));

        /* Parse custom wrap character for delimiter-based wrapping */
        json_t *wrap_char_val = json_object_get(col_obj, "wrap_char");
        col->wrap_char = strdup_safe(json_string_value(wrap_char_val));

        /* Parse padding (default is 1 space on each side) */
        json_t *padding_val = json_object_get(col_obj, "padding");
        col->padding = json_is_number(padding_val) ? json_integer_value(padding_val) : DEFAULT_PADDING;

        /* Parse fixed column width (0 means auto-calculate from content) */
        json_t *width_val = json_object_get(col_obj, "width");
        col->width = json_is_number(width_val) ? json_integer_value(width_val) : 0;
        /* Track whether width was explicitly specified by the user */
        col->width_specified = (col->width > 0);

        /* Parse visibility flag (columns can be hidden in output) */
        json_t *visible_val = json_object_get(col_obj, "visible");
        col->visible = json_is_boolean(visible_val) ? json_is_true(visible_val) : 1;
    }

    /* Parse sort configuration array for data ordering */
    json_t *sort_array = json_object_get(root, "sort");
    if (json_is_array(sort_array)) {
        /* Get the number of sort rules */
        config->sort_count = json_array_size(sort_array);
        /* Allocate memory for sort configurations */
        config->sorts = malloc(config->sort_count * sizeof(SortConfig));
        if (config->sorts == NULL && config->sort_count > 0) {
            fprintf(stderr, "Error: Memory allocation failed for sort config\n");
            json_decref(root);
            free_table_config(config);
            return 1;
        }

        /* Parse each sort rule */
        for (int i = 0; i < config->sort_count; i++) {
            json_t *sort_obj = json_array_get(sort_array, i);
            if (!json_is_object(sort_obj)) continue;

            SortConfig *sort = &config->sorts[i];
            /* Zero-initialize the sort config */
            memset(sort, 0, sizeof(SortConfig));

            /* Parse the key to sort by */
            json_t *key_val = json_object_get(sort_obj, "key");
            sort->key = strdup_safe(json_string_value(key_val));

            /* Parse sort direction (asc/desc) */
            json_t *dir_val = json_object_get(sort_obj, "direction");
            const char *dir_str = json_string_value(dir_val);
            /* 1 = descending, 0 = ascending */
            sort->direction = (dir_str && strcasecmp(dir_str, "desc") == 0) ? 1 : 0;

            /* Parse priority (lower number = higher priority for multi-key sort) */
            json_t *priority_val = json_object_get(sort_obj, "priority");
            sort->priority = json_is_number(priority_val) ? json_integer_value(priority_val) : 0;
        }
    } else {
        /* No sort array present */
        config->sort_count = 0;
        config->sorts = NULL;
    }

    /* Decrement reference count on the JSON root object */
    json_decref(root);
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON layout root object freed\n");
    }
    return 0;
}

/*
 * Free memory allocated for TableConfig structure
 * Iterates through all allocated fields and frees them safely
 */
void free_table_config(TableConfig *config) {
    /* Reference to global debug flag */
    extern int debug_mode;
    if (debug_mode) {
        fprintf(stderr, "Debug: Starting to free TableConfig structure\n");
    }
    /* Free theme name if allocated */
    if (config->theme_name) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free theme_name at address %p\n", (void*)config->theme_name);
        }
        free(config->theme_name);
        /* Set to NULL after freeing to prevent double-free */
        config->theme_name = NULL;
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed theme_name\n");
        }
    }
    /* Free title string if allocated */
    if (config->title) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free title at address %p\n", (void*)config->title);
        }
        free(config->title);
        /* Prevent double-free by nullifying the pointer */
        config->title = NULL;
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed title\n");
        }
    }
    /* Free footer string if allocated */
    if (config->footer) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free footer at address %p\n", (void*)config->footer);
        }
        free(config->footer);
        /* Nullify to prevent double-free */
        config->footer = NULL;
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed footer\n");
        }
    }

    /* Free each column's allocated fields, then the columns array itself */
    if (config->columns) {
        /* Iterate through all configured columns */
        for (int i = 0; i < config->column_count; i++) {
            ColumnConfig *col = &config->columns[i];
            /* Free header string */
            if (col->header) {
                free(col->header);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed header for column %d\n", i);
                }
            }
            /* Free key string */
            if (col->key) {
                free(col->key);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed key for column %d\n", i);
                }
            }
            /* Free format string */
            if (col->format) {
                free(col->format);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed format for column %d\n", i);
                }
            }
            /* Free wrap_char string */
            if (col->wrap_char) {
                free(col->wrap_char);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed wrap_char for column %d\n", i);
                }
            }
        }
        /* Free the columns array itself */
        free(config->columns);
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed columns array\n");
        }
    }

    /* Free each sort's allocated fields, then the sorts array */
    if (config->sorts) {
        /* Iterate through all configured sort rules */
        for (int i = 0; i < config->sort_count; i++) {
            SortConfig *sort = &config->sorts[i];
            /* Free sort key string */
            if (sort->key) {
                free(sort->key);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed key for sort %d\n", i);
                }
            }
        }
        /* Free the sorts array itself */
        free(config->sorts);
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed sorts array\n");
        }
    }

    /* Reset counts to zero for clean state */
    config->column_count = 0;
    config->sort_count = 0;
    if (debug_mode) {
        fprintf(stderr, "Debug: Completed freeing TableConfig structure\n");
    }
}
