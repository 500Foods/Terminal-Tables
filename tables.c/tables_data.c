/*
 * tables_data.c - Implementation of data processing for the tables utility
 * Handles loading, sorting, and summarizing data from JSON files.
 *
 * This file manages:
 *   - Reading and parsing data JSON files into TableData structures
 *   - Initializing summary statistics for each column
 *   - Sorting data rows based on configuration
 *   - Processing data rows to update summaries and column widths
 *   - Memory management for data structures
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <stdbool.h>
#include "tables_data.h"

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
 * Load and prepare data from JSON file
 * Reads the data JSON file, parses it into DataRow structures,
 * initializes summary statistics, and populates the TableData structure.
 * Returns 0 on success, 1 on error.
 */
int prepare_data(const char *data_file, TableConfig *config, TableData *data) {
    /* Jansson library JSON root and error tracking */
    json_t *root;
    json_error_t error;
    /* File handle for reading the data file */
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
        fprintf(stderr, "Debug: Starting to load data from %s\n", data_file);
    }

    /* Open the data file for reading */
    fp = fopen(data_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open data file %s\n", data_file);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Data file %s opened successfully\n", data_file);
    }

    /* Read file content into buffer in chunks, similar to layout file parsing */
    buffer_size = chunk_size;
    buffer = malloc(buffer_size + 1); /* Extra byte for null terminator */
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for buffer\n");
        fclose(fp);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Initial buffer allocated for reading data\n");
    }

    /* Loop to read the entire file content into the buffer */
    while (1) {
        /* Read a chunk of data into the buffer */
        size_t bytes_read = fread(buffer + total_read, 1, chunk_size, fp);
        /* Accumulate total bytes read */
        total_read += bytes_read;
        /* Check if we've read less than a full chunk (EOF or error) */
        if (bytes_read < chunk_size) {
            /* Check for end of file */
            if (feof(fp)) break;
            /* Check for read errors */
            if (ferror(fp)) {
                fprintf(stderr, "Error: Reading data file %s\n", data_file);
                free(buffer);
                fclose(fp);
                return 1;
            }
        }
        /* Expand buffer if full */
        buffer_size += chunk_size;
        char *new_buffer = realloc(buffer, buffer_size + 1); /* Extra byte for null terminator */
        if (new_buffer == NULL) {
            fprintf(stderr, "Error: Memory reallocation failed for buffer\n");
            free(buffer);
            fclose(fp);
            return 1;
        }
        /* Update buffer pointer to the newly allocated memory */
        buffer = new_buffer;
    }
    /* Close the file after reading completes */
    fclose(fp);

    /* Null-terminate the buffer for JSON parsing */
    buffer[total_read] = '\0';

    /* Parse the JSON buffer using the Jansson library */
    root = json_loads(buffer, 0, &error);
    free(buffer);
    if (root == NULL) {
        fprintf(stderr, "Error: JSON parsing failed for %s: %s\n", data_file, error.text);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON data parsed successfully from %s\n", data_file);
    }

    /* Validate that the root element is a JSON array */
    if (!json_is_array(root)) {
        fprintf(stderr, "Error: Data JSON root must be an array\n");
        json_decref(root);
        return 1;
    }

    /* Initialize the TableData structure to zero */
    memset(data, 0, sizeof(TableData));
    /* Get the number of rows from the JSON array */
    data->row_count = json_array_size(root);
    /* Allocate memory for all data rows */
    data->rows = malloc(data->row_count * sizeof(DataRow));
    if (data->rows == NULL && data->row_count > 0) {
        fprintf(stderr, "Error: Memory allocation failed for data rows\n");
        json_decref(root);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Allocated memory for %d data rows\n", data->row_count);
    }

    /* Allocate array for per-column summary statistics */
    data->summaries = malloc(config->column_count * sizeof(SummaryStats));
    if (data->summaries == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for summaries\n");
        free(data->rows);
        json_decref(root);
        return 1;
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Allocated memory for summaries of %d columns\n", config->column_count);
    }

    /* Initialize all summary statistics to zero/empty state */
    initialize_summaries(config, data);

    /* Process each JSON row object into a DataRow structure */
    for (int i = 0; i < data->row_count; i++) {
        /* Get the JSON object for this row */
        json_t *row_obj = json_array_get(root, i);
        if (!json_is_object(row_obj)) continue;

        DataRow *row = &data->rows[i];
        /* Check if this row should be annotated (display-only, excluded from summaries) */
        json_t *annotate_val = json_object_get(row_obj, "annotate");
        row->annotate = json_is_true(annotate_val);
        /* Allocate value array for all columns in this row */
        row->values = malloc(config->column_count * sizeof(char *));
        if (row->values == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for row values\n");
            /* Clean up previously allocated rows on error */
            for (int j = 0; j < i; j++) {
                for (int k = 0; k < config->column_count; k++) {
                    if (data->rows[j].values[k]) free(data->rows[j].values[k]);
                }
                free(data->rows[j].values);
            }
            free(data->rows);
            free(data->summaries);
            json_decref(root);
            return 1;
        }

        /* Extract each column value from the JSON object */
        for (int j = 0; j < config->column_count; j++) {
            /* Get the JSON key for this column from the config */
            const char *key = config->columns[j].key;
            json_t *val = json_object_get(row_obj, key);
            /* Handle different JSON value types */
            if (json_is_string(val)) {
                /* String values are duplicated directly */
                row->values[j] = strdup_safe(json_string_value(val));
            } else if (json_is_integer(val)) {
                /* Integer JSON values: format without scientific notation */
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%lld", (long long)json_integer_value(val));
                row->values[j] = strdup_safe(buffer);
            } else if (json_is_real(val)) {
                /* Real/float JSON values: use %g for clean representation */
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%g", json_real_value(val));
                row->values[j] = strdup_safe(buffer);
            } else if (json_is_number(val)) {
                /* Fallback for numbers that are neither integer nor real */
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%g", json_number_value(val));
                row->values[j] = strdup_safe(buffer);
            } else if (json_is_null(val)) {
                /* Explicit null values are stored as "null" string */
                row->values[j] = strdup_safe("null");
            } else {
                /* Any other type (boolean, object, array) defaults to "null" */
                row->values[j] = strdup_safe("null");
            }
        }
    }

    /* Free the JSON root object */
    json_decref(root);
    if (debug_mode) {
        fprintf(stderr, "Debug: JSON root object freed\n");
    }
    return 0;
}

