/*
 * tables_render_utils.h - Header file for utility functions used in table rendering
 *
 * This header declares utility functions for:
 *   - String duplication (safe, handles NULL input)
 *   - Display width calculation (handles ANSI codes and Unicode)
 *   - Text clipping (with and without color support)
 *   - Text wrapping (word-based and delimiter-based)
 *   - Dynamic command evaluation ($() substitution)
 *   - Color placeholder replacement ({RED}, {BLUE}, etc.)
 */

#ifndef TABLES_RENDER_UTILS_H
#define TABLES_RENDER_UTILS_H

#include "tables_config.h"

/*
 * Duplicate a string safely, returning NULL if input is NULL.
 * Wraps the POSIX strdup() with a NULL guard.
 */
char *strdup_safe(const char *str);

/*
 * Calculate display width of text, accounting for ANSI escape codes.
 * ANSI codes (which don't take up visible space) are stripped before counting.
 * Unicode characters are handled: CJK and emoji characters count as 2,
 * while most other characters count as 1.
 */
int get_display_width(const char *text);

/*
 * Clip text to a maximum display width, preserving ANSI codes and handling Unicode.
 * The clipping is left-based: characters are taken from the beginning until
 * max_width display columns are filled.
 * Returns a newly allocated string.
 */
char *clip_text_to_width(const char *text, int max_width);

/*
 * Wrap text to a specified width, returning an array of lines.
 * Handles ANSI escape codes by ignoring them in width calculations.
 * Mimics Bash script behavior by building lines word by word.
 * The line_count output parameter is set to the number of lines returned.
 */
char **wrap_text(const char *text, int width, int *line_count);

/*
 * Wrap text based on a delimiter, returning an array of lines.
 * Splits on every occurrence of the delimiter string, producing one line per segment.
 * Handles ANSI escape codes by preserving them in the output.
 */
char **wrap_text_delimiter(const char *text, int width, const char *delimiter, int *line_count);

/*
 * Free memory allocated for wrapped text lines.
 * Frees each line string and then the array of line pointers.
 */
void free_wrapped_text(char **lines, int line_count);

/*
 * Process a string to evaluate dynamic commands within $() and return the result.
 * Forks a process to execute the command (via popen) and captures its output.
 * Multiple $(...) expressions in the input are all evaluated and replaced.
 */
char *evaluate_dynamic_string(const char *input);

/*
 * Replace color placeholders like {RED}, {BLUE}, {NC}, etc., with ANSI escape codes.
 * In --mono mode, all placeholders are replaced with empty strings.
 * Supported placeholders: {RED}, {BLUE}, {GREEN}, {YELLOW}, {CYAN},
 *   {MAGENTA}, {WHITE}, {BOLD}, {DIM}, {UNDERLINE}, {NC}, {RESET}
 */
char *replace_color_placeholders(const char *input);

/*
 * Clip text to a specified width, handling multi-byte characters and ANSI codes.
 * The clipping behavior depends on the justification parameter:
 *   - POSITION_CENTER: Clip from both ends to show the middle
 *   - POSITION_RIGHT:  Skip characters from the beginning to show the end
 *   - Default (LEFT):  Clip from the right, keeping the beginning
 */
char *clip_text(const char *text, int width, Position justification);

/*
 * Clip text with color placeholders, processing colors first then clipping.
 * This is useful for data cell values that may contain {COLOR} placeholders.
 * The color placeholders are converted to ANSI codes first, then the text
 * is clipped based on display width (excluding ANSI codes from the width count).
 */
char *clip_text_with_colors(const char *text, int width, Position justification);

#endif /* TABLES_RENDER_UTILS_H */
