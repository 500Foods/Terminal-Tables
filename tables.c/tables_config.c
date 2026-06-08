/*
 * tables_config.c - Implementation of configuration parsing for the tables utility
 * Parses layout JSON files and manages configuration structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>
#include "tables_config.h"

/*
 * Helper function to duplicate a string, returning NULL if input is NULL
 */
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
 * Helper function to parse justification string to enum
 */
static Justification parse_justification(const char *str) {
    if (str == NULL) return JUSTIFY_LEFT;
    if (strcasecmp(str, "right") == 0) return JUSTIFY_RIGHT;
    if (strcasecmp(str, "center") == 0) return JUSTIFY_CENTER;
    return JUSTIFY_LEFT;
}

/*
 * Helper function to parse data type string to enum
 */
static DataType parse_data_type(const char *str) {
    if (str == NULL) return DATA_TEXT;
    if (strcasecmp(str, "int") == 0) return DATA_INT;
    if (strcasecmp(str, "num") == 0) return DATA_NUM;
    if (strcasecmp(str, "float") == 0) return DATA_FLOAT;
    if (strcasecmp(str, "kcpu") == 0) return DATA_KCPU;
    if (strcasecmp(str, "kmem") == 0) return DATA_KMEM;
    return DATA_TEXT;
}

/*
 * Helper function to parse value display string to enum
 */
static ValueDisplay parse_value_display(const char *str) {
    if (str == NULL) return VALUE_BLANK;
    if (strcasecmp(str, "0") == 0) return VALUE_ZERO;
    if (strcasecmp(str, "missing") == 0) return VALUE_MISSING;
    return VALUE_BLANK;
}

/*
 * Helper function to parse summary type string to enum
 */
static SummaryType parse_summary_type(const char *str) {
    if (str == NULL) return SUMMARY_NONE;
    if (strcasecmp(str, "sum") == 0) return SUMMARY_SUM;
    if (strcasecmp(str, "min") == 0) return SUMMARY_MIN;
    if (strcasecmp(str, "max") == 0) return SUMMARY_MAX;
    if (strcasecmp(str, "avg") == 0) return SUMMARY_AVG;
    if (strcasecmp(str, "count") == 0) return SUMMARY_COUNT;
    if (strcasecmp(str, "unique") == 0) return SUMMARY_UNIQUE;
    if (strcasecmp(str, "blanks") == 0) return SUMMARY_BLANKS;
    if (strcasecmp(str, "nonblanks") == 0) return SUMMARY_NONBLANKS;
    return SUMMARY_NONE;
}

/*
 * Helper function to parse wrap mode string to enum
 */
static WrapMode parse_wrap_mode(const char *str) {
    if (str == NULL) return WRAP_CLIP;
    if (strcasecmp(str, "wrap") == 0) return WRAP_WRAP;
    return WRAP_CLIP;
}

/*
 * Helper function to parse position string to enum
 */
static Position parse_position(const char *str) {
    if (str == NULL) return POSITION_NONE;
    if (strcasecmp(str, "left") == 0) return POSITION_LEFT;
    if (strcasecmp(str, "right") == 0) return POSITION_RIGHT;
    if (strcasecmp(str, "center") == 0) return POSITION_CENTER;
    if (strcasecmp(str, "full") == 0) return POSITION_FULL;
    return POSITION_NONE;
}

/*
 * Validate input files exist and are non-empty
 */
int validate_input_files(const char *layout_file, const char *data_file) {
    FILE *fp;
    
    fp = fopen(layout_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open layout file %s\n", layout_file);
        return 1;
    }
    fclose(fp);
    
    fp = fopen(data_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open data file %s\n", data_file);
        return 1;
    }
    fclose(fp);
    
    return 0;
}

/*
 * Parse layout JSON file into TableConfig structure
 */