/*
 * Initialize summaries for each column
 * Resets all SummaryStats fields to their default initial values
 */
void initialize_summaries(TableConfig *config, TableData *data) {
    /* Iterate over all columns and zero-initialize their summary stats */
    for (int i = 0; i < config->column_count; i++) {
        SummaryStats *stats = &data->summaries[i];
        /* Zero out the entire struct */
        memset(stats, 0, sizeof(SummaryStats));
        /* Flag to indicate if min has been initialized (min starts at 0, which is ambiguous) */
        stats->min_initialized = 0;
        /* Flag to indicate if max has been initialized (max starts at 0, which is ambiguous) */
        stats->max_initialized = 0;
        /* Track maximum decimal places found in float data for consistent formatting */
        stats->max_decimal_places = 0;
        /* Count of blank or zero values in this column */
        stats->blanks = 0;
        /* Count of non-blank or non-zero values in this column */
        stats->nonblanks = 0;
    }
}

/*
 * Sort data rows based on sort configuration
 * Currently a placeholder - sorting logic to be implemented in future versions
 */
void sort_data(TableConfig *config, TableData *data) {
    /* If no sort rules are defined, return immediately */
    if (config->sort_count == 0) return;

    /* TODO: Implement sorting logic */
    /* For now, just a placeholder to indicate sorting will be done here */
    /* Note: 'data' parameter is currently unused but will be needed for sorting implementation */
    (void)data; /* Suppress unused parameter warning */
}

