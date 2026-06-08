/*
 * tables_render_footer.h - Header file for footer rendering functions used in table rendering
 */

#ifndef TABLES_RENDER_FOOTER_H
#define TABLES_RENDER_FOOTER_H

#include "tables_config.h"

/*
 * Render the footer box with proper borders and positioning
 */
void render_footer(TableConfig *config, int total_width);

/*
 * Render the bottom border of the table, integrating with footer's top border if present
 */
void render_bottom_border_with_footer(TableConfig *config, int total_width, int footer_present, int footer_padding, int box_width);

#endif /* TABLES_RENDER_FOOTER_H */
