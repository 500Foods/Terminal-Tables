/*
 * tables_render_summaries.c - Functions for rendering table summaries
 *
 * This file implements the summary row rendering, which appears between
 * the data rows and the bottom border. Summaries provide aggregate
 * statistics (sum, min, max, avg, count, unique, blanks, nonblanks)
 * for columns that have a summary type configured.
 *
 * The summary row includes:
 *   - A separator line (horizontal rule with column junctions)
 *   - A data row with summary values, formatted and padded like regular cells
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tables_render_summaries.h"
#include "tables_render_utils.h"

/*
 * Render the summaries row if any summaries are defined
 * Checks all columns for summary types, and if any are found:
 *   1. Renders a separator line (horizontal rule)
 *   2. Renders each column's summary value with proper formatting
 *
 * Summary values are formatted based on data type and summary type,
 * with clipping and justification matching regular data cells.
 *
 * Parameters:
 *   config: Table configuration (columns, summary types, theme, etc.)
 *   data:   Table data with pre-calculated summary statistics
 */
void render_summaries(TableConfig *config, TableData *data) {
    /* Check if there are any summaries to render */
    int has_summaries = 0;
    /* Scan all columns for summary type */
    for (int j = 0; j < config->column_count; j++) {
        if (config->columns[j].summary != SUMMARY_NONE) {
            has_summaries = 1;
            break;
        }
    }
    /* Exit early if no summaries are configured */
    if (!has_summaries) return;

    /* Render summary separator line (horizontal rule between data and summaries) */
    printf("%s", config->theme.border_color);
    printf("%s", config->theme.l_junct);
    /* Iterate through all columns to render horizontal line characters */
    for (int j = 0; j < config->column_count; j++) {
        if (!config->columns[j].visible) continue;
        /* Print horizontal line characters for this column's width */
        for (int w = 0; w < config->columns[j].width; w++) {
            printf("%s", config->theme.h_line);
        }
        /* Check if next visible column exists for cross junction */
        int next_visible = 0;
        for (int k = j + 1; k < config->column_count; k++) {
            if (config->columns[k].visible) {
                next_visible = 1;
                break;
            }
        }
        /* Print cross junction at column separator positions */
        if (next_visible) {
            printf("%s", config->theme.cross);
        }
    }
    /* End the separator line */
    printf("%s%s\n", config->theme.r_junct, config->theme.text_color);

    /* Render summary row (match Bash: border v_line then reset) */
    printf("%s%s%s", config->theme.border_color, config->theme.v_line, config->theme.text_color);
    /* Iterate through all columns to render each summary value */
    for (int j = 0; j < config->column_count; j++) {
        if (!config->columns[j].visible) continue;
        ColumnConfig *col = &config->columns[j];
        SummaryStats *stats = &data->summaries[j];
        /* Buffer for building the summary text */
        char summary_text[256] = {0};
        /* Switch on summary type to determine what calculation to display */
        switch (col->summary) {
            case SUMMARY_SUM:
                /* Sum: only show if sum is not zero */
                if (stats->sum != 0.0) {
                    if (col->data_type == DATA_KCPU) {
                        /* CPU sum: format as millicores with "m" suffix */
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->sum);
                        char *formatted = format_with_commas(summary_text);
                        snprintf(summary_text, sizeof(summary_text), "%sm", formatted);
                        free(formatted);
                    } else if (col->data_type == DATA_KMEM) {
                        /* Memory sum: format with "M" suffix */
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
                        /* Num sum: format with comma separators */
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
                } else {
                    /* Zero sum: display empty string */
                    summary_text[0] = '\0';
                }
                break;
            case SUMMARY_MIN:
                /* Min: only show if min has been initialized (count > 0) */
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
                    } else if (col->data_type == DATA_INT) {
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->min);
                    } else if (col->data_type == DATA_NUM) {
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
                    /* No data: empty string */
                    summary_text[0] = '\0';
                }
                break;
            case SUMMARY_MAX:
                /* Max: only show if max has been initialized (count > 0) */
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
                    } else if (col->data_type == DATA_INT) {
                        snprintf(summary_text, sizeof(summary_text), "%.0f", stats->max);
                    } else if (col->data_type == DATA_NUM) {
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
                    /* No data: empty string */
                    summary_text[0] = '\0';
                }
                break;
            case SUMMARY_AVG:
                /* Avg: calculate average from sum/count, only if count > 0 */
                if (stats->avg_count > 0) {
                    /* Calculate average */
                    double avg_result = stats->avg_sum / stats->avg_count;
                    /* Only show if average is not zero */
                    if (avg_result != 0.0) {
                        if (col->data_type == DATA_FLOAT) {
                            char format[16];
                            snprintf(format, sizeof(format), "%%.%df", stats->max_decimal_places);
                            /* Explicit round-half-up, epsilon-guarded against binary
                             * floating-point representation noise, so the result does
                             * not depend on which side of a tie the raw double lands on. */
                            double scale = pow(10, stats->max_decimal_places);
                            double rounded_avg = floor(avg_result * scale + 0.5 + 1e-9) / scale;
                            snprintf(summary_text, sizeof(summary_text), format, rounded_avg);
                            char *formatted = format_with_commas(summary_text);
                            strncpy(summary_text, formatted, sizeof(summary_text) - 1);
                            summary_text[sizeof(summary_text) - 1] = '\0';
                            free(formatted);
                        } else if (col->data_type == DATA_INT) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", floor(avg_result + 0.5 + 1e-9));
                        } else if (col->data_type == DATA_NUM) {
                            snprintf(summary_text, sizeof(summary_text), "%.0f", floor(avg_result + 0.5 + 1e-9));
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
                        /* Zero average: empty string */
                        summary_text[0] = '\0';
                    }
                } else {
                    /* No data: display "N/A" */
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
        /* Duplicate the summary text for display processing */
        char *summary_display = strdup_safe(summary_text);
        /* Calculate display width of the summary text */
        int summary_width = get_display_width(summary_display);
        /* Effective width accounts for minimum padding */
        int effective_width = col->width - 1; /* Account for minimum padding, let rendering handle the rest */
        /* Clip summary text if it exceeds the column width (in clip mode) */
        if (summary_width > effective_width && col->wrap_mode == WRAP_CLIP) {
            char *truncated = malloc(col->width + 1);
            if (truncated) {
                int k = 0, display_count = 0;
                /* Track ANSI escape sequences during truncation */
                int in_ansi = 0;
                const char *start_p = summary_display;
                const char *end_p = summary_display + strlen(summary_display) - 1;

                if (col->justify == JUSTIFY_RIGHT) {
                    /* Right-justified: keep the end of the text */
                    int target_count = effective_width;
                    /* Scan backwards to find the starting position */
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
                    /* Copy from start_p to end */
                    for (const char *p = start_p; *p; p++) {
                        truncated[k++] = *p;
                    }
                } else if (col->justify == JUSTIFY_CENTER) {
                    /* Center-justified: keep the middle portion */
                    int total_excess = summary_width - effective_width;
                    int left_excess = total_excess / 2;
                    int right_excess = total_excess - left_excess;
                    const char *left_cut = summary_display;
                    const char *right_cut = end_p;
                    int left_count = 0, right_count = 0;

                    /* Scan left to find left cut point */
                    for (const char *p = summary_display; *p && left_count < left_excess; p++) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) left_count++;
                        left_cut = p;
                    }
                    /* Reset and scan right to find right cut point */
                    in_ansi = 0;
                    for (const char *p = end_p; p >= summary_display && right_count < right_excess; p--) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) right_count++;
                        right_cut = p;
                    }
                    /* Copy from left_cut to right_cut */
                    for (const char *p = left_cut; p <= right_cut && *p; p++) {
                        truncated[k++] = *p;
                    }
                } else {
                    /* Left-justified (default): keep the beginning of the text */
                    for (const char *p = summary_display; *p && display_count < effective_width; p++) {
                        if (*p == '\033') in_ansi = 1;
                        else if (in_ansi && *p == 'm') in_ansi = 0;
                        else if (!in_ansi) display_count++;
                        truncated[k++] = *p;
                    }
                }
                /* Null-terminate the truncated string */
                truncated[k] = '\0';
                /* Replace summary_display with the truncated version */
                free(summary_display);
                summary_display = truncated;
                summary_width = get_display_width(summary_display);
            }
        }
        /* Calculate padding to fill the column width */
        int total_padding = col->width - summary_width;
        /* Default: 1 space padding on each side */
        int padding_left = 1; /* Exactly one space padding on left */
        int padding_right = 1; /* Exactly one space padding on right */
        if (total_padding > 2) {
            /* If more space is available, adjust based on justification */
            int remaining_padding = total_padding - 2;
            if (col->justify == JUSTIFY_RIGHT) {
                /* Right: all extra space goes to the left */
                padding_left += remaining_padding;
            } else if (col->justify == JUSTIFY_CENTER) {
                /* Center: split extra space between left and right */
                padding_left += remaining_padding / 2;
                padding_right += remaining_padding - (remaining_padding / 2);
            } else {
                /* Left: all extra space goes to the right */
                padding_right += remaining_padding;
            }
        }
        /* Match Bash render_cell: pad outside color, reset after content, then border */
        printf("%*s%s%s%s%*s%s%s%s",
               padding_left, "",
               config->theme.summary_color, summary_display, config->theme.text_color,
               padding_right, "",
               config->theme.border_color, config->theme.v_line, config->theme.text_color);
        free(summary_display);
    }
    /* End the summary row with a newline */
    printf("\n");
}
