/*
 * tables_render_title.h - Header file for title rendering functions used in table rendering
 */

#ifndef TABLES_RENDER_TITLE_H
#define TABLES_RENDER_TITLE_H

#include "tables_config.h"

/*
 * Render the title box with proper borders and positioning
 */
void render_title(TableConfig *config, int total_width);

/*
 * Render the top border of the table, integrating with title's bottom border if present
 */
void render_top_border_with_title(TableConfig *config, int total_width, int title_present, int title_padding, int box_width);

#endif /* TABLES_RENDER_TITLE_H */
