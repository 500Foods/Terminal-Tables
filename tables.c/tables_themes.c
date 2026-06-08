/*
 * tables_themes.c - Implementation of theme management for the tables utility
 * Manages visual themes with ANSI color codes and border characters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_themes.h"

/* ANSI color code constants - commented out to suppress unused variable warnings */
/* static const char *RED = "\033[0;31m"; */
/* static const char *BLUE = "\033[0;34m"; */
/* static const char *GREEN = "\033[0;32m"; */
/* static const char *CYAN = "\033[0;36m"; */
/* static const char *BRIGHT_WHITE = "\033[1;37m"; */
/* static const char *DEFAULT_COLOR = "\033[0m"; */

/* Theme definitions */
static ThemeConfig RED_THEME = {
    .border_color = "\033[0;31m",
    .caption_color = "\033[0;32m",
    .header_color = "\033[1;37m",
    .footer_color = "\033[0;36m",
    .summary_color = "\033[1;37m",
    .text_color = "\033[0m",
    .tl_corner = "╭",
    .tr_corner = "╮",
    .bl_corner = "╰",
    .br_corner = "╯",
    .h_line = "─",
    .v_line = "│",
    .t_junct = "┬",
    .b_junct = "┴",
    .l_junct = "├",
    .r_junct = "┤",
    .cross = "┼"
};

static ThemeConfig BLUE_THEME = {
    .border_color = "\033[0;34m",
    .caption_color = "\033[0;34m",
    .header_color = "\033[1;37m",
    .footer_color = "\033[0;36m",
    .summary_color = "\033[1;37m",
    .text_color = "\033[0m",
    .tl_corner = "╭",
    .tr_corner = "╮",
    .bl_corner = "╰",
    .br_corner = "╯",
    .h_line = "─",
    .v_line = "│",
    .t_junct = "┬",
    .b_junct = "┴",
    .l_junct = "├",
    .r_junct = "┤",
    .cross = "┼"
};

/*
 * Set the active theme based on the theme name in the configuration
 */
void get_theme(TableConfig *config) {
    char *theme_name = config->theme_name;
    ThemeConfig *selected_theme = &RED_THEME; // Default to Red
    
    if (theme_name) {
        if (strcasecmp(theme_name, "blue") == 0) {
            selected_theme = &BLUE_THEME;
        } else if (strcasecmp(theme_name, "red") != 0) {
            fprintf(stderr, "%sWarning: Unknown theme '%s', using Red%s\n", 
                    RED_THEME.border_color, theme_name, RED_THEME.text_color);
        }
    }
    
    // Copy the selected theme to the config
    config->theme = *selected_theme;
}

/*
 * Free any dynamically allocated memory in the theme (currently none, as themes are static)
 */
void free_theme(ThemeConfig *theme) {
    // No dynamically allocated memory in ThemeConfig currently
    // This function is provided for future extensibility
    (void)theme; // Suppress unused parameter warning
}
