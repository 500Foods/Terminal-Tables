/*
 * tables_render_title.c - Functions for rendering the title box of a table
 *
 * This file implements:
 *   - Title box rendering with proper borders and positioning
 *   - Top border rendering integrated with the title box
 *
 * Titles are optional and rendered above the table. They can be positioned
 * as left, center, right, full (spanning table width), or none.
 * When a title is present and positioned, the top border of the table is
 * modified to flow into the title box's bottom border, creating a visually
 * connected appearance.
 *
 * Title text can contain:
 *   - Dynamic commands in $(...) syntax (e.g., $(date))
 *   - Color placeholders like {RED}, {BLUE}, {NC}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_title.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"

/*
 * Render the title box with proper borders and positioning
 * This function:
 *   1. Evaluates dynamic commands in the title text
 *   2. Replaces color placeholders with ANSI escape codes
 *   3. Clips the title text if it exceeds available width
 *   4. Positions the title box (left, center, right, full)
 *   5. Renders the title box's top border
 *   6. Renders the title text line with borders
 *   7. Frees allocated memory
 *
 * Parameters:
 *   config:      Table configuration containing theme and title settings
 *   total_width: Total width of the table (including borders)
 */
void render_title(TableConfig *config, int total_width) {
    /* Check if a title is configured and non-empty */
    int title_present = (config->title && strlen(config->title) > 0);
    /* Exit early if no title to render */
    if (!title_present) return;

    /* Evaluate dynamic commands (e.g., $(date), $(jq ...)) in the title text */
    char *evaluated_title = evaluate_dynamic_string(config->title);
    if (evaluated_title == NULL) {
        /* Fallback: use the raw title text if evaluation fails */
        fprintf(stderr, "Error: Failed to evaluate dynamic title string\n");
        evaluated_title = strdup(config->title ? config->title : "");
        if (evaluated_title == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for title string\n");
            return;
        }
    }

    /* Replace color placeholders ({RED}, {BLUE}, etc.) with ANSI escape codes */
    char *processed_title = replace_color_placeholders(evaluated_title);
    if (processed_title == NULL) {
        /* Fallback: use the evaluated title if color processing fails */
        fprintf(stderr, "Error: Failed to process color placeholders in title\n");
        processed_title = strdup(evaluated_title);
        if (processed_title == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for processed title string\n");
            free(evaluated_title);
            return;
        }
    }
    /* Free the evaluated string now that we have the processed version */
    free(evaluated_title);

    /* Set up display variables for width calculations */
    char *display_title = processed_title;
    int title_width = get_display_width(display_title);
    /* Box width includes 2 padding spaces on each side inside borders */
    int box_width = title_width + 4;

    /* Calculate maximum allowable title width within the table */
    int max_title_width = 0;
    if (config->title_pos == POSITION_FULL) {
        /* Full position: title box spans entire table width */
        max_title_width = total_width > 4 ? total_width - 4 : 0;
    } else {
        /* Other positions: same max width calculation */
        max_title_width = total_width > 4 ? total_width - 4 : 0;
    }

    /* Only clip titles for positioned titles (left, center, right) when they exceed table width.
       Default/none positioned titles can extend beyond table width. */
    if (box_width > total_width && (config->title_pos == POSITION_LEFT ||
        config->title_pos == POSITION_CENTER || config->title_pos == POSITION_RIGHT)) {
        /* Clip the title text to fit within the maximum width */
        char *clipped_title = clip_text_to_width(display_title, max_title_width);
        if (clipped_title) {
            /* Free the previous display_title if it was a previously clipped version */
            if (display_title != processed_title) {
                free(display_title);
            }
            /* Use the clipped version as the new display title */
            display_title = clipped_title;
        }
    }

    /* Recalculate title dimensions after potential clipping */
    title_width = get_display_width(display_title);
    if (config->title_pos == POSITION_FULL) {
        /* Full position: box spans entire table width */
        box_width = total_width;
    } else if (config->title_pos == POSITION_NONE) {
        /* None position: box width based on content, may extend beyond table */
        box_width = title_width + 4;
    } else {
        /* For positioned titles (left, center, right), if the title is longer than table width,
           clip it to fit within the table width */
        if (title_width + 4 > total_width) {
            /* Need to clip the title further to fit within table width */
            char *further_clipped = clip_text_to_width(display_title, total_width - 4);
            if (further_clipped) {
                if (display_title != processed_title) {
                    free(display_title);
                }
                display_title = further_clipped;
                title_width = get_display_width(display_title);
            }
            /* Set box width to table width when clipped */
            box_width = total_width;
        } else {
            /* Normal positioning when title fits within table width */
            box_width = title_width + 4;
        }
    }

    /* Calculate left padding for title positioning */
    int title_padding = 0;
    if (config->title_pos == POSITION_CENTER) {
        /* Center: distribute remaining space evenly */
        title_padding = (total_width - box_width) / 2;
    } else if (config->title_pos == POSITION_RIGHT) {
        /* Right: push title to the right edge */
        title_padding = total_width - box_width;
    }

    /* Match Bash: uncolored leading padding, then border-colored box */
    /* Print leading spaces (uncolored) for title positioning */
    if (title_padding > 0) {
        printf("%*s", title_padding, "");
    }
    /* Print the top border of the title box */
    printf("%s%s", config->theme.border_color, config->theme.tl_corner);
    /* Horizontal line characters for the top border */
    for (int i = 0; i < box_width - 2; i++) {
        printf("%s", config->theme.h_line);
    }
    /* Top-right corner of the title box */
    printf("%s%s\n", config->theme.tr_corner, config->theme.text_color);

    /* Print the title text line */
    /* Print leading spaces (uncolored) for title positioning */
    if (title_padding > 0) {
        printf("%*s", title_padding, "");
    }
    /* Border color, vertical line, then reset to text color for content */
    printf("%s%s%s", config->theme.border_color, config->theme.v_line, config->theme.text_color);
    /* Available width for text inside the box (minus 2 for borders) */
    int available_width = box_width - 2;
    /* Clip the title text to fit within available width, respecting position */
    char *clipped_text = clip_text(display_title, available_width, config->title_pos);

    /* Calculate display width and padding for the clipped text */
    int text_width = get_display_width(clipped_text);
    /* Default: 1 space padding on each side */
    int left_padding = 1;
    int right_padding = 1;

    if (config->title_pos == POSITION_FULL) {
        /* For full position, center the text within the available width */
        /* Ensure at least 1 space padding on each side */
        int effective_text_width = available_width - 2; /* Reserve space for padding */
        if (text_width > effective_text_width) {
            /* Need to re-clip the text to leave room for padding */
            free(clipped_text);
            clipped_text = clip_text_to_width(display_title, effective_text_width);
            text_width = get_display_width(clipped_text);
        }
        /* Calculate centering spaces */
        int spaces = (available_width - text_width - 2) / 2; /* -2 for minimum padding */
        left_padding = 1 + spaces;  /* At least 1 space + centering */
        right_padding = available_width - text_width - left_padding;
    }

    /* Print the title text with color, padding, and closing border */
    printf("%*s%s%s%s%*s%s%s%s\n", left_padding, "", config->theme.header_color, clipped_text, config->theme.text_color, right_padding, "", config->theme.border_color, config->theme.v_line, config->theme.text_color);

    /* Free the clipped text buffer */
    free(clipped_text);
    /* Free the display_title if it was a further clipped version */
    if (display_title != processed_title) {
        free(display_title);
    }
    /* Free the processed title string */
    free(processed_title);
}

