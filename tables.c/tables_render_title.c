/*
 * tables_render_title.c - Functions for rendering the title box of a table
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_title.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"

/*
 * Render the title box with proper borders and positioning
 */
void render_title(TableConfig *config, int total_width) {
    int title_present = (config->title && strlen(config->title) > 0);
    if (!title_present) return;

    char *evaluated_title = evaluate_dynamic_string(config->title);
    if (evaluated_title == NULL) {
        fprintf(stderr, "Error: Failed to evaluate dynamic title string\n");
        evaluated_title = strdup(config->title ? config->title : "");
        if (evaluated_title == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for title string\n");
            return;
        }
    }

    char *processed_title = replace_color_placeholders(evaluated_title);
    if (processed_title == NULL) {
        fprintf(stderr, "Error: Failed to process color placeholders in title\n");
        processed_title = strdup(evaluated_title);
        if (processed_title == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for processed title string\n");
            free(evaluated_title);
            return;
        }
    }
    free(evaluated_title);

    char *display_title = processed_title;
    int title_width = get_display_width(display_title);
    int box_width = title_width + 4;

    int max_title_width = 0;
    if (config->title_pos == POSITION_FULL) {
        max_title_width = total_width > 4 ? total_width - 4 : 0;
    } else {
        max_title_width = total_width > 4 ? total_width - 4 : 0;
    }

    // Only clip titles for positioned titles (left, center, right) when they exceed table width
    // Default/none positioned titles can extend beyond table width
    if (box_width > total_width && (config->title_pos == POSITION_LEFT ||
        config->title_pos == POSITION_CENTER || config->title_pos == POSITION_RIGHT)) {
        char *clipped_title = clip_text_to_width(display_title, max_title_width);
        if (clipped_title) {
            if (display_title != processed_title) {
                free(display_title);
            }
            display_title = clipped_title;
        }
    }

    title_width = get_display_width(display_title);
    if (config->title_pos == POSITION_FULL) {
        box_width = total_width;
    } else if (config->title_pos == POSITION_NONE) {
        box_width = title_width + 4;
    } else {
        // For positioned titles (left, center, right), if the title is longer than table width,
        // clip it to fit within the table width
        if (title_width + 4 > total_width) {
            // Need to clip the title further to fit within table width
            char *further_clipped = clip_text_to_width(display_title, total_width - 4);
            if (further_clipped) {
                if (display_title != processed_title) {
                    free(display_title);
                }
                display_title = further_clipped;
                title_width = get_display_width(display_title);
            }
            box_width = total_width;
        } else {
            box_width = title_width + 4;
        }
    }

    int title_padding = 0;
    if (config->title_pos == POSITION_CENTER) {
        title_padding = (total_width - box_width) / 2;
    } else if (config->title_pos == POSITION_RIGHT) {
        title_padding = total_width - box_width;
    }

    printf("%s%*s%s", config->theme.border_color, title_padding, "", config->theme.tl_corner);
    for (int i = 0; i < box_width - 2; i++) {
        printf("%s", config->theme.h_line);
    }
    printf("%s%s\n", config->theme.tr_corner, config->theme.text_color);

    printf("%s%*s%s", config->theme.border_color, title_padding, "", config->theme.v_line);
    int available_width = box_width - 2;
    char *clipped_text = clip_text(display_title, available_width, config->title_pos);
    
    int text_width = get_display_width(clipped_text);
    int left_padding = 1;
    int right_padding = 1;

    if (config->title_pos == POSITION_FULL) {
        // For full position, center the text within the available width (excluding borders)
        // Ensure at least 1 space padding on each side
        int effective_text_width = available_width - 2; // Reserve space for padding
        if (text_width > effective_text_width) {
            // Need to re-clip the text to leave room for padding
            free(clipped_text);
            clipped_text = clip_text_to_width(display_title, effective_text_width);
            text_width = get_display_width(clipped_text);
        }
        int spaces = (available_width - text_width - 2) / 2; // -2 for minimum padding
        left_padding = 1 + spaces;  // At least 1 space + centering
        right_padding = available_width - text_width - left_padding;
    }

    printf("%*s%s%s%s%*s%s%s\n", left_padding, "", config->theme.header_color, clipped_text, config->theme.text_color, right_padding, "", config->theme.border_color, config->theme.v_line);
    
    free(clipped_text);
    if (display_title != processed_title) {
        free(display_title);
    }
    free(processed_title);
}

