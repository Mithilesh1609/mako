
// text_render.c
//
// Simple text renderer.
//
// Author: Ajay Tatachar <ajaymt2@illinois.edu>

#include <stddef.h>
#include "text_render.h"
#include "font_monaco.h"

void text_dimensions(const char *str, size_t len, size_t *w, size_t *h)
{
  size_t rows = 0;
  size_t cols = 0;
  size_t row = 0;
  size_t col = 0;
  for (size_t i = 0; i < len; ++i) {
    switch (str[i]) {
    case '\n':
      ++row; col = 0;
      break;
    case '\t':
      col += 4;
      if (!row) row = 1;
      break;
    default:
      ++col;
      if (!row) row = 1;
    }
    if (row > rows) rows = row;
    if (col > cols) cols = col;
  }

  *w = cols * FONTWIDTH;
  *h = rows * (FONTHEIGHT + FONTVPADDING);
}

void text_render(const char *str, size_t len, size_t w, size_t h, char *buf)
{
  size_t row = 0;
  size_t col = 0;
  char *font = font_monaco();
  for (size_t i = 0; i < len; ++i) {
    if (col * FONTWIDTH >= w) break;
    if (row * (FONTHEIGHT + FONTVPADDING) >= h) break;

    char c = 0;
    switch (str[i]) {
    case '\n': ++row; col = 0; break;
    case '\t': col += 4; break;
    default: c = str[i];
    }

    if (c < 32 || c > 126) continue;

    char *p = buf
      + (col * FONTWIDTH)
      + (row * (FONTHEIGHT + FONTVPADDING) * w);
    for (size_t y = 0; y < FONTHEIGHT; ++y) {
      for (size_t x = 0; x < FONTWIDTH; ++x) {
        char byte = font[((c - 32) * FONTHEIGHT) + y];
        char bit = (byte >> (7 - x)) & 1;
        p[x] = bit;
      }
      p += w;
    }

    ++col;
  }
}
