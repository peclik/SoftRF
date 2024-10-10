/*
 * EPDdatabox.cpp
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

#include "../system/SoC.h"

#if defined(USE_EPAPER)

#include "EPD.h"


void databox_s::init(const char* titleA, uint16_t xA, uint16_t yA, uint16_t widthA, uint16_t heightA, const GFXfont* valueFont, const char* sampleText) {
  setTitle(titleA);
  x = xA;
  y = yA;
  width  = widthA;
  height = heightA;
  value_str[0] = '\0';
  timestamp = 0;
  setValueLayout(valueFont, sampleText);
}

void databox_s::setTitle(const char* titleA) {
  int16_t  tbx, tby;
  uint16_t tbw, tbh;
  snprintf(title, sizeof(title), "%s", titleA);
  display->setFont(&FreeMono9pt7b);
  // do not use the title for getTextBounds (some lowercase letter are higher then capitals,
  // and the title would be shifted too much)
  display->getTextBounds("B", 0, 0, &tbx, &tby, &tbw, &tbh);
  title_x = 4;
  title_y = 4 + tbh;
}

void databox_s::setValueLayout(const GFXfont* valueFont, const char* sampleText) {
  int16_t  tbx, tby;
  uint16_t tbw, tbh;
  value_font = valueFont;
  display->setFont(value_font);
  display->getTextBounds(sampleText, 0, 0, &tbx, &tby, &tbw, &tbh);
  value_y = (title_y + height - 2 + tbh) / 2;
  value_x = (width - tbw) / 2;
}

void databox_s::drawRect() {
  //display->drawRoundRect(x + 1, y + 1, width - 2, height - 2, 4, GxEPD_BLACK);
  display->drawRoundRect(x, y, width, height, 0, GxEPD_BLACK);
}

void databox_s::printTitle() {
  display->setFont(&FreeMono9pt7b);
  display->setCursor(x + title_x, y + title_y);
  display->print(title);
}

void databox_s::printValue() {
  display->setFont(value_font);
  display->setCursor(x + value_x, y + value_y);
  display->print(value_str);
}

void databox_s::redraw() {
  drawRect();
  printTitle();
  printValue();
}

#endif /* EPDNVABOX_H */
