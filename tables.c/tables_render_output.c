/*
 * tables_render_output.c - Main rendering logic for outputting formatted tables
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_render_output.h"
#include "tables_render_title.h"
#include "tables_render_headers.h"
#include "tables_render_rows.h"
#include "tables_render_summaries.h"
#include "tables_render_footer.h"
#include "tables_render_layout.h"
#include "tables_render_utils.h"

/*
 * Main function to render the entire table
 */
extern int debug_layout;
void render_table(TableConfig *config, TableData *data) {
    // Calculate column widths based on content
    calculate_column_widths(config, data);
    
    // Calculate total width of the table
    int total_width = calculate_total_width(config);
    if (debug_layout) {
        fprintf(stderr, "Debug Layout: Total table width = %d\n", total_width);
        fprintf(stderr, "Debug Layout: Column widths:\n");
        int calculated_total = 0;
        for (int j = 0; j < config->column_count; j++) {
            if (config->columns[j].visible) {
                int col_width = config->columns[j].width;
                if (col_width == 0) {
                    // If width is not set, provide a note
                    fprintf(stderr, "Debug Layout:   Column %d (%s): width = %d (not explicitly set, may be auto-calculated)\n", 
                            j, config->columns[j].header ? config->columns[j].header : "Unnamed", 
                            col_width);
                } else {
                    fprintf(stderr, "Debug Layout:   Column %d (%s): width = %d\n", 
                            j, config->columns[j].header ? config->columns[j].header : "Unnamed", 
                            col_width);
                }
                calculated_total += col_width;
            }
        }
        fprintf(stderr, "Debug Layout: Vertical line positions:\n");
        int cumulative_width = 0;
        fprintf(stderr, "Debug Layout:   Position 0 (left border)\n");
        for (int j = 0; j < config->column_count; j++) {
            if (config->columns[j].visible) {
                cumulative_width += config->columns[j].width;
                fprintf(stderr, "Debug Layout:   Position %d (after column %d)\n", 
                        cumulative_width, j);
            }
        }
        fprintf(stderr, "Debug Layout: Note: Total width from columns = %d (may include inter-column separators in rendering)\n", calculated_total);
    }
    
    // Check if title is present for rendering
    int title_present = (config->title && strlen(config->title) > 0);
    int title_width = 0;
    int box_width = 0;
    int title_padding = 0;

    if (title_present) {
        // Process the title the same way as in render_title() to get accurate width
        char *evaluated_title = evaluate_dynamic_string(config->title);
        if (evaluated_title == NULL) {
            evaluated_title = strdup(config->title ? config->title : "");
        }
        
        char *processed_title = replace_color_placeholders(evaluated_title);
        if (processed_title == NULL) {
            processed_title = strdup(evaluated_title);
        }
        free(evaluated_title);
        
        title_width = get_display_width(processed_title);
        box_width = title_width + 4; // Add padding for box borders and internal padding
        
        // Apply the same clipping logic as in render_title()
        if (config->title_pos == POSITION_FULL) {
            box_width = total_width;
            title_padding = 0;
        } else if (config->title_pos == POSITION_NONE) {
            // No clipping for POSITION_NONE
            title_padding = 0;
        } else {
            // For positioned titles (left, center, right), if the title is longer than table width,
            // clip it to fit within the table width
            if (title_width + 4 > total_width) {
                box_width = total_width;
                title_padding = 0; // When clipped to table width, no padding
            } else {
                // Normal positioning when title fits
                if (config->title_pos == POSITION_CENTER) {
                    title_padding = (total_width - box_width) / 2;
                } else if (config->title_pos == POSITION_RIGHT) {
                    title_padding = total_width - box_width;
                } else { // POSITION_LEFT
                    title_padding = 0;
                }
            }
        }
        
        free(processed_title);
    }

    // Render title if present
    render_title(config, total_width);

    // Render top border (integrating with title if present)
    render_top_border_with_title(config, total_width, title_present, title_padding, box_width);

    // Render headers for visible columns
    render_headers(config);
    render_header_separator(config);

    // Render data rows
    render_rows(config, data);

    // Render summaries if any
    render_summaries(config, data);

    // Render bottom border if no footer is present
    int footer_present = (config->footer && strlen(config->footer) > 0);
    if (!footer_present) {
        printf("%s", config->theme.border_color);
        printf("%s", config->theme.bl_corner);
        int current_pos = 1;
        for (int j = 0; j < config->column_count; j++) {
            if (!config->columns[j].visible) continue;
            for (int w = 0; w < config->columns[j].width; w++) {
                printf("%s", config->theme.h_line);
                current_pos++;
            }
            if (j < config->column_count - 1) {
                int next_col_visible = 0;
                for (int k = j + 1; k < config->column_count; k++) {
                    if (config->columns[k].visible) {
                        next_col_visible = 1;
                        break;
                    }
                }
                if (next_col_visible) {
                    printf("%s", config->theme.b_junct);
                    current_pos++;
                }
            }
        }
        printf("%s", config->theme.br_corner);
        printf("%s\n", config->theme.text_color);
    }

    // Render footer if present
    render_footer(config, total_width);
}
