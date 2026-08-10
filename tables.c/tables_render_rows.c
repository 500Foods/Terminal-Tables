/*
 * tables_render_rows.c - Functions for rendering table data rows with multi-line support and breaking
 *
 * This file implements:
 *   - Data row rendering with text wrapping, clipping, and justification
 *   - Multi-line cell support (cells with wrapped content)
 *   - Break-on-change: visual separator when a designated column's value changes
 *   - Color placeholder replacement for data cell content
 *
 * The rendering matches the Bash reference implementation's behavior:
 *   - Padding is applied outside of color codes
 *   - Text color is reset after cell content
 *   - Vertical borders use the border_color theme setting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_rows.h"
#include "tables_render_utils.h"

/*
 * Render the data rows of the table with support for wrapping, truncation, and breaking
 *
 * This function:
 *   1. Finds the break column (if any) for break-on-change separators
 *   2. Allocates memory for formatted cell values (which may be multi-line)
 *   3. Formats and clips each cell value based on column configuration
 *   4. Renders each row line by line, with vertical borders between cells
 *   5. Inserts break separators when the break column's value changes
 *   6. Frees all allocated memory
 *
 * Parameters:
 *   config: Table configuration (columns, theme, etc.)
 *   data:   Table data with rows to render
 */
void render_rows(TableConfig *config, TableData *data) {
    /* Reference to global debug flag */
    extern int debug_mode;

    /* Find the break column if any (first column with break_on_change set) */
    int break_col = -1;
    for (int j = 0; j < config->column_count; j++) {
        if (config->columns[j].break_on_change) {
            break_col = j;
            break;
        }
    }

    /* Allocate 3D array: [row][column][line] for formatted cell values */
    char ****formatted_values = malloc(data->row_count * sizeof(char ***));
    if (formatted_values == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for formatted_values\n");
        return;
    }
    /* Track line counts for each cell: [row][column] */
    int **line_counts = malloc(data->row_count * sizeof(int *));
    if (line_counts == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for line_counts\n");
        free(formatted_values);
        return;
    }

    /* Initialize formatted_values and line_counts arrays for all rows */
    for (int i = 0; i < data->row_count; i++) {
        /* Allocate array of column pointers for this row */
        formatted_values[i] = malloc(config->column_count * sizeof(char **));
        if (formatted_values[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for formatted_values[%d]\n", i);
            /* Clean up previously allocated rows on error */
            for (int k = 0; k < i; k++) {
                free(formatted_values[k]);
            }
            free(formatted_values);
            free(line_counts);
            return;
        }
        /* Allocate line count array for this row */
        line_counts[i] = malloc(config->column_count * sizeof(int));
        if (line_counts[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for line_counts[%d]\n", i);
            free(formatted_values[i]);
            for (int k = 0; k < i; k++) {
                free(formatted_values[k]);
                free(line_counts[k]);
            }
            free(formatted_values);
            free(line_counts);
            return;
        }
        /* Initialize all column entries to NULL and 0 lines */
        for (int j = 0; j < config->column_count; j++) {
            formatted_values[i][j] = NULL;
            line_counts[i][j] = 0;
        }
    }
    if (debug_mode) {
        fprintf(stderr, "Debug: Allocated memory for formatted_values and line_counts for %d rows and %d columns\n", data->row_count, config->column_count);
    }

    /* Format and clip/wrap text for all cells */
    for (int i = 0; i < data->row_count; i++) {
        DataRow *row = &data->rows[i];
        for (int j = 0; j < config->column_count; j++) {
            /* Skip non-visible columns */
            if (!config->columns[j].visible) continue;
            ColumnConfig *col = &config->columns[j];
            /* Get the raw value from the data row */
            const char *raw_value = row->values[j];
            /* Format the value with type-aware formatting and precision tracking */
            char *formatted = format_display_value_with_precision(raw_value, col->null_val, col->zero_val, col->data_type, col->format, col->string_limit, col->wrap_mode, col->wrap_char, col->justify, data->summaries[j].max_decimal_places);
            /* Determine if clipping or wrapping is needed based on width and mode */
            if (col->width_specified && col->wrap_mode == WRAP_CLIP) {
                /* Clip mode: truncate text that exceeds column width */
                int effective_width = col->width - 2; /* Account for padding on both sides (1 left + 1 right) */
                /* Determine clip position based on justification */
                Position clip_position = POSITION_LEFT;
                if (col->justify == JUSTIFY_RIGHT) {
                    clip_position = POSITION_RIGHT;
                } else if (col->justify == JUSTIFY_CENTER) {
                    clip_position = POSITION_CENTER;
                }

                /* Use color-aware clipping for proper ANSI code handling */
                char *clipped = clip_text_with_colors(formatted, effective_width, clip_position);
                free(formatted);
                formatted = clipped;
                /* Store as single-line value (no wrapping) */
                formatted_values[i][j] = malloc(sizeof(char *));
                formatted_values[i][j][0] = formatted;
                line_counts[i][j] = 1;
            } else if (col->width_specified && col->wrap_mode == WRAP_WRAP) {
                /* Wrap mode: break long text into multiple lines */
                int line_count = 0;
                char **wrapped;
                if (col->wrap_char && strlen(col->wrap_char) > 0) {
                    /* Delimiter-based wrapping: split on the wrap_char */
                    wrapped = wrap_text_delimiter(formatted, col->width - 2, col->wrap_char, &line_count);
                    if (wrapped) {
                        free(formatted);
                        /* Clip each wrapped line if it exceeds the width */
                        for (int l = 0; l < line_count; l++) {
                            int display_width = get_display_width(wrapped[l]);
                            /* Calculate effective width based on justification */
                            int effective_width = (col->justify == JUSTIFY_RIGHT) ? col->width - 1 : col->width - 2;
                            if (display_width > effective_width) {
                                char *truncated = malloc(effective_width + 1);
                                if (truncated) {
                                    int k = 0, display_count = 0;
                                    /* Track ANSI escape sequences during truncation */
                                    int in_ansi = 0;
                                    const char *start_p = wrapped[l];
                                    const char *end_p = wrapped[l] + strlen(wrapped[l]) - 1;

                                    if (col->justify == JUSTIFY_RIGHT) {
                                        /* Right-justified: keep the end of the text */
                                        int target_count = effective_width;
                                        /* Scan backwards to find the starting position */
                                        for (const char *p = end_p; p >= wrapped[l] && target_count > 0; p--) {
                                            if (*p == '\033') in_ansi = 1;
                                            else if (in_ansi && *p == 'm') in_ansi = 0;
                                            else if (!in_ansi) target_count--;
                                            if (target_count <= 0) {
                                                start_p = p + 1;
                                                break;
                                            }
                                        }
                                        if (start_p < wrapped[l]) start_p = wrapped[l];
                                        /* Copy from start_p to end */
                                        for (const char *p = start_p; *p; p++) {
                                            truncated[k++] = *p;
                                        }
                                    } else if (col->justify == JUSTIFY_CENTER) {
                                        /* Center-justified: keep the middle portion */
                                        int total_excess = display_width - effective_width;
                                        int left_excess = total_excess / 2;
                                        int right_excess = total_excess - left_excess;
                                        const char *left_cut = wrapped[l];
                                        const char *right_cut = end_p;
                                        int left_count = 0, right_count = 0;

                                        /* Scan left to find left cut point */
                                        for (const char *p = wrapped[l]; *p && left_count < left_excess; p++) {
                                            if (*p == '\033') in_ansi = 1;
                                            else if (in_ansi && *p == 'm') in_ansi = 0;
                                            else if (!in_ansi) left_count++;
                                            left_cut = p;
                                        }
                                        /* Reset and scan right to find right cut point */
                                        in_ansi = 0;
                                        for (const char *p = end_p; p >= wrapped[l] && right_count < right_excess; p--) {
                                            if (*p == '\033') in_ansi = 1;
                                            else if (in_ansi && *p == 'm') in_ansi = 0;
                                            else if (!in_ansi) right_count++;
                                            right_cut = p;
                                        }
                                        /* Copy from left_cut to right_cut */
                                        if (left_cut < right_cut) {
                                            for (const char *p = left_cut + 1; p <= right_cut && *p; p++) {
                                                if (display_count < effective_width) {
                                                    truncated[k++] = *p;
                                                    if (!in_ansi && *p != '\033') display_count++;
                                                }
                                            }
                                        } else {
                                            for (const char *p = left_cut; p <= right_cut && *p; p++) {
                                                truncated[k++] = *p;
                                            }
                                        }
                                    } else {
                                        /* Left-justified (default), take first 'effective_width' characters */
                                        /* Use same logic as main clipping section (lines 152-158) */
                                        for (const char *p = wrapped[l]; *p && display_count < effective_width; p++) {
                                            if (*p == '\033') in_ansi = 1;
                                            else if (in_ansi && *p == 'm') in_ansi = 0;
                                            else if (!in_ansi) display_count++;
                                            truncated[k++] = *p;
                                        }
                                    }
                                    /* Null-terminate the truncated string */
                                    truncated[k] = '\0';
                                    /* Replace the wrapped line with the truncated version */
                                    free(wrapped[l]);
                                    wrapped[l] = truncated;
                                }
                            }
                        }
                        /* Store the wrapped lines */
                        formatted_values[i][j] = wrapped;
                        line_counts[i][j] = line_count;
                    } else {
                        /* Delimiter wrapping failed: store as single line */
                        formatted_values[i][j] = malloc(sizeof(char *));
                        formatted_values[i][j][0] = formatted;
                        line_counts[i][j] = 1;
                    }
                } else {
                    /* Standard word wrapping without custom delimiter */
                    wrapped = wrap_text(formatted, col->width - 2, &line_count);
                    if (wrapped) {
                        free(formatted);
                        formatted_values[i][j] = wrapped;
                        line_counts[i][j] = line_count;
                    } else {
                        /* Word wrapping failed: store as single line */
                        formatted_values[i][j] = malloc(sizeof(char *));
                        formatted_values[i][j][0] = formatted;
                        line_counts[i][j] = 1;
                    }
                }
            } else {
                /* No width specified or no wrapping needed: store value as single line */
                formatted_values[i][j] = malloc(sizeof(char *));
                formatted_values[i][j][0] = formatted;
                line_counts[i][j] = 1;
            }
        }
    }

    /* Render rows with multi-line support and breaking */
    char *prev_break_value = NULL;
    for (int i = 0; i < data->row_count; i++) {
        /* Check for break-on-change on the designated column */
        if (break_col >= 0 && i > 0) {
            /* Get the current row's break column value */
            char *current_break_value = data->rows[i].values[break_col];
            /* Compare with previous row's break value */
            if (prev_break_value && current_break_value && strcmp(prev_break_value, current_break_value) != 0) {
                /* Value changed: render a break separator line */
                printf("%s", config->theme.border_color);
                printf("%s", config->theme.l_junct);
                /* Render horizontal line across all visible columns */
                for (int j = 0; j < config->column_count; j++) {
                    if (!config->columns[j].visible) continue;
                    for (int w = 0; w < config->columns[j].width; w++) {
                        printf("%s", config->theme.h_line);
                    }
                    /* Check if next visible column exists for junction character */
                    int next_visible = 0;
                    for (int k = j + 1; k < config->column_count; k++) {
                        if (config->columns[k].visible) {
                            next_visible = 1;
                            break;
                        }
                    }
                    if (next_visible) {
                        printf("%s", config->theme.cross);
                    }
                }
                /* End the separator line */
                printf("%s%s\n", config->theme.r_junct, config->theme.text_color);
            }
            /* Update the previous break value */
            prev_break_value = current_break_value;
        } else if (i == 0 && break_col >= 0) {
            /* Initialize the break value from the first row */
            prev_break_value = data->rows[i].values[break_col];
        }

        /* Determine max lines for this row (all columns rendered to same height) */
        int max_lines = 1;
        for (int j = 0; j < config->column_count; j++) {
            if (!config->columns[j].visible) continue;
            if (line_counts[i][j] > max_lines) max_lines = line_counts[i][j];
        }

        /* Render each line of the row */
        for (int line = 0; line < max_lines; line++) {
            /* Match Bash: border on v_line, reset, then cells */
            printf("%s%s%s", config->theme.border_color, config->theme.v_line, config->theme.text_color);
            for (int j = 0; j < config->column_count; j++) {
                /* Skip non-visible columns */
                if (!config->columns[j].visible) continue;
                ColumnConfig *col = &config->columns[j];
                /* Get the text for this line (or empty string if this cell has fewer lines) */
                char *text = (line < line_counts[i][j]) ? formatted_values[i][j][line] : "";
                /* Process color placeholders in data fields */
                char *colored_text = replace_color_placeholders(text);
                /* Calculate display width and padding */
                int value_width = get_display_width(colored_text);
                int total_padding = col->width - value_width;
                /* Default: 1 space padding on each side */
                int padding_left = 1;  /* Minimum 1 space padding on left */
                int padding_right = 1; /* Minimum 1 space padding on right */
                int remaining_padding = total_padding - 2; /* Account for minimum padding */
                if (remaining_padding > 0) {
                    /* Distribute extra padding based on justification */
                    if (col->justify == JUSTIFY_RIGHT) {
                        padding_left += remaining_padding;
                    } else if (col->justify == JUSTIFY_CENTER) {
                        padding_left += remaining_padding / 2;
                        padding_right += remaining_padding - (remaining_padding / 2);
                    } else {
                        padding_right += remaining_padding;
                    }
                }
                /* Match Bash render_cell: pad, text_color+content+reset, pad, border v_line+reset */
                printf("%*s%s%s%s%*s%s%s%s",
                       padding_left, "",
                       config->theme.text_color, colored_text, config->theme.text_color,
                       padding_right, "",
                       config->theme.border_color, config->theme.v_line, config->theme.text_color);
                free(colored_text);
            }
            /* End the row line */
            printf("\n");
        }
    }

    /* Clean up formatted values by freeing all allocated memory */
    for (int i = 0; i < data->row_count; i++) {
        for (int j = 0; j < config->column_count; j++) {
            if (formatted_values[i][j]) {
                free_wrapped_text(formatted_values[i][j], line_counts[i][j]);
            }
        }
        free(formatted_values[i]);
        free(line_counts[i]);
    }
    free(formatted_values);
    free(line_counts);
    if (debug_mode) {
        fprintf(stderr, "Debug: Freed memory for formatted_values and line_counts\n");
    }
}
