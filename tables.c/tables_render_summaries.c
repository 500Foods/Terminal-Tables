/*
 * tables_render_summaries.c - Functions for rendering table summaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_summaries.h"
#include "tables_render_utils.h"

/*
 * Render the summaries row if any summaries are defined
 */
void render_summaries(TableConfig *config, TableData *data) {
    // Check if there are any summaries to render
    int has_summaries = 0;
    for (int j = 0; j < config->column_count; j++) {
        if (config->columns[j].summary != SUMMARY_NONE) {
            has_summaries = 1;
            break;
        }
    }
    if (!has_summaries) return;

    // Render summary separator
    printf("%s", config->theme.border_color);
    printf("%s", config->theme.l_junct);
    for (int j = 0; j < config->column_count; j++) {
        if (!config->columns[j].visible) continue;
        for (int w = 0; w < config->columns[j].width; w++) {
            printf("%s", config->theme.h_line);
        }
        if (j < config->column_count - 1) {
            printf("%s", config->theme.cross);
        }
    }
    printf("%s%s\n", config->theme.r_junct, config->theme.text_color);
    
    // Render summary row
    printf("%s%s", config->theme.border_color, config->theme.v_line);
    for (int j = 0; j < config->column_count; j++) {
        if (!config->columns[j].visible) continue;
        ColumnConfig *col = &config->columns[j];
        SummaryStats *stats = &data->summaries[j];
        char summary_text[256] = {0};
        switch (col->summary) {
            case SUMMARY_SUM:
                // Only show summary if sum is not zero
                if (stats->sum != 0.0) {
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
                } else {
                    summary_text[0] = '\0'; // Empty string for zero sum
                }
                break;
            case SUMMARY_MIN:
                // Only show summary if min has been set (stats->count > 0)
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
                    summary_text[0] = '\0'; // Empty string if no data
                }
                break;
            case SUMMARY_MAX:
                // Only show summary if max has been set (stats->count > 0)
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
                    summary_text[0] = '\0'; // Empty string if no data
                }
                break;
            case SUMMARY_AVG:
                if (stats->avg_count > 0) {
                    double avg_result = stats->avg_sum / stats->avg_count;
                    // Only show if average is not zero
                    if (avg_result != 0.0) {
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
                        summary_text[0] = '\0'; // Empty string for zero average
                    }
                } else {
                    summary_text[0] = '\0'; // Empty string if no data
                }
                break;
            case SUMMARY_COUNT:
                snprintf(summary_text, sizeof(summary_text), "%d", stats->count);
                break;
            case SUMMARY_UNIQUE:
                snprintf(summary_text, sizeof(summary_text), "%d", stats->unique_count);
                break;
            case SUMMARY_BLANKS:
                snprintf(summary_text, sizeof(summary_text), "%d", stats->blanks);
                char *formatted_blanks = format_with_commas(summary_text);
                strncpy(summary_text, formatted_blanks, sizeof(summary_text) - 1);
                free(formatted_blanks);
                break;
            case SUMMARY_NONBLANKS:
                snprintf(summary_text, sizeof(summary_text), "%d", stats->nonblanks);
                char *formatted_nonblanks = format_with_commas(summary_text);
                strncpy(summary_text, formatted_nonblanks, sizeof(summary_text) - 1);
                free(formatted_nonblanks);
                break;
            default:
                summary_text[0] = '\0';
        }
        char *summary_display = strdup_safe(summary_text);
        int summary_width = get_display_width(summary_display);
        int effective_width = col->width - 1; // Account for minimum padding, let rendering handle the rest
        if (summary_width > effective_width && col->wrap_mode == WRAP_CLIP) {
            char *truncated = malloc(col->width + 1);
            if (truncated) {
                int k = 0, display_count = 0;
                int in_ansi = 0;
                const char *start_p = summary_display;
                const char *end_p = summary_display + strlen(summary_display) - 1;
                
                if (col->justify == JUSTIFY_RIGHT) {
                    int target_count = effective_width;
                    for (const char *p = end_p; p >= summary_display && target_count > 0; p--) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) target_count--;
                        if (target_count <= 0) {
                            start_p = p;
                            break;
                        }
                    }
                    if (start_p < summary_display) start_p = summary_display;
                    for (const char *p = start_p; *p; p++) {
                        truncated[k++] = *p;
                    }
                } else if (col->justify == JUSTIFY_CENTER) {
                    int total_excess = summary_width - effective_width;
                    int left_excess = total_excess / 2;
                    int right_excess = total_excess - left_excess;
                    const char *left_cut = summary_display;
                    const char *right_cut = end_p;
                    int left_count = 0, right_count = 0;
                    
                    for (const char *p = summary_display; *p && left_count < left_excess; p++) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) left_count++;
                        left_cut = p;
                    }
                    in_ansi = 0;
                    for (const char *p = end_p; p >= summary_display && right_count < right_excess; p--) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) right_count++;
                        right_cut = p;
                    }
                    for (const char *p = left_cut; p <= right_cut && *p; p++) {
                        truncated[k++] = *p;
                    }
                } else {
                    for (const char *p = summary_display; *p && display_count < effective_width; p++) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) display_count++;
                        truncated[k++] = *p;
                    }
                }
                truncated[k] = '\0';
                free(summary_display);
                summary_display = truncated;
                summary_width = get_display_width(summary_display);
            }
        }
        int total_padding = col->width - summary_width;
        int padding_left = 1; // Exactly one space padding on left
        int padding_right = 1; // Exactly one space padding on right
        if (total_padding > 2) { // If more space is available, adjust based on justification
            int remaining_padding = total_padding - 2;
            if (col->justify == JUSTIFY_RIGHT) {
                padding_left += remaining_padding;
            } else if (col->justify == JUSTIFY_CENTER) {
                padding_left += remaining_padding / 2;
                padding_right += remaining_padding - (remaining_padding / 2);
            } else {
                padding_right += remaining_padding;
            }
        }
        printf("%s%*s%s%*s", config->theme.summary_color, padding_left, "", summary_display, padding_right, "");
        free(summary_display);
        printf("%s%s", config->theme.border_color, config->theme.v_line);
    }
    printf("%s\n", config->theme.text_color);
}
