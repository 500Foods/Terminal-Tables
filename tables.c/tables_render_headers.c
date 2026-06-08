/*
 * tables_render_headers.c - Functions for rendering table headers and separators
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_headers.h"
#include "tables_render_utils.h"

/*
 * Render the table headers with proper alignment and padding
 */
void render_headers(TableConfig *config) {
    printf("%s%s", config->theme.border_color, config->theme.v_line);
    for (int j = 0; j < config->column_count; j++) {
        if (!config->columns[j].visible) continue;
        ColumnConfig *col = &config->columns[j];
        char *header = strdup_safe(col->header ? col->header : "");
        int header_width = get_display_width(header);
        int max_header_width = col->width - 2; // Account for minimum 1 space padding on each side
        char *display_header = header;
        if (header_width > max_header_width && max_header_width > 0) {
            display_header = malloc(max_header_width + 1);
            if (display_header) {
                if (col->justify == JUSTIFY_RIGHT) {
                    // For right alignment, clip from the left
                    int start_pos = header_width - max_header_width;
                    strncpy(display_header, header + start_pos, max_header_width);
                } else if (col->justify == JUSTIFY_CENTER) {
                    // For center alignment, clip from both sides
                    int clip_each_side = (header_width - max_header_width) / 2;
                    int start_pos = clip_each_side;
                    strncpy(display_header, header + start_pos, max_header_width);
                } else {
                    // For left alignment, clip from the right
                    strncpy(display_header, header, max_header_width);
                }
                display_header[max_header_width] = '\0';
            }
            header_width = get_display_width(display_header);
        }
        int total_padding = col->width - header_width;
        int padding_left = 1;  // Minimum 1 space padding on left
        int padding_right = 1; // Minimum 1 space padding on right
        int remaining_padding = total_padding - 2; // Account for minimum padding
        if (remaining_padding > 0) {
            if (col->justify == JUSTIFY_RIGHT) {
                padding_left += remaining_padding;
            } else if (col->justify == JUSTIFY_CENTER) {
                padding_left += remaining_padding / 2;
                padding_right += remaining_padding - (remaining_padding / 2);
            } else {
                padding_right += remaining_padding;
            }
        }
        printf("%s%*s%s%*s", config->theme.caption_color, padding_left, "", display_header, padding_right, "");
        if (display_header != header) {
            free(display_header);
        }
        free(header);
        printf("%s%s", config->theme.border_color, config->theme.v_line);
    }
    printf("%s\n", config->theme.text_color);
}

/*
 * Render the separator line below the headers
 */
void render_header_separator(TableConfig *config) {
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
}
