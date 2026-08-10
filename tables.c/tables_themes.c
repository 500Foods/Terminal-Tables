/*
 * tables_themes.c - Implementation of theme management for the tables utility
 * Manages visual themes with ANSI color codes and border characters.
 *
 * Themes control the visual appearance of tables:
 *   - Color settings for borders, headers, footers, summaries, and text
 *   - Border characters (corners, junctions, horizontal/vertical lines)
 *
 * Currently supports two themes:
 *   - Red: Default theme with red borders
 *   - Blue: Alternative theme with blue borders
 *
 * Themes can be selected in the layout JSON's "theme" field.
 * When --mono is active, all color codes are set to empty strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tables_themes.h"

/* ANSI color code constants - commented out to suppress unused variable warnings */
/* These were originally defined but are now inline in the theme structures below */
/* static const char *RED = "\033[0;31m"; */
/* static const char *BLUE = "\033[0;34m"; */
/* static const char *GREEN = "\033[0;32m"; */
/* static const char *CYAN = "\033[0;36m"; */
/* static const char *BRIGHT_WHITE = "\033[1;37m"; */
/* static const char *DEFAULT_COLOR = "\033[0m"; */

/*
 * Red theme definition
 * Uses red for borders, cyan for captions/footers, bright white for headers/summaries
 * Reset (\033[0m) for default text color
 * Unicode box-drawing characters for borders
 */
static ThemeConfig RED_THEME = {
    .border_color = "\033[0;31m",   /* Red: borders, separators, corners */
    .caption_color = "\033[0;36m",  /* Cyan: column header text */
    .header_color = "\033[1;37m",   /* Bright white: table title text */
    .footer_color = "\033[0;36m",   /* Cyan: footer text */
    .summary_color = "\033[1;37m",  /* Bright white: summary row text */
    .text_color = "\033[0m",        /* Reset: regular data cell text */
    .tl_corner = "╭",               /* Top-left corner */
    .tr_corner = "╮",               /* Top-right corner */
    .bl_corner = "╰",               /* Bottom-left corner */
    .br_corner = "╯",              /* Bottom-right corner */
    .h_line = "─",                  /* Horizontal line */
    .v_line = "│",                  /* Vertical line */
    .t_junct = "┬",                 /* Top junction (for column separators) */
    .b_junct = "┴",                 /* Bottom junction (for column separators) */
    .l_junct = "├",                 /* Left junction (for column separators) */
    .r_junct = "┤",                 /* Right junction (for column separators) */
    .cross = "┼"                    /* Cross junction (for column separators) */
};

/*
 * Blue theme definition
 * Uses blue for borders, cyan for captions/footers, bright white for headers/summaries
 * Same border characters as Red theme
 */
static ThemeConfig BLUE_THEME = {
    .border_color = "\033[0;34m",   /* Blue: borders, separators, corners */
    .caption_color = "\033[0;36m",  /* Cyan: column header text */
    .header_color = "\033[1;37m",   /* Bright white: table title text */
    .footer_color = "\033[0;36m",   /* Cyan: footer text */
    .summary_color = "\033[1;37m",  /* Bright white: summary row text */
    .text_color = "\033[0m",        /* Reset: regular data cell text */
    .tl_corner = "╭",               /* Top-left corner */
    .tr_corner = "╮",               /* Top-right corner */
    .bl_corner = "╰",               /* Bottom-left corner */
    .br_corner = "╯",              /* Bottom-right corner */
    .h_line = "─",                  /* Horizontal line */
    .v_line = "│",                  /* Vertical line */
    .t_junct = "┬",                 /* Top junction */
    .b_junct = "┴",                 /* Bottom junction */
    .l_junct = "├",                 /* Left junction */
    .r_junct = "┤",                 /* Right junction */
    .cross = "┼"                    /* Cross junction */
};

/*
 * Set the active theme based on the theme name in the configuration.
 * When mono_mode is set, all color strings are empty (no ANSI output).
 *
 * Parameters:
 *   config: Table configuration containing the theme_name to look up
 */
void get_theme(TableConfig *config) {
    /* Reference to global mono mode flag */
    extern int mono_mode;
    /* Get the theme name from the config */
    char *theme_name = config->theme_name;
    /* Default to Red theme */
    ThemeConfig *selected_theme = &RED_THEME;

    if (theme_name) {
        /* Check for Blue theme (case-insensitive) */
        if (strcasecmp(theme_name, "blue") == 0) {
            selected_theme = &BLUE_THEME;
        } else if (strcasecmp(theme_name, "red") != 0) {
            /* Unknown theme name: warn and fall back to Red */
            fprintf(stderr, "%sWarning: Unknown theme '%s', using Red%s\n",
                    RED_THEME.border_color, theme_name, RED_THEME.text_color);
        }
    }

    /* Copy the selected theme to the config structure */
    config->theme = *selected_theme;

    /* In mono mode, clear all color strings for monochrome output */
    if (mono_mode) {
        config->theme.border_color = "";
        config->theme.caption_color = "";
        config->theme.header_color = "";
        config->theme.footer_color = "";
        config->theme.summary_color = "";
        config->theme.text_color = "";
    }
}

/*
 * Free any dynamically allocated memory in the theme (currently none, as themes are static)
 * This function is provided for future extensibility when themes may have
 * dynamically allocated color strings or border characters.
 */
void free_theme(ThemeConfig *theme) {
    /* No dynamically allocated memory in ThemeConfig currently */
    /* This function is provided for future extensibility */
    (void)theme; /* Suppress unused parameter warning */
}
