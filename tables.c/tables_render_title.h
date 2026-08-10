/*
 * tables_render_title.h - Header file for title rendering functions used in table rendering
 *
 * Declares functions for rendering the title box and the top border
 * that integrates with the title.
 */

#ifndef TABLES_RENDER_TITLE_H
#define TABLES_RENDER_TITLE_H

#include "tables_config.h"

/*
 * Render the title box with proper borders and positioning
 * The title box appears above the table. It supports dynamic content
 * (e.g., $(date)), color placeholders, and positioning (left, center, right, full).
 * When the title is positioned and wider than the table, it is clipped.
 *
 * Parameters:
 *   config:      Table configuration containing theme and title settings
 *   total_width: Total width of the table (including borders)
 */
void render_title(TableConfig *config, int total_width);

/*
 * Render the top border of the table, integrating with title's bottom border if present
 * When a title is present, the top border connects to the title box's bottom border
 * at the appropriate position. When no title is present, a standard top border
 * with a top-left corner, horizontal lines, and top-right corner is rendered.
 *
 * Parameters:
 *   config:         Table configuration containing theme and column settings
 *   total_width:    Total width of the table (including borders)
 *   title_present:  Whether a title box is being rendered (1) or not (0)
 *   title_padding:  Left padding for the title box (position offset)
 *   box_width:      Width of the title box (including its borders)
 */
void render_top_border_with_title(TableConfig *config, int total_width, int title_present, int title_padding, int box_width);

#endif /* TABLES_RENDER_TITLE_H */
