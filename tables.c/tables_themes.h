/*
 * tables_themes.h - Header file for theme management in the tables utility
 * Defines structures and function prototypes for handling visual themes.
 */

#ifndef TABLES_THEMES_H
#define TABLES_THEMES_H

#include "tables_config.h"

/* Function prototypes */
void get_theme(TableConfig *config);
void free_theme(ThemeConfig *theme);

#endif /* TABLES_THEMES_H */