void render_top_border_with_title(TableConfig *config, int total_width, int title_present, int title_padding, int box_width) {
    printf("%s", config->theme.border_color);
    int *column_positions = malloc((config->column_count - 1) * sizeof(int));
    int col_pos_count = 0;
    if (column_positions) {
        int col_width_sum = 0;
        for (int j = 0; j < config->column_count - 1; j++) {
            if (!config->columns[j].visible) continue;
            col_width_sum += config->columns[j].width;
            int next_visible = 0;
            for (int k = j + 1; k < config->column_count; k++) {
                if (config->columns[k].visible) {
                    next_visible = 1;
                    break;
                }
            }
            if (next_visible) {
                column_positions[col_pos_count++] = col_width_sum;
                col_width_sum++;
            }
        }
    }

    if (title_present) {
        int title_start = title_padding;
        int title_end = title_padding + box_width - 1;
        
        // For positioned titles (left, center, right), never extend beyond table width
        // Only default/none positioned titles can extend beyond table width
        int render_width = total_width;
        if (config->title_pos == POSITION_NONE && title_end >= total_width - 1) {
            render_width = title_end + 1;
        }

        for (int i = 0; i < render_width; i++) {
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
                // If title starts at position 0, use l_junct (connects to table)
                // If title doesn't start at position 0, use tl_corner (standalone corner)
                printf("%s", (title_start == 0) ? config->theme.l_junct : config->theme.tl_corner);
            } else if (i == render_width - 1) {
                // If we're at the end of the render width
                if (title_end >= total_width - 1 && render_width == total_width) {
                    // Title spans full table width - use right junction
                    printf("%s", config->theme.r_junct);
                } else if (title_end >= total_width - 1) {
                    // Title extends beyond table width - use appropriate corner
                    printf("%s", is_col_junct ? config->theme.r_junct : config->theme.br_corner);
                } else {
                    // Title is within table width - use top-right corner
                    printf("%s", config->theme.tr_corner);
                }
            } else if (i == title_start) {
                // For positioned titles that have been clipped to table width, use l_junct to connect
                if ((config->title_pos == POSITION_CENTER || config->title_pos == POSITION_RIGHT) &&
                    box_width == total_width && title_start > 0) {
                    printf("%s", config->theme.l_junct);
                } else {
                    printf("%s", is_col_junct ? config->theme.cross : config->theme.b_junct);
                }
            } else if (i == title_end && title_end < render_width - 1) {
                // If title ends before the render width, use appropriate junction
                if (i >= total_width - 1) {
                    printf("%s", config->theme.br_corner);
                } else {
                    printf("%s", is_col_junct ? config->theme.cross : config->theme.b_junct);
                }
            } else if (i == total_width - 1 && title_end > total_width - 1) {
                printf("%s", config->theme.t_junct);  // Junction at table edge when title extends beyond
            } else if (i > title_start && i < title_end) {
                printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
            } else if (i >= total_width && i < title_end) {
                // Extension beyond table width - just horizontal lines
                printf("%s", config->theme.h_line);
            } else {
                printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
            }
        }
    } else {
        printf("%s", config->theme.tl_corner);
        for (int i = 1; i < total_width - 1; i++) {
            int is_col_junct = 0;
            if (column_positions) {
                for (int k = 0; k < col_pos_count; k++) {
                    if (i == column_positions[k] + 1) {
                        is_col_junct = 1;
                        break;
                    }
                }
            }
            printf("%s", is_col_junct ? config->theme.t_junct : config->theme.h_line);
        }
        printf("%s", config->theme.tr_corner);
    }

    if (column_positions) {
        free(column_positions);
    }
    printf("%s\n", config->theme.text_color);
}
