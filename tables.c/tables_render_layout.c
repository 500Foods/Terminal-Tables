/*
 * tables_render_layout.c - Functions for calculating layout dimensions for table rendering
 * Contains functions for determining column widths based on content and configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_layout.h"
#include "tables_datatypes.h"
#include "tables_render_utils.h"

/*
 * Calculate column widths based on content and configuration
 */
void calculate_column_widths(TableConfig *config, TableData *data) {
    for (int j = 0; j < config->column_count; j++) {
        ColumnConfig *col = &config->columns[j];
        if (col->width_specified) continue; // Width already specified in config
        
        int max_width = 0;
        if (col->header) {
            int header_width = get_display_width(col->header);
            if (header_width > max_width) max_width = header_width;
        }
        
        // Check data rows
        for (int i = 0; i < data->row_count; i++) {
            const char *value = data->rows[i].values[j];
            char *formatted = format_display_value_with_precision(value, col->null_val, col->zero_val, col->data_type, col->format, col->string_limit, col->wrap_mode, col->wrap_char, col->justify, data->summaries[j].max_decimal_places);
            int width = get_display_width(formatted);
            if (width > max_width) max_width = width;
            free(formatted);
        }
        
        // Check summary if present
        if (col->summary != SUMMARY_NONE) {
            SummaryStats *stats = &data->summaries[j];
            char summary_text[256];
            switch (col->summary) {
                case SUMMARY_SUM:
                    if (col->data_type == DATA_KCPU) {
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        snprintf(summary_text, sizeof(summary_text), "%sm", formatted);
                        free(formatted);
                    } else if (col->data_type == DATA_KMEM) {
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        snprintf(summary_text, sizeof(summary_text), "%sM", formatted);
                        free(formatted);
                    } else if (col->data_type == DATA_FLOAT) {
                        char format[16];
                        snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                        snprintf(summary_text, sizeof(summary_text), format, stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    } else if (col->data_type == DATA_INT || col->data_type == DATA_NUM) {
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    } else {
                        snprintf(summary_text, sizeof(summary_text), "%.2f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                        summary_text[sizeof(summary_text) - 1] = '\0';
                        free(formatted);
                    }
                    break;
                case SUMMARY_MIN:
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
                        summary_text[0] = '\0';
                    }
                    break;
                case SUMMARY_MAX:
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
                        summary_text[0] = '\0';
                    }
                    break;
                case SUMMARY_AVG:
                    if (stats->avg_count > 0) {
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
                        snprintf(summary_text, sizeof(summary_text), "N/A");
                    }
                    break;
                case SUMMARY_COUNT:
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->count);
                    break;
                case SUMMARY_UNIQUE:
                    snprintf(summary_text, sizeof(summary_text), "%d", stats->unique_count);
                    break;
                default:
                    summary_text[0] = '\0';
            }
            int summary_width = get_display_width(summary_text);
            if (summary_width > max_width) max_width = summary_width;
        }
        
    col->width = max_width + 2; // Add 1 character padding on each side
}
}

/*
 * Calculate the total width of the table
 */
int calculate_total_width(TableConfig *config) {
    int total_width = 0;
    int visible_columns = 0;

    // Sum the widths of visible columns
    for (int j = 0; j < config->column_count; j++) {
        if (config->columns[j].visible) {
            total_width += config->columns[j].width;
            visible_columns++;
        }
    }

    // Add width for vertical separators (one less than the number of visible columns)
    if (visible_columns > 0) {
        total_width += (visible_columns - 1);
    }

    // Add width for left and right borders
    total_width += 2;

    return total_width;
}
