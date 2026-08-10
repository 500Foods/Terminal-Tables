/*
 * tables_render_footer.h - Header file for footer rendering functions used in table rendering
 *
 * Declares functions for rendering the footer box and the integrated bottom border.
 * The footer is rendered below the table data, optionally with a separator line
 * that connects to the table's bottom border.
 */

#ifndef TABLES_RENDER_FOOTER_H
#define TABLES_RENDER_FOOTER_H

#include "tables_config.h"

/*
 * Render the footer box with proper borders and positioning
 * Processes dynamic commands and color placeholders in the footer text,
 * clips if necessary, and positions the box according to footer_pos.
 * Also renders the bottom border of the footer box.
 *
 * Parameters:
 *   config:      Table configuration containing theme and footer settings
 *   total_width: Total width of the table (including borders)
 */
void render_footer(TableConfig *config, int total_width);

/*
 * Render the bottom border of the table, integrating with footer's top border if present
 * When footer_present is true, the border connects to the footer box at the
 * appropriate position. When false, a standard bottom border is rendered.
 *
 * Parameters:
 *   config:          Table configuration containing theme and column settings
 *   total_width:     Total width of the table (including borders)
 *   footer_present:  Whether a footer is being rendered (1) or not (0)
 *   footer_padding:  Left padding for the footer box (position offset)
 *   box_width:       Width of the footer box (including its borders)
 */
void render_bottom_border_with_footer(TableConfig *config, int total_width, int footer_present, int footer_padding, int box_width);

#endif /* TABLES_RENDER_FOOTER_H */
