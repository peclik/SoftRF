/*
 * View_Radar_EPD.cpp
 * Copyright (C) 2019-2024 Linar Yusupov
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

#include "../driver/EPD.h"

#include <TimeLib.h>

#include "../TrafficHelper.h"
#include "../driver/Battery.h"
#include "../driver/EEPROM.h"
#include "../protocol/data/NMEA.h"
#include "../protocol/data/GDL90.h"
#include "../driver/LED.h"
#include "../driver/RF.h"

static int EPD_zoom = ZOOM_MEDIUM;

enum {
   STATE_RVIEW_NONE,
   STATE_RVIEW_RADAR,
   STATE_RVIEW_NOFIX,
   STATE_RVIEW_NODATA
};

static int view_state_curr = STATE_RVIEW_NONE;
static int view_state_prev = STATE_RVIEW_NONE;

static void EPD_Draw_Radar()
{
  int16_t  tbx, tby;
  uint16_t tbw, tbh;
  uint16_t x;
  uint16_t y;
  char buf[6];

#if defined(USE_EPD_TASK)
  if (EPD_update_in_progress == EPD_UPDATE_NONE) {
//  if (SoC->Display_lock()) {
#else
  {
#endif
    /* divider is a half of full scale */
    int32_t divider = 2000;

    uint16_t display_width  = display->width();
    uint16_t display_height = display->height();

#if defined(EPD_ASPECT_RATIO_2C1)
    int16_t dy = 0;

    if (display->epd2.panel == GxEPD2::DEPG0213BN) {
      if (display_width  == 128) display_width  = 122;
      if (display_height == 128) {
        display_height = 122;
        if (display->getRotation() == ROTATE_90 ) { dy = 6; }
      }
    }
#endif /* EPD_ASPECT_RATIO_2C1 */

    display->setFont(&FreeMono9pt7b);
    display->getTextBounds("N", 0, 0, &tbx, &tby, &tbw, &tbh);

#if defined(EPD_ASPECT_RATIO_1C1)
    uint16_t radar_x = 0;
    uint16_t radar_y = (display_height - display_width) / 2;
    uint16_t radar_w = display_width;

    uint16_t radar_center_x = radar_w / 2;
    uint16_t radar_center_y = radar_y + radar_w / 2;
    uint16_t radius = radar_w / 2 - 2;
#endif /* EPD_ASPECT_RATIO_1C1 */
#if defined(EPD_ASPECT_RATIO_2C1)
    uint16_t radar_x = (display_width - display_height) / 4;
    uint16_t radar_y = 0 + dy;
    uint16_t radar_w = (display_width + display_height) / 2;

    uint16_t radar_center_x = radar_x + radar_w / 2;
    uint16_t radar_center_y = radar_y + display_height - display_height / 4;
    uint16_t radius = radar_w / 2 - 2;