/*
 * Process data rows, update summaries and calculate widths
 * Iterates through all rows, updates summary statistics for non-annotated rows,
 * and tracks the maximum number of lines per row for layout purposes.
 */
void process_data_rows(TableConfig *config, TableData *data) {
    /* Start with a minimum of 1 line per row */
    data->max_lines = 1;
    /* If there are no data rows, nothing to process */
    if (data->row_count == 0) return;

    /* Iterate through each data row */
    for (int i = 0; i < data->row_count; i++) {
        DataRow *row = &data->rows[i];
        /* Current row's line count (single line for now) */
        int line_count = 1;

        /* Annotated rows are display-only and skipped for summary updates */
        if (!row->annotate) {
            /* Process each column in this row */
            for (int j = 0; j < config->column_count; j++) {
                ColumnConfig *col = &config->columns[j];
                /* Get the raw string value from the row */
                const char *value = row->values[j];
                /* Determine the data type for this column */
                DataType data_type = col->data_type;
                /* Determine the summary type for this column */
                SummaryType summary_type = col->summary;

                /* Update summary stats for this column with this value */
                update_summaries(j, value, data_type, summary_type, &data->summaries[j]);
            }
        }

        /* Track maximum line count across all rows for multi-line rendering */
        if (line_count > data->max_lines) {
            data->max_lines = line_count;
        }
    }

    /* TODO: Update column widths based on summary values if summaries are present */
}

/*
 * Helper function to count decimal places in a string representation of a number
 * Examines the fractional part after the decimal point and counts digits
 */
static int count_decimal_places(const char *value) {
    /* Find the decimal point in the string */
    const char *decimal_point = strchr(value, '.');
    /* No decimal point means zero decimal places */
    if (decimal_point == NULL) {
        return 0;
    }
    /* Point to the character after the decimal point */
    const char *end = decimal_point + 1;
    /* Count consecutive digit characters */
    while (*end >= '0' && *end <= '9') {
        end++;
    }
    /* Return the count of digits after the decimal point */
    return end - decimal_point - 1;
}

/*
 * Update summary statistics for a column
 * Processes a single value for a column, updating sum, min, max, avg,
 * unique count, blanks/nonblanks count, and decimal place tracking.
 */
