/*
 * EPDdatabox.h
 * Copyright (C) 2024 Richard Pecl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EPDDATABOX_H
#define EPDDATABOX_H

#if defined(USE_EPAPER)
#define ENABLE_GxEPD2_GFX       1
#include <GxEPD2_BW.h>


#endif /* USE_EPAPER */

typedef struct navbox_struct
{
  char      title[16];
  uint16_t  x;
  uint16_t  y;
  uint16_t  width;
  uint16_t  height;
  int32_t   value;
//  int32_t   prev_value;
  uint32_t  timestamp;
} navbox_t;

// string databox
struct databox_s : public navbox_t
{
  uint16_t  title_x; // x coordinate for title text (relative to box x coordinate)
  uint16_t  title_y; // y coordinate for title text (relative to box y coordinate)
  uint16_t  value_x; // x coordinate for value text (relative to box x coordinate)
  uint16_t  value_y; // y coordinate for value text (relative to box y coordinate)
  char  value_str[16]; // formatted value (used instead of the value member)
  const GFXfont* value_font; // font for value text

  void init(const char* titleA, uint16_t xA, uint16_t yA, uint16_t widthA, uint16_t heightA, const GFXfont* valueFont, const char* sampleText);

  void setTitle(const char* titleA);

  void setValueLayout(const GFXfont* valueFont, const char* sampleText);

  void drawRect();

  void printTitle();

  void printValue();

  void redraw();
};

#endif /* EPDDATABOX_H */