#endif /* EPD_ASPECT_RATIO_2C1 */

    if (ui->units == UNITS_METRIC || ui->units == UNITS_MIXED) {
      switch(EPD_zoom)
      {
      case ZOOM_LOWEST:
        divider = 30000; /* 60 KM */
        break;
      case ZOOM_LOW:
        divider =  5000; /* 10 KM */
        break;
      case ZOOM_HIGH:
        divider =  1000; /*  2 KM */
        break;
      case ZOOM_MEDIUM:
      default:
        divider =  2000;  /* 4 KM */
        break;
      }
    } else {
      switch(EPD_zoom)
      {
      case ZOOM_LOWEST:
        divider = 27780;  /* 30 NM */
        break;
      case ZOOM_LOW:
        divider = 4630;  /*  5 NM */
        break;
      case ZOOM_HIGH:
        divider =  926;  /*  1 NM */
        break;
      case ZOOM_MEDIUM:  /*  2 NM */
      default:
        divider = 1852;
        break;
      }
    }

    display->fillScreen(GxEPD_WHITE);

    {
      for (int i=0; i < MAX_TRACKING_OBJECTS; i++) {
        if (Container[i].addr && (now() - Container[i].timestamp) <= EPD_EXPIRATION_TIME) {

          int16_t rel_x;
          int16_t rel_y;
          float distance;
          float bearing;

          bool isTeam = (Container[i].addr == ui->team) ;

          distance = Container[i].distance;
          bearing  = Container[i].bearing;

          switch (ui->orientation)
          {
          case DIRECTION_NORTH_UP:
            break;
          case DIRECTION_TRACK_UP:
            bearing -= ThisAircraft.course;
            break;
          default:
            /* TBD */
            break;
          }

          rel_x = constrain(distance * sin(radians(bearing)),
                                       -32768, 32767);
          rel_y = constrain(distance * cos(radians(bearing)),
                                       -32768, 32767);

          int16_t x = ((int32_t) rel_x * (int32_t) radius) / divider;
          int16_t y = ((int32_t) rel_y * (int32_t) radius) / divider;

          float RelativeVertical = Container[i].altitude - ThisAircraft.altitude;

          if        (RelativeVertical >   EPD_RADAR_V_THRESHOLD) {
            if (isTeam) {
              display->drawTriangle(radar_center_x + x - 5, radar_center_y - y + 4,
                                    radar_center_x + x    , radar_center_y - y - 6,
                                    radar_center_x + x + 5, radar_center_y - y + 4,
                                    GxEPD_BLACK);
              display->drawTriangle(radar_center_x + x - 6, radar_center_y - y + 5,
                                    radar_center_x + x    , radar_center_y - y - 7,
                                    radar_center_x + x + 6, radar_center_y - y + 5,
                                    GxEPD_BLACK);
            } else {
              display->fillTriangle(radar_center_x + x - 4, radar_center_y - y + 3,
                                    radar_center_x + x    , radar_center_y - y - 5,
                                    radar_center_x + x + 4, radar_center_y - y + 3,
                                    GxEPD_BLACK);
            }
          } else if (RelativeVertical < - EPD_RADAR_V_THRESHOLD) {
            if (isTeam) {
              display->drawTriangle(radar_center_x + x - 5, radar_center_y - y - 4,
                                    radar_center_x + x    , radar_center_y - y + 6,
                                    radar_center_x + x + 5, radar_center_y - y - 4,
                                    GxEPD_BLACK);
              display->drawTriangle(radar_center_x + x - 6, radar_center_y - y - 5,
                                    radar_center_x + x    , radar_center_y - y + 7,
                                    radar_center_x + x + 6, radar_center_y - y - 5,
                                    GxEPD_BLACK);
            } else {
              display->fillTriangle(radar_center_x + x - 4, radar_center_y - y - 3,
                                    radar_center_x + x    , radar_center_y - y + 5,
                                    radar_center_x + x + 4, radar_center_y - y - 3,
                                    GxEPD_BLACK);
            }
          } else {
            if (isTeam) {
              display->drawCircle(radar_center_x + x,
                                  radar_center_y - y,
                                  6, GxEPD_BLACK);
              display->drawCircle(radar_center_x + x,
                                  radar_center_y - y,
                                  7, GxEPD_BLACK);
             } else {
              display->fillCircle(radar_center_x + x,
                                  radar_center_y - y,
                                  5, GxEPD_BLACK);
            }
          }
        }
      }

      display->drawCircle(  radar_center_x, radar_center_y,
                            radius, GxEPD_BLACK);
      display->drawCircle(  radar_center_x, radar_center_y,
                            radius / 2, GxEPD_BLACK);

      if (ThisAircraft.aircraft_type == AIRCRAFT_TYPE_GLIDER     ||
          ThisAircraft.aircraft_type == AIRCRAFT_TYPE_TOWPLANE   ||
          ThisAircraft.aircraft_type == AIRCRAFT_TYPE_HELICOPTER ||
          ThisAircraft.aircraft_type == AIRCRAFT_TYPE_DROPPLANE  ||
          ThisAircraft.aircraft_type == AIRCRAFT_TYPE_POWERED    ||
          ThisAircraft.aircraft_type == AIRCRAFT_TYPE_JET) {

        /* little airplane */
        display->drawFastVLine(radar_center_x,      radar_center_y - 4, 14, GxEPD_BLACK);
        display->drawFastVLine(radar_center_x + 1,  radar_center_y - 4, 14, GxEPD_BLACK);

        display->drawFastHLine(radar_center_x - 8,  radar_center_y,     18, GxEPD_BLACK);
        display->drawFastHLine(radar_center_x - 10, radar_center_y + 1, 22, GxEPD_BLACK);

        display->drawFastHLine(radar_center_x - 3,  radar_center_y + 8,  8, GxEPD_BLACK);
        display->drawFastHLine(radar_center_x - 2,  radar_center_y + 9,  6, GxEPD_BLACK);

      } else {

        /* arrow tip */
        display->fillTriangle(radar_center_x - 7, radar_center_y + 5,
                              radar_center_x    , radar_center_y - 5,
                              radar_center_x + 7, radar_center_y + 5,
                              GxEPD_BLACK);
        display->fillTriangle(radar_center_x - 7, radar_center_y + 5,
                              radar_center_x    , radar_center_y + 2,
                              radar_center_x + 7, radar_center_y + 5,
                              GxEPD_WHITE);
      }

      switch (ui->orientation)
      {
      case DIRECTION_NORTH_UP:
        x = radar_x + radar_w / 2 - radius + tbw/2;
        y = radar_y + (radar_w + tbh) / 2;
        display->setCursor(x , y);
        display->print("W");
        x = radar_x + radar_w / 2 + radius - (3 * tbw)/2;
        y = radar_y + (radar_w + tbh) / 2;
        display->setCursor(x , y);
        display->print("E");
        x = radar_x + (radar_w - tbw) / 2;
        y = radar_y + radar_w/2 - radius + (3 * tbh)/2;
        display->setCursor(x , y);
        display->print("N");
        x = radar_x + (radar_w - tbw) / 2;
        y = radar_y + radar_w/2 + radius - tbh/2;
        display->setCursor(x , y);
        display->print("S");
        break;
      case DIRECTION_TRACK_UP:
        x = radar_x + radar_w / 2 - radius + tbw/2;
        y = radar_y + (radar_w + tbh) / 2;
        display->setCursor(x , y);
        display->print("L");
        x = radar_x + radar_w / 2 + radius - (3 * tbw)/2;
        y = radar_y + (radar_w + tbh) / 2;
        display->setCursor(x , y);
        display->print("R");
        x = radar_x + (radar_w - tbw) / 2;
        y = radar_y + radar_w/2 + radius - tbh/2;
        display->setCursor(x , y);
        display->print("B");

        display->setFont(&FreeMonoBold9pt7b);
        snprintf(buf, sizeof(buf), "%03d", (int) ThisAircraft.course);
        display->getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);

        x = radar_x + (radar_w - tbw) / 2;
        y = radar_y + radar_w/2 - radius + (3 * tbh)/2;
        display->setCursor(x , y);
        display->print(buf);
        display->drawRoundRect( x - 2, y - tbh - 2,
                                tbw + 8, tbh + 6,
                                4, GxEPD_BLACK);
        break;
      default:
        /* TBD */
        break;
      }

      snprintf(buf, sizeof(buf), "%u", Battery_charge());
      display->setFont(&OpenSansCondensed15pt7b);
      display->getTextBounds(buf, 0, 0, &tbx, &tby, &tbw, &tbh);
      x = display_width - tbw - 3;
      y = tbh;
      display->setCursor(x, y);
      display->print(buf);

      y += 10;
      u8g2Fonts.setCursor(display_width - 5, y);
      u8g2Fonts.print((Battery_charge() + 19) / 20);

      display->setFont(&FreeMono9pt7b);
      display->getTextBounds("ACFTS", 0, 0, &tbx, &tby, &tbw, &tbh);
