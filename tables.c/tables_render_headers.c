/*
 * tables_render_headers.c - Functions for rendering table headers and separators
 *
 * This file implements:
 *   - Header row rendering with proper alignment, padding, and justification
 *   - Separator line rendering between headers and data rows
 *
 * Headers are rendered with the caption_color theme setting, and include
 * minimum 1-space padding on each side. Text can be clipped and justified
 * (left, right, center) within the column width.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_headers.h"
#include "tables_render_utils.h"

/*
 * Render the table headers with proper alignment and padding
 * Each column header is:
 *   - Clipped if it exceeds the column width (based on justification)
 *   - Padded to fill the column width
 *   - Rendered with color coding matching the Bash implementation
 */
void render_headers(TableConfig *config) {
    /* Match Bash: border color on v_line, then reset before cell content */
    printf("%s%s%s", config->theme.border_color, config->theme.v_line, config->theme.text_color);
    /* Iterate through all columns */
    for (int j = 0; j < config->column_count; j++) {
        /* Skip non-visible columns */
        if (!config->columns[j].visible) continue;
        ColumnConfig *col = &config->columns[j];
        /* Duplicate the header string for safe manipulation */
        char *header = strdup_safe(col->header ? col->header : "");
        /* Calculate display width of the header text */
        int header_width = get_display_width(header);
        /* Calculate maximum width available for header text (column width minus padding) */
        int max_header_width = col->width - 2; /* Account for minimum 1 space padding on each side */
        /* Pointer to the display header (may point to clipped version or original) */
        char *display_header = header;
        /* If header exceeds available width, clip it based on justification */
        if (header_width > max_header_width && max_header_width > 0) {
            display_header = malloc(max_header_width + 1);
            if (display_header) {
                if (col->justify == JUSTIFY_RIGHT) {
                    /* For right alignment, clip from the left (show end of string) */
                    int start_pos = header_width - max_header_width;
                    strncpy(display_header, header + start_pos, max_header_width);
                } else if (col->justify == JUSTIFY_CENTER) {
                    /* For center alignment, clip from both sides (show middle) */
                    int clip_each_side = (header_width - max_header_width) / 2;
                    int start_pos = clip_each_side;
                    strncpy(display_header, header + start_pos, max_header_width);
                } else {
                    /* For left alignment, clip from the right (show start of string) */
                    strncpy(display_header, header, max_header_width);
                }
                /* Null-terminate the clipped string */
                display_header[max_header_width] = '\0';
            }
            /* Recalculate width after clipping */
            header_width = get_display_width(display_header);
        }
        /* Calculate total padding needed to fill the column */
        int total_padding = col->width - header_width;
        /* Default: 1 space padding on each side */
        int padding_left = 1;  /* Minimum 1 space padding on left */
        int padding_right = 1; /* Minimum 1 space padding on right */
        /* Calculate remaining padding after minimum */
        int remaining_padding = total_padding - 2; /* Account for minimum padding */
        if (remaining_padding > 0) {
            /* Distribute extra padding based on justification */
            if (col->justify == JUSTIFY_RIGHT) {
                /* Right-justified: add all extra space to the left */
                padding_left += remaining_padding;
            } else if (col->justify == JUSTIFY_CENTER) {
                /* Center-justified: split extra space between left and right */
                padding_left += remaining_padding / 2;
                padding_right += remaining_padding - (remaining_padding / 2);
            } else {
                /* Left-justified: add all extra space to the right */
                padding_right += remaining_padding;
            }
        }
        /* Match Bash render_cell: pad outside color, reset after content, then border */
        printf("%*s%s%s%s%*s%s%s%s",
               padding_left, "",
               config->theme.caption_color, display_header, config->theme.text_color,
               padding_right, "",
               config->theme.border_color, config->theme.v_line, config->theme.text_color);
        /* Free the clipped header if it was a different allocation */
        if (display_header != header) {
            free(display_header);
        }
        free(header);
    }
    /* End the header row with a newline */
    printf("\n");
}

/*
 * Render the separator line below the headers
 * This is a horizontal line with junctions at column separators,
 * connecting the left border to the right border.
 */
void render_header_separator(TableConfig *config) {
    /* Set border color for the separator line */
    printf("%s", config->theme.border_color);
    /* Left junction character */
    printf("%s", config->theme.l_junct);
    /* Iterate through all columns */
    for (int j = 0; j < config->column_count; j++) {
        /* Skip non-visible columns */
        if (!config->columns[j].visible) continue;
        /* Print horizontal line characters for this column's width */
        for (int w = 0; w < config->columns[j].width; w++) {
            printf("%s", config->theme.h_line);
        }
        /* Check if there's another visible column after this one */
        int next_visible = 0;
        for (int k = j + 1; k < config->column_count; k++) {
            if (config->columns[k].visible) {
                next_visible = 1;
                break;
            }
        }
        /* Print cross junction if next column is visible */
        if (next_visible) {
            printf("%s", config->theme.cross);
        }
    }
    /* Right junction character and end the line */
    printf("%s%s\n", config->theme.r_junct, config->theme.text_color);
}
