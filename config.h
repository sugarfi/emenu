/* See LICENSE file for copyright and license details. */

#define LINE_HEIGHT 20 // Height of one block in pixels, not one line of text
#define WINDOW_WIDTH 150 // Width of the window
#define LINES 10 // The number of lines shown at a time
#define WINDOW_HEIGHT (LINE_HEIGHT * LINES) + LINE_HEIGHT // Don't change this
const char *colors[] = { // The color scheme: first color is background, second foreground
    "#ffffff",
    "#000000"
};
const char *fonts[] = { // The font to use
    "monospace:size=10"
};
