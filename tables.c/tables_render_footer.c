/*
 * tables_render_footer.c - Functions for rendering the footer box of a table
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_footer.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"

void render_bottom_border_with_footer(TableConfig *config, int total_width, int footer_present, int footer_padding, int box_width) {
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

    if (footer_present) {
        int footer_start = footer_padding;
        int footer_end = footer_padding + box_width - 1;
        int max_width = (footer_end >= total_width) ? footer_end + 1 : total_width;

        for (int i = 0; i < max_width; i++) {
            int is_col_junct = 0;
            if (column_positions) {
                for (int k = 0; k < col_pos_count; k++) {
                    if (i == column_positions[k] + 1) {
                        is_col_junct = 1;
                        break;
                    }
                }
            }

            if (i == 0) {
                printf("%s", (footer_start == 0) ? config->theme.l_junct : config->theme.bl_corner);
            } else if (i == max_width - 1) {
                if (footer_end > total_width - 1) {
                    printf("%s", config->theme.tr_corner);  // Footer extends beyond table
                } else if (footer_end == total_width - 1) {
                    printf("%s", config->theme.r_junct);    // Footer ends exactly at table edge
                } else {
                    printf("%s", config->theme.br_corner);  // Normal bottom-right corner
                }
            } else if (i == total_width - 1 && footer_end > total_width - 1) {
                printf("%s", config->theme.b_junct);  // Junction at table edge when footer extends beyond
            } else if (i == footer_start) {
                printf("%s", is_col_junct ? config->theme.cross : config->theme.t_junct);
            } else if (i == footer_end) {
                printf("%s", is_col_junct ? config->theme.cross : config->theme.t_junct);
            } else if (i > footer_start && i < footer_end) {
                printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
            } else {
                printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
            }
        }
    } else {
        printf("%s", config->theme.bl_corner);
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
            printf("%s", is_col_junct ? config->theme.b_junct : config->theme.h_line);
        }
        printf("%s", config->theme.br_corner);
    }

    if (column_positions) {
        free(column_positions);
    }
    printf("%s\n", config->theme.text_color);
}


/*
 * Render the footer box with proper borders and positioning
 */
void render_footer(TableConfig *config, int total_width) {
    int footer_present = (config->footer && strlen(config->footer) > 0);
    if (!footer_present) return;

    char *evaluated_footer = evaluate_dynamic_string(config->footer);
    if (evaluated_footer == NULL) {
        fprintf(stderr, "Error: Failed to evaluate dynamic footer string\n");
        evaluated_footer = strdup(config->footer ? config->footer : "");
        if (evaluated_footer == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for footer string\n");
            return;
        }
    }

    char *processed_footer = replace_color_placeholders(evaluated_footer);
    if (processed_footer == NULL) {
        fprintf(stderr, "Error: Failed to process color placeholders in footer\n");
        processed_footer = strdup(evaluated_footer);
        if (processed_footer == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for processed footer string\n");
            free(evaluated_footer);
            return;
        }
    }
    free(evaluated_footer);

    char *display_footer = processed_footer;
    int footer_width = get_display_width(display_footer);
    int box_width = footer_width + 4;

    extern int debug_mode;
    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Original footer text: '%s'\n", config->footer ? config->footer : "NULL");
        fprintf(stderr, "Debug Footer: Processed footer text: '%s'\n", display_footer);
        fprintf(stderr, "Debug Footer: Footer display width: %d\n", footer_width);
        fprintf(stderr, "Debug Footer: Initial box width: %d\n", box_width);
        fprintf(stderr, "Debug Footer: Total table width: %d\n", total_width);
        fprintf(stderr, "Debug Footer: Footer position: %d\n", config->footer_pos);
    }

    int max_footer_width = 0;
    if (config->footer_pos == POSITION_FULL) {
        max_footer_width = total_width > 4 ? total_width - 4 : 0;
    } else {
        max_footer_width = total_width > 4 ? total_width - 4 : 0;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Max footer width: %d\n", max_footer_width);
    }

    if (box_width > total_width && config->footer_pos != POSITION_NONE) {
        char *clipped_footer = clip_text(display_footer, max_footer_width, config->footer_pos);
        if (clipped_footer) {
            if (display_footer != processed_footer) {
                free(display_footer);
            }
            display_footer = clipped_footer;
        }
    }

    footer_width = get_display_width(display_footer);
    if (config->footer_pos == POSITION_FULL) {
        box_width = total_width;
    } else if (config->footer_pos == POSITION_NONE) {
        box_width = footer_width + 4;
    } else {
        box_width = footer_width + 4;
        if (box_width > total_width) {
            box_width = total_width;
        }
    }

    int footer_padding = 0;
    if (config->footer_pos == POSITION_CENTER) {
        footer_padding = (total_width - box_width) / 2;
    } else if (config->footer_pos == POSITION_RIGHT) {
        footer_padding = total_width - box_width;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug Footer: Final footer width: %d\n", footer_width);
        fprintf(stderr, "Debug Footer: Final box width: %d\n", box_width);
        fprintf(stderr, "Debug Footer: Footer padding: %d\n", footer_padding);
        fprintf(stderr, "Debug Footer: Expected total width: %d\n", footer_padding + box_width);
    }

    // Bottom border of table integrating with footer if present
    render_bottom_border_with_footer(config, total_width, footer_present, footer_padding, box_width);

    // Footer text
    printf("%s%*s%s", config->theme.border_color, footer_padding, "", config->theme.v_line);
    int available_width = box_width - 2;
    char *clipped_text = clip_text(display_footer, available_width, config->footer_pos);
    
    int text_width = get_display_width(clipped_text);
    int left_padding = 1;
    int right_padding = 1;

    if (config->footer_pos == POSITION_FULL) {
        // For full position, center the text within the available width (excluding borders)
        // Ensure at least 1 space padding on each side
        int effective_text_width = available_width - 2; // Reserve space for padding
        if (text_width > effective_text_width) {
            // Need to re-clip the text to leave room for padding
            free(clipped_text);
            clipped_text = clip_text_to_width(display_footer, effective_text_width);
            text_width = get_display_width(clipped_text);
        }
        int spaces = (available_width - text_width - 2) / 2; // -2 for minimum padding
        left_padding = 1 + spaces;  // At least 1 space + centering
        right_padding = available_width - text_width - left_padding;
    }

    printf("%*s%s%s%s%*s%s%s\n", left_padding, "", config->theme.footer_color, clipped_text, config->theme.text_color, right_padding, "", config->theme.border_color, config->theme.v_line);
    
    free(clipped_text);

    // Bottom border of footer box
    printf("%s%*s%s", config->theme.border_color, footer_padding, "", config->theme.bl_corner);
    for (int i = 0; i < box_width - 2; i++) {
        printf("%s", config->theme.h_line);
    }
    printf("%s%s\n", config->theme.br_corner, config->theme.text_color);

    if (display_footer != processed_footer) {
        free(display_footer);
    }
    free(processed_footer);
}