#if defined(EPD_ASPECT_RATIO_1C1)
      x = 0;
      y = display_height - 1;
#endif /* EPD_ASPECT_RATIO_1C1 */
#if defined(EPD_ASPECT_RATIO_2C1)
      x = tbw / 2;
      y = display_height + dy - 1;
#endif /* EPD_ASPECT_RATIO_2C1 */
      display->setCursor(x, y);
      display->print("ACFTS");

      y -= tbh + 5;

      display->setFont(&OpenSansCondensed15pt7b);
      display->setCursor(x, y);
      display->print(Traffic_Count());

      display->setFont(&FreeMono9pt7b);
      display->getTextBounds("km ", 0, 0, &tbx, &tby, &tbw, &tbh);

#if defined(EPD_ASPECT_RATIO_1C1)
      x = display_width - tbw;
      y = display_height - 1;
#endif /* EPD_ASPECT_RATIO_1C1 */
#if defined(EPD_ASPECT_RATIO_2C1)
      x = radar_x + radar_w - tbw;
      y = display_height + dy - 1;
#endif /* EPD_ASPECT_RATIO_2C1 */
      display->setCursor(x, y);

      display->print(ui->units == UNITS_METRIC || ui->units == UNITS_MIXED ?
                     "km" : "NM");

      y -= tbh + 5;

      char zoom_text[5];
      if (ui->units == UNITS_METRIC || ui->units == UNITS_MIXED) {
        snprintf(zoom_text, sizeof(zoom_text), "%s",
          EPD_zoom == ZOOM_LOWEST ? "60" :
          EPD_zoom == ZOOM_LOW    ? "10" :
          EPD_zoom == ZOOM_MEDIUM ? " 4" :
          EPD_zoom == ZOOM_HIGH   ? " 2" : "");
      } else {
        snprintf(zoom_text, sizeof(zoom_text), "%s",
          EPD_zoom == ZOOM_LOWEST ? "30" :
          EPD_zoom == ZOOM_LOW    ? " 5" :
          EPD_zoom == ZOOM_MEDIUM ? " 2" :
          EPD_zoom == ZOOM_HIGH   ? " 1" : "");
      }

      display->setFont(&OpenSansCondensed15pt7b);
      display->getTextBounds(zoom_text, 0, 0, &tbx, &tby, &tbw, &tbh);