void update_summaries(int col_idx, const char *value, DataType data_type, SummaryType summary_type, SummaryStats *stats) {
    /* Reference to global debug flag */
    extern int debug_mode;

    /* Determine if the value is null (either NULL pointer or "null" string) */
    bool is_null = (value == NULL || strcmp(value, "null") == 0);
    /* Determine if the value is blank (null or empty string) */
    bool is_blank = is_null || (value && strcmp(value, "") == 0);

    /* For numeric types, also check for zero values (treated as blank) */
    if (!is_blank) {
        double num_val = 0.0;
        /* Determine if this data type is numeric */
        bool is_numeric = (data_type == DATA_INT || data_type == DATA_NUM || data_type == DATA_FLOAT ||
                           data_type == DATA_KCPU || data_type == DATA_KMEM);

        if (is_numeric) {
            /* Handle Kubernetes CPU values with 'm' suffix (millicores) */
            if (data_type == DATA_KCPU && strstr(value, "m") != NULL) {
                /* Duplicate the value string to safely modify it */
                char *num_part = strdup(value);
                if (num_part == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed for num_part\n");
                    num_val = 0.0;
                } else {
                    /* Remove trailing 'm' to get the numeric part */
                    num_part[strlen(num_part) - 1] = '\0';
                    /* Convert to double for numeric comparison */
                    num_val = atof(num_part);
                    free(num_part);
                }
            } else if (data_type == DATA_KMEM) {
                /* Handle Kubernetes memory values with unit suffixes */
                char *num_part = strdup(value);
                if (num_part == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed for num_part\n");
                    num_val = 0.0;
                } else {
                    /* Find the unit character (M, G, or K) */
                    char *unit = strstr(num_part, "M") ? strstr(num_part, "M") :
                                 strstr(num_part, "G") ? strstr(num_part, "G") :
                                 strstr(num_part, "K") ? strstr(num_part, "K") : NULL;
                    /* Truncate at the unit position */
                    if (unit) *unit = '\0';
                    num_val = atof(num_part);
                    free(num_part);
                }
            } else {
                /* Standard numeric conversion for int, num, float types */
                num_val = atof(value);
            }
            /* Treat zero numeric values as blank */
            if (num_val == 0.0) is_blank = true;
        }
    }

    /* Update blanks or nonblanks counter based on value status */
    if (is_blank) {
        stats->blanks++;
    } else {
        stats->nonblanks++;
    }

    /* For null values, stop processing (no numeric stats to update) */
    if (is_null) {
        return;
    }

    /* Track maximum decimal places for float formatting consistency */
    if (data_type == DATA_FLOAT) {
        int decimal_places = count_decimal_places(value);
        if (decimal_places > stats->max_decimal_places) {
            stats->max_decimal_places = decimal_places;
        }
    }

    /* Always increment count for non-null values (needed for min/max display logic) */
    stats->count++;

    /* Process numeric values for sum, min, max, and avg calculations */
    if (data_type == DATA_INT || data_type == DATA_NUM || data_type == DATA_FLOAT) {
        /* Convert string to double */
        double num_val = atof(value);

        /* Accumulate sum for summary */
        stats->sum += num_val;

        /* Update minimum value */
        if (!stats->min_initialized) {
            /* First value encountered becomes the initial min */
            stats->min = num_val;
            stats->min_initialized = 1;
        } else if (num_val < stats->min) {
            /* New value is smaller than current min */
            stats->min = num_val;
        }

        /* Update maximum value */
        if (!stats->max_initialized) {
            /* First value encountered becomes the initial max */
            stats->max = num_val;
            stats->max_initialized = 1;
        } else if (num_val > stats->max) {
            /* New value is larger than current max */
            stats->max = num_val;
        }

        /* Update average tracking fields */
        stats->avg_sum += num_val;
        stats->avg_count++;

    } else if (data_type == DATA_KCPU && strstr(value, "m") != NULL) {
        /* Handle millicores values for CPU type */
        char *num_part = strdup(value);
        if (num_part) {
            /* Remove trailing 'm' suffix */
            num_part[strlen(num_part) - 1] = '\0';
            /* Convert to double */
            double num_val = atof(num_part);
            free(num_part);

            /* Update sum */
            stats->sum += num_val;

            /* Update min */
            if (!stats->min_initialized) {
                stats->min = num_val;
                stats->min_initialized = 1;
            } else if (num_val < stats->min) {
                stats->min = num_val;
            }

            /* Update max */
            if (!stats->max_initialized) {
                stats->max = num_val;
                stats->max_initialized = 1;
            } else if (num_val > stats->max) {
                stats->max = num_val;
            }
        }

    } else if (data_type == DATA_KMEM) {
        /* Handle Kubernetes memory values with various unit suffixes */
        char *num_part = strdup(value);
        if (num_part) {
            /* Determine the conversion multiplier based on unit */
            double multiplier = 1.0;
            double num_val = 0.0;

            /* Mi (mebibytes) - multiplier 1.0 */
            if (strstr(value, "Mi") != NULL) {
                num_part[strlen(num_part) - 2] = '\0';
                multiplier = 1.0;
            /* M (megabytes) - multiplier 1.0 */
            } else if (strstr(value, "M") != NULL) {
                num_part[strlen(num_part) - 1] = '\0';
                multiplier = 1.0;
            /* Gi (gibibytes) - multiplier 1000.0 */
            } else if (strstr(value, "Gi") != NULL) {
                num_part[strlen(num_part) - 2] = '\0';
                multiplier = 1000.0;
            /* G (gigabytes) - multiplier 1000.0 */
            } else if (strstr(value, "G") != NULL) {
                num_part[strlen(num_part) - 1] = '\0';
                multiplier = 1000.0;
            /* Ki (kibibytes) - multiplier 1/1000 */
            } else if (strstr(value, "Ki") != NULL) {
                num_part[strlen(num_part) - 2] = '\0';
                multiplier = 1.0 / 1000.0;
            /* K (kilobytes) - multiplier 1/1000 */
            } else if (strstr(value, "K") != NULL) {
                num_part[strlen(num_part) - 1] = '\0';
                multiplier = 1.0 / 1000.0;
            }

            /* Apply multiplier and update stats */
            num_val = atof(num_part) * multiplier;
            free(num_part);

            /* Update sum */
            stats->sum += num_val;

            /* Update min */
            if (!stats->min_initialized) {
                stats->min = num_val;
                stats->min_initialized = 1;
            } else if (num_val < stats->min) {
                stats->min = num_val;
            }

            /* Update max */
            if (!stats->max_initialized) {
                stats->max = num_val;
                stats->max_initialized = 1;
            } else if (num_val > stats->max) {
                stats->max = num_val;
            }
        }
    }

    /* Handle unique values tracking (only when needed) */
    if (summary_type == SUMMARY_UNIQUE) {
        /* Check if value is already in unique_values array */
        for (int i = 0; i < stats->unique_count; i++) {
            if (strcmp(stats->unique_values[i], value) == 0) {
                if (debug_mode) {
                    fprintf(stderr, "Debug: Value '%s' already in unique_values for column %d\n", value, col_idx);
                }
                /* Value already exists, skip adding */
                return;
            }
        }
        /* Add new unique value to the array */
        if (debug_mode) {
            fprintf(stderr, "Debug: Adding new unique value '%s' for column %d, new count will be %d\n", value, col_idx, stats->unique_count + 1);
        }
        /* Reallocate the unique_values array to accommodate one more entry */
        char **new_unique_values = realloc(stats->unique_values, (stats->unique_count + 1) * sizeof(char *));
        if (new_unique_values == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for unique values\n");
            return;
        }
        stats->unique_values = new_unique_values;
        /* Duplicate and store the new unique value */
        stats->unique_values[stats->unique_count] = strdup_safe(value);
        if (stats->unique_values[stats->unique_count] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for unique value string\n");
        } else {
            if (debug_mode) {
                fprintf(stderr, "Debug: Successfully added unique value '%s' at index %d for column %d\n", value, stats->unique_count, col_idx);
            }
        }
        /* Increment the unique value count */
        stats->unique_count++;
    }
}

/*
 * Free memory allocated for TableData structure
 * Frees all data rows (including their values), summary stats, and unique values
 */
void free_table_data(TableData *data, int column_count) {
    /* Free each data row and its values */
    if (data->rows) {
        /* Iterate through each row */
        for (int i = 0; i < data->row_count; i++) {
            DataRow *row = &data->rows[i];
            /* Free each column value in the row, if allocated */
            if (row->values) {
                for (int j = 0; j < column_count; j++) {
                    if (row->values[j]) free(row->values[j]);
                }
                /* Free the values array itself */
                free(row->values);
            }
        }
        /* Free the rows array */
        free(data->rows);
    }

    /* Free summary statistics for each column */
    if (data->summaries) {
        /* Iterate through each column's summary stats */
        for (int i = 0; i < column_count; i++) {
            SummaryStats *stats = &data->summaries[i];
            /* Free unique values array if present */
            if (stats->unique_values) {
                for (int j = 0; j < stats->unique_count; j++) {
                    if (stats->unique_values[j]) free(stats->unique_values[j]);
                }
                free(stats->unique_values);
            }
        }
        /* Free the summaries array */
        free(data->summaries);
    }

    /* Reset counts to zero */
    data->row_count = 0;
    data->max_lines = 0;
}