/*
 * Render the top border of the table, integrating with title's bottom border if present
 * When a title is present, the top border connects to the title box's bottom border.
 * When no title is present, it renders a standard top border.
 *
 * Parameters:
 *   config:         Table configuration containing theme and column settings
 *   total_width:    Total width of the table (including borders)
 *   title_present:  Whether a title box is being rendered (1) or not (0)
 *   title_padding:  Left padding for the title box (position offset)
 *   box_width:      Width of the title box (including its borders)
 */
void render_top_border_with_title(TableConfig *config, int total_width, int title_present, int title_padding, int box_width) {
    /* Print border color for the top border line */
    printf("%s", config->theme.border_color);

    /* Pre-compute column junction positions for alignment with data columns */
    int *column_positions = malloc((config->column_count - 1) * sizeof(int));
    int col_pos_count = 0;
    if (column_positions) {
        /* Calculate cumulative column widths to find junction points */
        int col_width_sum = 0;
        /* Iterate through all columns except the last */
        for (int j = 0; j < config->column_count - 1; j++) {
            /* Skip non-visible columns */
            if (!config->columns[j].visible) continue;
            /* Add this column's width to the running total */
            col_width_sum += config->columns[j].width;
            /* Check if the next column is visible */
            int next_visible = 0;
            for (int k = j + 1; k < config->column_count; k++) {
                if (config->columns[k].visible) {
                    next_visible = 1;
                    break;
                }
            }
            /* If next column is visible, record this junction position */
            if (next_visible) {
                column_positions[col_pos_count++] = col_width_sum;
                /* Add 1 for the vertical separator character */
                col_width_sum++;
            }
        }
    }

    if (title_present) {
        /* Title is present: the top border must integrate with the title box's bottom border */
        int title_start = title_padding;
        int title_end = title_padding + box_width - 1;

        /* For positioned titles (left, center, right), never extend beyond table width.
           Only default/none positioned titles can extend beyond table width. */
        int render_width = total_width;
        if (config->title_pos == POSITION_NONE && title_end >= total_width - 1) {
            /* None position: may extend beyond table width */
            render_width = title_end + 1;
        }

        /* Iterate through each position in the top border */
        for (int i = 0; i < render_width; i++) {
            /* Check if this position aligns with a column junction */
            int is_col_junct = 0;
            if (column_positions && i < total_width) {
                for (int k = 0; k < col_pos_count; k++) {
                    if (i == column_positions[k] + 1) {
                        is_col_junct = 1;
                        break;
                    }
                }
            }

            if (i == 0) {
                /* Left edge: use left junction if title starts at 0, or top-left corner otherwise */
                /* If title starts at position 0, use l_junct (connects to table) */
                /* If title doesn't start at position 0, use tl_corner (standalone corner) */
                printf("%s", (title_start == 0) ? config->theme.l_junct : config->theme.tl_corner);
            } else if (i == render_width - 1) {
                /* Right edge of the render width */
                if (title_end >= total_width - 1 && render_width == total_width) {
                    /* Title spans full table width: use right junction */
                    printf("%s", config->theme.r_junct);
                } else if (title_end >= total_width - 1) {
                    /* Title extends beyond table: use appropriate corner */
                    printf("%s", is_col_junct ? config->theme.r_junct : config->theme.br_corner);
                } else {
                    /* Title is within table width: use top-right corner */
                    printf("%s", config->theme.tr_corner);
                }
            } else if (i == total_width - 1 && title_end > total_width - 1) {
                /* Junction at table edge when title extends beyond */
                printf("%s", config->theme.t_junct);
            } else if (i == title_start) {
                /* Start of title box */
                /* For positioned titles that have been clipped to table width, use l_junct to connect */
                if ((config->title_pos == POSITION_CENTER || config->title_pos == POSITION_RIGHT) &&
                    box_width == total_width && title_start > 0) {
                    printf("%s", config->theme.l_junct);
                } else {
                    printf("%s", is_col_junct ? config->theme.cross : config->theme.b_junct);
                }
            } else if (i == title_end && title_end < render_width - 1) {
                /* End of title box (not at the render edge) */
                if (i >= total_width - 1) {
                    printf("%s", config->theme.br_corner);
                } else {
                    printf("%s", is_col_junct ? config->theme.cross : config->theme.b_junct);
                }
            } else if (i > title_start && i < title_end) {
                /* Inside the title box region */
                printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
            } else if (i >= total_width && i < title_end) {
                /* Extension beyond table width: just horizontal lines */
                printf("%s", config->theme.h_line);
            } else {
                /* Outside title box: horizontal line or top junction at column crossings */
                printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
            }
        }
    } else {
        /* No title: render a standard top border line */
        printf("%s", config->theme.tl_corner);
        /* Fill horizontal line characters between corners */
        for (int i = 1; i < total_width - 1; i++) {
            /* Check for column junction positions */
            int is_col_junct = 0;
            if (column_positions) {
                for (int k = 0; k < col_pos_count; k++) {
                    if (i == column_positions[k] + 1) {
                        is_col_junct = 1;
                        break;
                    }
                }
            }
            /* Use top junction at column positions, horizontal line elsewhere */
            printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
        }
        /* Right corner */
        printf("%s", config->theme.tr_corner);
    }

    /* Free the column positions array */
    if (column_positions) {
        free(column_positions);
    }
    /* Reset text color and end the line */
    printf("%s\n", config->theme.text_color);
}
