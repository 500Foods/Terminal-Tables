/*
 * tables_render.h - Header file for table rendering functionality
 */

#ifndef TABLES_RENDER_H
#define TABLES_RENDER_H

#include "tables_config.h"
#include "tables_data.h"
#include "tables_render_utils.h"
#include "tables_render_layout.h"
#include "tables_render_output.h"
#include "tables_render_title.h"
#include "tables_render_headers.h"
#include "tables_render_rows.h"
#include "tables_render_summaries.h"
#include "tables_render_footer.h"

// Main rendering function
void render_table(TableConfig *config, TableData *data);

#endif /* TABLES_RENDER_H */
