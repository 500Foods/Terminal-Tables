/*
 * tables_render_layout.c - Functions for calculating layout dimensions for table rendering
 *
 * This file computes the visual geometry of the table:
 *   - calculate_column_widths(): Determines each column's width based on
 *     header text, data values, and summary values
 *   - calculate_total_width(): Sums visible column widths plus borders/separators
 *
 * Column widths are computed after data is loaded and summaries are calculated,
 * so that all content (headers, data, summaries) fits within the table.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_layout.h"
#include "tables_datatypes.h"
#include "tables_render_utils.h"

/*
 * Calculate column widths based on content and configuration
 * For each column:
 *   1. If width is explicitly specified, skip (already set)
 *   2. Start with header width as minimum
 *   3. Check all data values (formatted with proper type handling)
 *   4. Check summary values if a summary type is configured
 *   5. Set final width = max content width + 2 (1 padding space on each side)
 *
 * Parameters:
 *   config: Table configuration with column definitions
 *   data:   Table data with rows, summaries, and decimal place tracking
 */
void calculate_column_widths(TableConfig *config, TableData *data) {
    /* Iterate over each column */
    for (int j = 0; j < config->column_count; j++) {
        ColumnConfig *col = &config->columns[j];
        /* Skip columns with explicitly specified width */
        if (col->width_specified) continue; /* Width already specified in config */

        /* Track the maximum width needed for this column */
        int max_width = 0;
        /* Check if header exceeds current max */
        if (col->header) {
            int header_width = get_display_width(col->header);
            if (header_width > max_width) max_width = header_width;
        }

        /* Check data rows for wider values */
        for (int i = 0; i < data->row_count; i++) {
            /* Get the raw value from the data row */
            const char *value = data->rows[i].values[j];
            /* Format the value using type-aware formatting with precision tracking */
            char *formatted = format_display_value_with_precision(value, col->null_val, col->zero_val, col->data_type, col->format, col->string_limit, col->wrap_mode, col->wrap_char, col->justify, data->summaries[j].max_decimal_places);
            /* Replace color placeholders to get true display width */
            char *colored = replace_color_placeholders(formatted);
            int width = get_display_width(colored);
            /* Update max_width if this value is wider */
            if (width > max_width) max_width = width;
            free(colored);
            free(formatted);
        }

        /* Check summary values if this column has a summary type */
        if (col->summary != SUMMARY_NONE) {
            SummaryStats *stats = &data->summaries[j];
            char summary_text[256];
            /* Build summary text based on summary type and data type */
            switch (col->summary) {
                case SUMMARY_SUM:
                    /* Sum: aggregate of all values in the column */
                    if (col->data_type == DATA_KCPU) {
                        /* CPU sum: format as millicores */
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        snprintf(summary_text, sizeof(summary_text), "%sm", formatted);
                        free(formatted);
                    } else if (col->data_type == DATA_KMEM) {
                        /* Memory sum: format with M suffix */
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        snprintf(summary_text, sizeof(summary_text), "%sM", formatted);
                        free(formatted);
                    } else if (col->data_type == DATA_FLOAT) {
                        /* Float sum: format with consistent decimal places */
                        char format[16];
                        snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                        snprintf(summary_text, sizeof(summary_text), format, stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    } else if (col->data_type == DATA_INT) {
                        /* Integer sum: raw value, no comma separators */
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                    } else if (col->data_type == DATA_NUM) {
                        /* Num sum: with comma formatting */
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    } else {
                        /* Text sum: fallback to 2 decimal places */
                        snprintf(summary_text, sizeof(summary_text), "%.2f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    }
                    break;
                case SUMMARY_MIN:
                    /* Min: smallest value in the column */
                    if (stats->count > 0) {
                        if (col->data_type == DATA_KCPU) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->min);
                            char *formatted = format_with_commas(summary_text);
                            snprintf(summary_text, sizeof(summary_text), "%sm", formatted);
                            free(formatted);
                        } else if (col->data_type == DATA_KMEM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->min);
                            char *formatted = format_with_commas(summary_text);
                            snprintf(summary_text, sizeof(summary_text), "%sM", formatted);
                            free(formatted);
                        } else if (col->data_type == DATA_FLOAT) {
                            char format[16];
                            snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                            snprintf(summary_text, sizeof(summary_text), format, stats->min);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else if (col->data_type == DATA_INT || col->data_type == DATA_NUM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->min);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else {
                            snprintf(summary_text, sizeof(summary_text), "%.2f", stats->min);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        }
                    } else {
                        /* No data: empty summary text */
                        summary_text[0] = '\0';
                    }
                    break;
                case SUMMARY_MAX:
                    /* Max: largest value in the column */
                    if (stats->count > 0) {
                        if (col->data_type == DATA_KCPU) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->max);
                            char *formatted = format_with_commas(summary_text);
                            snprintf(summary_text, sizeof(summary_text), "%sm", formatted);
                            free(formatted);
                        } else if (col->data_type == DATA_KMEM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->max);
                            char *formatted = format_with_commas(summary_text);
                            snprintf(summary_text, sizeof(summary_text), "%sM", formatted);
                            free(formatted);
                        } else if (col->data_type == DATA_FLOAT) {
                            char format[16];
                            snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                            snprintf(summary_text, sizeof(summary_text), format, stats->max);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else if (col->data_type == DATA_INT || col->data_type == DATA_NUM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", stats->max);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else {
                            snprintf(summary_text, sizeof(summary_text), "%.2f", stats->max);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        }
                    } else {
                        /* No data: empty summary text */
                        summary_text[0] = '\0';
                    }
                    break;
                case SUMMARY_AVG:
                    /* Avg: average of all values in the column */
                    if (stats->avg_count > 0) {
                        /* Calculate average from accumulated sum and count */
                        double avg_result = stats->avg_sum / stats->avg_count;
                        if (col->data_type == DATA_FLOAT) {
                            char format[16];
                            snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                            snprintf(summary_text, sizeof(summary_text), format, avg_result);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else if (col->data_type == DATA_INT || col->data_type == DATA_NUM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", avg_result);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else {
                            snprintf(summary_text, sizeof(summary_text), "%.2f", avg_result);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        }
                    } else {
                        /* No data: display "N/A" for empty average */
                        snprintf(summary_text, sizeof(summary_text), "N/A");
                    }
                    break;
                case SUMMARY_COUNT:
                    /* Count: number of non-null values */
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->count);
                    break;
                case SUMMARY_UNIQUE:
                    /* Unique: number of distinct values */
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->unique_count);
                    break;
                case SUMMARY_BLANKS:
                    /* Blanks: count of blank/zero values with comma formatting */
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->blanks);
                    char *formatted_blanks = format_with_commas(summary_text);
                    strncpy(summary_text, formatted_blanks, sizeof(summary_text) - 1);
                    free(formatted_blanks);
                    break;
                case SUMMARY_NONBLANKS:
                    /* Nonblanks: count of non-blank/non-zero values with comma formatting */
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->nonblanks);
                    char *formatted_nonblanks = format_with_commas(summary_text);
                    strncpy(summary_text, formatted_nonblanks, sizeof(summary_text) - 1);
                    free(formatted_nonblanks);
                    break;
                default:
                    /* No summary for this type */
                    summary_text[0] = '\0';
            }
            /* Update max_width with the summary text width if applicable */
            int summary_width = get_display_width(summary_text);
            if (summary_width > max_width) max_width = summary_width;
        }

        /* Final column width = max content width + 2 (1 space padding on each side) */
        col->width = max_width + 2; /* Add 1 character padding on each side */
    }
}

/*
 * Calculate the total width of the table
 * Total width = sum of visible column widths + (num_visible - 1) vertical separators + 2 border characters
 *
 * Parameters:
 *   config: Table configuration with column definitions and theme settings
 *
 * Returns: The total display width of the rendered table
 */
int calculate_total_width(TableConfig *config) {
    /* Accumulator for total width */
    int total_width = 0;
    /* Counter for visible columns */
    int visible_columns = 0;

    /* Sum the widths of visible columns */
    for (int j = 0; j < config->column_count; j++) {
        if (config->columns[j].visible) {
            total_width += config->columns[j].width;
            visible_columns++;
        }
    }

    /* Add width for vertical separators (one less than number of visible columns) */
    if (visible_columns > 0) {
        total_width += (visible_columns - 1);
    }

    /* Add width for left and right borders (2 border characters) */
    total_width += 2;

    return total_width;
}
