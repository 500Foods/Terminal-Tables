/*
 * tables_render_headers.h - Header file for header rendering functions used in table rendering
 *
 * Declares functions for rendering the column headers row and the separator
 * line that appears below the headers.
 */

#ifndef TABLES_RENDER_HEADERS_H
#define TABLES_RENDER_HEADERS_H

#include "tables_config.h"

/*
 * Render the table headers with proper alignment and padding
 * Each column header is clipped if needed, padded to fill the column width,
 * and rendered with caption_color. Color placeholders are not processed
 * in headers (only in data cells).
 */
void render_headers(TableConfig *config);

/*
 * Render the separator line below the headers
 * A horizontal line with cross junctions at column separators,
 * connecting the left border (l_junct) to the right border (r_junct).
 */
void render_header_separator(TableConfig *config);

#endif /* TABLES_RENDER_HEADERS_H */