int parse_layout_file(const char *filename, TableConfig *config) {
    json_t *root;
    json_error_t error;
    FILE *fp;
    char *buffer = NULL;
    size_t buffer_size = 0;
    size_t total_read = 0;
    size_t chunk_size = 1024;
    extern int debug_mode;
    
    if (debug_mode) {
        fprintf(stderr, "Debug: Starting to parse layout file %s\n", filename);
    }
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open layout file %s\n", filename);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Layout file %s opened successfully\n", filename);
    }
    
    // Read file content into buffer
    buffer = malloc(chunk_size + 1); // Extra byte for null terminator
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for buffer\n");
        fclose(fp);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Allocated initial buffer of size %zu for layout file\n", chunk_size + 1);
    }
    buffer_size = chunk_size;
    
    while (1) {
        size_t bytes_read = fread(buffer + total_read, 1, chunk_size, fp);
        total_read += bytes_read;
        if (debug_mode) {
            fprintf(stderr, "Debug: Read %zu bytes, total read now %zu\n", bytes_read, total_read);
        }
        if (bytes_read < chunk_size) {
            if (feof(fp)) {
                if (debug_mode) {
                    fprintf(stderr, "Debug: End of file reached\n");
                }
                break;
            }
            if (ferror(fp)) {
                fprintf(stderr, "Error: Reading layout file %s\n", filename);
                free(buffer);
                fclose(fp);
                return 1;
            }
        }
        buffer_size += chunk_size;
        char *new_buffer = realloc(buffer, buffer_size + 1); // Extra byte for null terminator
        if (new_buffer == NULL) {
            fprintf(stderr, "Error: Memory reallocation failed for buffer\n");
            free(buffer);
            fclose(fp);
            return 1;
        }
        if (debug_mode) {
            fprintf(stderr, "Debug: Reallocated buffer to size %zu for layout file\n", buffer_size + 1);
        }
        buffer = new_buffer;
    }
    fclose(fp);
    
    // Null-terminate the buffer
    buffer[total_read] = '\0';
    if (debug_mode) {
        fprintf(stderr, "Debug: Read %zu bytes from layout file, buffer null-terminated\n", total_read);
    }
    
    // Parse JSON
    if (debug_mode) {
        fprintf(stderr, "Debug: Starting JSON parsing for layout file\n");
    }
    root = json_loads(buffer, 0, &error);
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON parsing completed, freeing buffer\n");
    }
    free(buffer);
    if (root == NULL) {
        fprintf(stderr, "Error: JSON parsing failed for %s: %s\n", filename, error.text);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON layout parsed successfully from %s\n", filename);
    }
    
    // Initialize config structure
    memset(config, 0, sizeof(TableConfig));
    
    // Parse theme name
    json_t *theme_val = json_object_get(root, "theme");
    config->theme_name = strdup_safe(json_string_value(theme_val) ? json_string_value(theme_val) : "Red");
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed theme_name as '%s'\n", config->theme_name ? config->theme_name : "NULL");
    }
    
    // Parse title and position
    json_t *title_val = json_object_get(root, "title");
    config->title = strdup_safe(json_string_value(title_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed title as '%s'\n", config->title ? config->title : "NULL");
    }
    json_t *title_pos_val = json_object_get(root, "title_position");
    config->title_pos = parse_position(json_string_value(title_pos_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed title_position as %d\n", config->title_pos);
    }
    
    // Parse footer and position
    json_t *footer_val = json_object_get(root, "footer");
    config->footer = strdup_safe(json_string_value(footer_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed footer as '%s'\n", config->footer ? config->footer : "NULL");
    }
    json_t *footer_pos_val = json_object_get(root, "footer_position");
    config->footer_pos = parse_position(json_string_value(footer_pos_val));
    if (debug_mode) {
        fprintf(stderr, "Debug: Parsed footer_position as %d\n", config->footer_pos);
    }
    
    // Parse columns array
    json_t *columns_array = json_object_get(root, "columns");
    if (!json_is_array(columns_array) || json_array_size(columns_array) == 0) {
        fprintf(stderr, "Error: No columns defined in layout JSON\n");
        json_decref(root);
        free_table_config(config);
        return 1;
    }
    
    config->column_count = json_array_size(columns_array);
    if (config->column_count > MAX_COLUMNS) {
        fprintf(stderr, "Warning: Too many columns, truncating to %d\n", MAX_COLUMNS);
        config->column_count = MAX_COLUMNS;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Configured %d columns for layout\n", config->column_count);
    }
    
    config->columns = malloc(config->column_count * sizeof(ColumnConfig));
    if (config->columns == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for columns\n");
        json_decref(root);
        free_table_config(config);
        return 1;
    }
    
    for (int i = 0; i < config->column_count; i++) {
        json_t *col_obj = json_array_get(columns_array, i);
        if (!json_is_object(col_obj)) continue;
        
        ColumnConfig *col = &config->columns[i];
        memset(col, 0, sizeof(ColumnConfig));
        
        json_t *header_val = json_object_get(col_obj, "header");
        col->header = strdup_safe(json_string_value(header_val));
        if (col->header == NULL || strlen(col->header) == 0) {
            fprintf(stderr, "Error: Column %d has no header\n", i);
            json_decref(root);
            free_table_config(config);
            return 1;
        }
        
        json_t *key_val = json_object_get(col_obj, "key");
        const char *key_str = json_string_value(key_val);
        if (key_str == NULL || strlen(key_str) == 0) {
            // Derive key from header (lowercase, replace non-alphanumeric with underscore)
            char *derived_key = strdup(col->header);
            if (derived_key == NULL) {
                fprintf(stderr, "Error: Memory allocation failed for derived key\n");
                json_decref(root);
                free_table_config(config);
                return 1;
            }
            for (char *p = derived_key; *p; p++) {
                if (!isalnum(*p)) *p = '_';
                else *p = tolower(*p);
            }
            col->key = derived_key;
        } else {
            col->key = strdup_safe(key_str);
        }
        
        json_t *justify_val = json_object_get(col_obj, "justification");
        col->justify = parse_justification(json_string_value(justify_val));
        
        json_t *datatype_val = json_object_get(col_obj, "datatype");
        col->data_type = parse_data_type(json_string_value(datatype_val));
        
        json_t *null_val = json_object_get(col_obj, "null_value");
        col->null_val = parse_value_display(json_string_value(null_val));
        
        json_t *zero_val = json_object_get(col_obj, "zero_value");
        col->zero_val = parse_value_display(json_string_value(zero_val));
        
        json_t *format_val = json_object_get(col_obj, "format");
        col->format = strdup_safe(json_string_value(format_val));
        
        json_t *summary_val = json_object_get(col_obj, "summary");
        col->summary = parse_summary_type(json_string_value(summary_val));
        
        json_t *break_val = json_object_get(col_obj, "break");
        col->break_on_change = json_is_true(break_val);
        
        json_t *string_limit_val = json_object_get(col_obj, "string_limit");
        col->string_limit = json_is_number(string_limit_val) ? json_integer_value(string_limit_val) : 0;
        
        json_t *wrap_mode_val = json_object_get(col_obj, "wrap_mode");
        col->wrap_mode = parse_wrap_mode(json_string_value(wrap_mode_val));
        
        json_t *wrap_char_val = json_object_get(col_obj, "wrap_char");
        col->wrap_char = strdup_safe(json_string_value(wrap_char_val));
        
        json_t *padding_val = json_object_get(col_obj, "padding");
        col->padding = json_is_number(padding_val) ? json_integer_value(padding_val) : DEFAULT_PADDING;
        
        json_t *width_val = json_object_get(col_obj, "width");
        col->width = json_is_number(width_val) ? json_integer_value(width_val) : 0;
        col->width_specified = (col->width > 0);
        
        json_t *visible_val = json_object_get(col_obj, "visible");
        col->visible = json_is_boolean(visible_val) ? json_is_true(visible_val) : 1;
    }
    
    // Parse sort array
    json_t *sort_array = json_object_get(root, "sort");
    if (json_is_array(sort_array)) {
        config->sort_count = json_array_size(sort_array);
        config->sorts = malloc(config->sort_count * sizeof(SortConfig));
        if (config->sorts == NULL && config->sort_count > 0) {
            fprintf(stderr, "Error: Memory allocation failed for sort config\n");
            json_decref(root);
            free_table_config(config);
            return 1;
        }
        
        for (int i = 0; i < config->sort_count; i++) {
            json_t *sort_obj = json_array_get(sort_array, i);
            if (!json_is_object(sort_obj)) continue;
            
            SortConfig *sort = &config->sorts[i];
            memset(sort, 0, sizeof(SortConfig));
            
            json_t *key_val = json_object_get(sort_obj, "key");
            sort->key = strdup_safe(json_string_value(key_val));
            
            json_t *dir_val = json_object_get(sort_obj, "direction");
            const char *dir_str = json_string_value(dir_val);
            sort->direction = (dir_str && strcasecmp(dir_str, "desc") == 0) ? 1 : 0;
            
            json_t *priority_val = json_object_get(sort_obj, "priority");
            sort->priority = json_is_number(priority_val) ? json_integer_value(priority_val) : 0;
        }
    } else {
        config->sort_count = 0;
        config->sorts = NULL;
    }
    
    json_decref(root);
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON layout root object freed\n");
    }
    return 0;
}

/*
 * Free memory allocated for TableConfig structure
 */
void free_table_config(TableConfig *config) {
    extern int debug_mode;
    if (debug_mode) {
        fprintf(stderr, "Debug: Starting to free TableConfig structure\n");
    }
    if (config->theme_name) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free theme_name at address %p\n", (void*)config->theme_name);
        }
        free(config->theme_name);
        config->theme_name = NULL; // Set to NULL after freeing to prevent double-free
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed theme_name\n");
        }
    }
    if (config->title) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free title at address %p\n", (void*)config->title);
        }
        free(config->title);
        config->title = NULL; // Set to NULL after freeing to prevent double-free
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed title\n");
        }
    }
    if (config->footer) {
        if (debug_mode) {
            fprintf(stderr, "Debug: About to free footer at address %p\n", (void*)config->footer);
        }
        free(config->footer);
        config->footer = NULL; // Set to NULL after freeing to prevent double-free
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed footer\n");
        }
    }
    
    if (config->columns) {
        for (int i = 0; i < config->column_count; i++) {
            ColumnConfig *col = &config->columns[i];
            if (col->header) {
                free(col->header);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed header for column %d\n", i);
                }
            }
            if (col->key) {
                free(col->key);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed key for column %d\n", i);
                }
            }
            if (col->format) {
                free(col->format);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed format for column %d\n", i);
                }
            }
            if (col->wrap_char) {
                free(col->wrap_char);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed wrap_char for column %d\n", i);
                }
            }
        }
        free(config->columns);
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed columns array\n");
        }
    }
    
    if (config->sorts) {
        for (int i = 0; i < config->sort_count; i++) {
            SortConfig *sort = &config->sorts[i];
            if (sort->key) {
                free(sort->key);
                if (debug_mode) {
                    fprintf(stderr, "Debug: Freed key for sort %d\n", i);
                }
            }
        }
        free(config->sorts);
        if (debug_mode) {
            fprintf(stderr, "Debug: Freed sorts array\n");
        }
    }
    
    // Reset counts
    config->column_count = 0;
    config->sort_count = 0;
    if (debug_mode) {
        fprintf(stderr, "Debug: Completed freeing TableConfig structure\n");
    }
}