#if defined(EPD_ASPECT_RATIO_1C1)
      x = display_width - tbw;
#endif /* EPD_ASPECT_RATIO_1C1 */
#if defined(EPD_ASPECT_RATIO_2C1)
      x = radar_x + radar_w - tbw;
#endif /* EPD_ASPECT_RATIO_2C1 */
      display->setCursor(x, y);

      display->print(zoom_text);
    }

#if defined(USE_EPD_TASK)
    /* a signal to background EPD update task */
    EPD_update_in_progress = EPD_UPDATE_FAST;
//    SoC->Display_unlock();
//    yield();
#else
    display->display(true);
#endif
  }
}

void EPD_radar_setup()
{
  EPD_zoom = ui->zoom;
}

void EPD_radar_loop()
{
  if (isTimeToEPD()) {
    bool hasFix = isValidGNSSFix() || (settings->mode == SOFTRF_MODE_TXRX_TEST);

    if (hasFix) {
      EPD_Draw_Radar();
    } else {
      EPD_Message(NO_FIX_TEXT, NULL);
    }

    EPDTimeMarker = millis();
  }
}

void EPD_radar_zoom()
{
  if (EPD_zoom < ZOOM_HIGH) EPD_zoom++;
}

void EPD_radar_unzoom()
{
  if (EPD_zoom > ZOOM_LOWEST) EPD_zoom--; else EPD_zoom = ZOOM_HIGH;
}

#endif /* USE_EPAPER */
