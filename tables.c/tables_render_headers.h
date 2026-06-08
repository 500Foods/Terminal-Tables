/*
 * tables_render_headers.h - Header file for header rendering functions used in table rendering
 */

#ifndef TABLES_RENDER_HEADERS_H
#define TABLES_RENDER_HEADERS_H

#include "tables_config.h"

/*
 * Render the table headers with proper alignment and padding
 */
void render_headers(TableConfig *config);

/*
 * Render the separator line below the headers
 */
void render_header_separator(TableConfig *config);

#endif /* TABLES_RENDER_HEADERS_H */
