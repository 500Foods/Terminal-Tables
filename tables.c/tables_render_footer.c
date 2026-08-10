/*
 * tables_render_footer.c - Functions for rendering the footer box of a table
 *
 * This file implements footer rendering, including:
 *   - Bottom border integration with footer (title_border_with_footer)
 *   - Footer text box rendering with proper positioning (render_footer)
 *
 * Footers are optional and positioned below the table. They can be positioned
 * as left, center, right, full (spanning table width), or none.
 * When a footer is present, the bottom border of the table is modified to
 * flow into the footer box's top border, creating a visually connected appearance.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_footer.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"

/*
 * Render the bottom border of the table, integrating with footer's top border if present
 * This function replaces the normal bottom border with one that connects to the footer box.
 * When no footer is present, it renders a standard bottom border.
 *
 * Parameters:
 *   config:          Table configuration containing theme and column settings
 *   total_width:     Total width of the table (including borders)
 *   footer_present:  Whether a footer is being rendered (1) or not (0)
 *   footer_padding:  Left padding for the footer box (position offset)
 *   box_width:       Width of the footer box (including its borders)
 */
void render_bottom_border_with_footer(TableConfig *config, int total_width, int footer_present, int footer_padding, int box_width) {
    /* Print the border color for the entire bottom border line */
    printf("%s", config->theme.border_color);

    /* Pre-compute column junction positions for alignment with data columns */
    int *column_positions = malloc((config->column_count - 1) * sizeof(int));
    int col_pos_count = 0;
    if (column_positions) {
        /* Calculate cumulative column widths to find junction points */
        int col_width_sum = 0;
        /* Iterate through all columns except the last (no junction after last column) */
        for (int j = 0; j < config->column_count - 1; j++) {
            /* Skip non-visible columns in the position calculation */
            if (!config->columns[j].visible) continue;
            /* Add this column's width to the running total */
            col_width_sum += config->columns[j].width;
            /* Check if the next column is visible (determines if we need a junction here) */
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
                /* Add 1 for the vertical separator character between columns */
                col_width_sum++;
            }
        }
    }

    /* Render the bottom border line */
    if (footer_present) {
        /* Footer is present: the border must integrate with the footer box */
        int footer_start = footer_padding;
        int footer_end = footer_padding + box_width - 1;
        /* Determine the maximum render width (may extend beyond table if footer is positioned) */
        int max_width = (footer_end >= total_width) ? footer_end + 1 : total_width;

        /* Iterate through each position in the bottom border */
        for (int i = 0; i < max_width; i++) {
            /* Check if this position aligns with a column junction */
            int is_col_junct = 0;
            if (column_positions) {
                for (int k = 0; k < col_pos_count; k++) {
                    if (i == column_positions[k] + 1) {
                        is_col_junct = 1;
                        break;
                    }
                }
            }

            /* Determine the appropriate border character based on position */
            if (i == 0) {
                /* Left edge of the border line */
                printf("%s", (footer_start == 0) ? config->theme.l_junct : config->theme.bl_corner);
            } else if (i == max_width - 1) {
                /* Right edge of the border line */
                if (footer_end > total_width - 1) {
                    /* Footer extends beyond table: use top-right corner of footer box */
                    printf("%s", config->theme.tr_corner);
                } else if (footer_end == total_width - 1) {
                    /* Footer ends exactly at table edge: use right junction */
                    printf("%s", config->theme.r_junct);
                } else {
                    /* Normal bottom-right corner of the table */
                    printf("%s", config->theme.br_corner);
                }
            } else if (i == total_width - 1 && footer_end > total_width - 1) {
                /* Junction at table edge when footer extends beyond */
                printf("%s", config->theme.b_junct);
            } else if (i == footer_start) {
                /* Start of footer box: use cross or T-junction depending on column alignment */
                printf("%s", is_col_junct ? config->theme.cross : config->theme.t_junct);
            } else if (i == footer_end) {
                /* End of footer box: use cross or T-junction */
                printf("%s", is_col_junct ? config->theme.cross : config->theme.t_junct);
            } else if (i > footer_start && i < footer_end) {
                /* Inside the footer box: use bottom junction for column crossings, horizontal line otherwise */
                printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
            } else {
                /* Outside the footer box: use bottom junction for column crossings, horizontal line otherwise */
                printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
            }
        }
    } else {
        /* No footer: render a standard bottom border line */
        printf("%s", config->theme.bl_corner);
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
            /* Use bottom junction at column positions, horizontal line elsewhere */
            printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
        }
        printf("%s", config->theme.br_corner);
    }

    /* Free the column positions array */
    if (column_positions) {
        free(column_positions);
    }
    /* Reset text color and end the line */
    printf("%s\n", config->theme.text_color);
}


