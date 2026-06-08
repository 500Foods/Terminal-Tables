/*
 * tables_render_utils.h - Header file for utility functions used in table rendering
 */

#ifndef TABLES_RENDER_UTILS_H
#define TABLES_RENDER_UTILS_H

#include "tables_config.h"

/*
 * Helper function to duplicate a string, returning NULL if input is NULL
 */
char *strdup_safe(const char *str);

/*
 * Calculate display width of text, accounting for ANSI escape codes
 */
int get_display_width(const char *text);

/*
 * Clip text to a maximum display width, preserving ANSI codes and handling Unicode properly
 */
char *clip_text_to_width(const char *text, int max_width);

/*
 * Wrap text to a specified width, returning an array of lines
 */
char **wrap_text(const char *text, int width, int *line_count);

/*
 * Wrap text based on a delimiter, returning an array of lines
 */
char **wrap_text_delimiter(const char *text, int width, const char *delimiter, int *line_count);

/*
 * Free memory allocated for wrapped text lines
 */
void free_wrapped_text(char **lines, int line_count);

/*
 * Process a string to evaluate dynamic commands within $() and return the result
 */
char *evaluate_dynamic_string(const char *input);

/*
 * Replace color placeholders like {RED}, {NC}, etc., with ANSI escape codes
 */
char *replace_color_placeholders(const char *input);

/*
 * Clip text to a specified width
 */
char *clip_text(const char *text, int width, Position justification);

/*
 * Clip text with color placeholders, processing colors first then clipping
 */
char *clip_text_with_colors(const char *text, int width, Position justification);

#endif /* TABLES_RENDER_UTILS_H */