/*
 * Render the footer box with proper borders and positioning
 * This function:
 *   1. Evaluates any dynamic commands in the footer text (e.g., $(date))
 *   2. Replaces color placeholders with ANSI codes
 *   3. Clips the footer text if it exceeds available width
 *   4. Positions the footer box (left, center, right, full)
 *   5. Renders the bottom border (integrated with footer)
 *   6. Renders the footer text line
 *   7. Renders the footer's bottom border
 *
 * Parameters:
 *   config:      Table configuration containing theme and footer settings
 *   total_width: Total width of the table (including borders)
 */
void render_footer(TableConfig *config, int total_width) {
    /* Check if a footer is configured and non-empty */
    int footer_present = (config->footer && strlen(config->footer) > 0);
    /* Exit early if no footer to render */
    if (!footer_present) return;

    /* Evaluate dynamic commands (e.g., $(date), $(jq ...)) in the footer text */
    char *evaluated_footer = evaluate_dynamic_string(config->footer);
    if (evaluated_footer == NULL) {
        /* Fallback: use the raw footer text if evaluation fails */
        fprintf(stderr, "Error: Failed to evaluate dynamic footer string\n");
        evaluated_footer = strdup(config->footer ? config->footer : "");
        if (evaluated_footer == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for footer string\n");
            return;
        }
    }

    /* Replace color placeholders ({RED}, {BLUE}, etc.) with ANSI escape codes */
    char *processed_footer = replace_color_placeholders(evaluated_footer);
    if (processed_footer == NULL) {
        /* Fallback: use the evaluated footer if color processing fails */
        fprintf(stderr, "Error: Failed to process color placeholders in footer\n");
        processed_footer = strdup(evaluated_footer);
        if (processed_footer == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for processed footer string\n");
            free(evaluated_footer);
            return;
        }
    }
    /* Free the evaluated string now that we have the processed version */
    free(evaluated_footer);

    /* Set up display variables for width calculations */
    char *display_footer = processed_footer;
    int footer_width = get_display_width(display_footer);
    /* Box width includes 2 padding spaces (1 on each side) inside borders */
    int box_width = footer_width + 4;

    /* Reference to global debug flag */
    extern int debug_mode;
    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Original footer text: '%s'\n", config->footer ? config->footer : "NULL");
        fprintf(stderr, "Debug Footer: Processed footer text: '%s'\n", display_footer);
        fprintf(stderr, "Debug Footer: Footer display width: %d\n", footer_width);
        fprintf(stderr, "Debug Footer: Initial box width: %d\n", box_width);
        fprintf(stderr, "Debug Footer: Total table width: %d\n", total_width);
        fprintf(stderr, "Debug Footer: Footer position: %d\n", config->footer_pos);
    }

    /* Calculate maximum allowable footer width within the table */
    int max_footer_width = 0;
    if (config->footer_pos == POSITION_FULL) {
        /* Full position: footer spans entire table width */
        max_footer_width = total_width > 4 ? total_width - 4 : 0;
    } else {
        /* Other positions: same max width calculation */
        max_footer_width = total_width > 4 ? total_width - 4 : 0;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Max footer width: %d\n", max_footer_width);
    }

    /* If footer box is wider than the table, clip the footer text to fit */
    if (box_width > total_width && config->footer_pos != POSITION_NONE) {
        char *clipped_footer = clip_text(display_footer, max_footer_width, config->footer_pos);
        if (clipped_footer) {
            /* Free the previous display_footer if it was a clipped version */
            if (display_footer != processed_footer) {
                free(display_footer);
            }
            display_footer = clipped_footer;
        }
    }

    /* Recalculate footer dimensions after potential clipping */
    footer_width = get_display_width(display_footer);
    if (config->footer_pos == POSITION_FULL) {
        /* Full position: box spans entire table width */
        box_width = total_width;
    } else if (config->footer_pos == POSITION_NONE) {
        /* None position: box width based on content, may extend beyond table */
        box_width = footer_width + 4;
    } else {
        /* Positioned (left, center, right): box may extend up to table width */
        box_width = footer_width + 4;
        if (box_width > total_width) {
            box_width = total_width;
        }
    }

    /* Calculate left padding for footer positioning */
    int footer_padding = 0;
    if (config->footer_pos == POSITION_CENTER) {
        /* Center: distribute remaining space evenly on both sides */
        footer_padding = (total_width - box_width) / 2;
    } else if (config->footer_pos == POSITION_RIGHT) {
        /* Right: push footer to the right edge */
        footer_padding = total_width - box_width;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Final footer width: %d\n", footer_width);
        fprintf(stderr, "Debug Footer: Final box width: %d\n", box_width);
        fprintf(stderr, "Debug Footer: Footer padding: %d\n", footer_padding);
        fprintf(stderr, "Debug Footer: Expected total width: %d\n", footer_padding + box_width);
    }

    /* Render the bottom border of the table, integrated with footer's top border */
    render_bottom_border_with_footer(config, total_width, footer_present, footer_padding, box_width);

    /* Render the footer text line */
    /* Print leading padding (uncolored, to match Bash output) */
    if (footer_padding > 0) {
        printf("%*s", footer_padding, "");
    }
    /* Print border color, vertical line, then reset to text color */
    printf("%s%s%s", config->theme.border_color, config->theme.v_line, config->theme.text_color);
    /* Calculate available width for text (box width minus 2 for borders) */
    int available_width = box_width - 2;
    /* Clip the footer text to fit within available width, respecting position */
    char *clipped_text = clip_text(display_footer, available_width, config->footer_pos);

    /* Calculate display width of the clipped text for padding calculations */
    int text_width = get_display_width(clipped_text);
    /* Default: 1 space padding on each side */
    int left_padding = 1;
    int right_padding = 1;

    if (config->footer_pos == POSITION_FULL) {
        /* For full position, center the text within the available width */
        /* Ensure at least 1 space padding on each side */
        int effective_text_width = available_width - 2; /* Reserve space for padding */
        if (text_width > effective_text_width) {
            /* Need to re-clip the text to leave room for padding */
            free(clipped_text);
            clipped_text = clip_text_to_width(display_footer, effective_text_width);
            text_width = get_display_width(clipped_text);
        }
        /* Calculate centering spaces */
        int spaces = (available_width - text_width - 2) / 2; /* -2 for minimum padding */
        left_padding = 1 + spaces;  /* At least 1 space + centering */
        right_padding = available_width - text_width - left_padding;
    }

    /* Print the footer text line with color, padding, and borders */
    printf("%*s%s%s%s%*s%s%s%s\n", left_padding, "", config->theme.footer_color, clipped_text, config->theme.text_color, right_padding, "", config->theme.border_color, config->theme.v_line, config->theme.text_color);

    /* Free the clipped text buffer */
    free(clipped_text);

    /* Render the bottom border of the footer box */
    if (footer_padding > 0) {
        printf("%*s", footer_padding, "");
    }
    /* Left corner of footer bottom border */
    printf("%s%s", config->theme.border_color, config->theme.bl_corner);
    /* Horizontal line characters for the footer box width */
    for (int i = 0; i < box_width - 2; i++) {
        printf("%s", config->theme.h_line);
    }
    /* Right corner and color reset */
    printf("%s%s\n", config->theme.br_corner, config->theme.text_color);

    /* Clean up dynamically allocated strings */
    if (display_footer != processed_footer) {
        free(display_footer);
    }
    free(processed_footer);
}
