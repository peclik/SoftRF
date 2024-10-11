/*
 * Status1_EPD.cpp
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

#include "../driver/EPD.h"
#include "../TrafficHelper.h"
#include "../driver/Battery.h"
#include "../driver/Baro.h"
#include "../driver/RF.h"
#include <protocol.h>

#if defined(ARDUINO_ARCH_NRF52)
#include <pcf8563.h>
#include <bluefruit.h>

extern RTC_Date fw_build_date_time;
#endif /* ARDUINO_ARCH_NRF52 */

static databox_s navbox11;
static databox_s navbox12;
static databox_s navbox21;
static databox_s navbox22;
static databox_s navbox23;
static databox_s navbox31;
static databox_s navbox32;
static databox_s navbox33;
static databox_s navbox41;
static databox_s navbox42;
static databox_s navbox43;

void EPD_status1_setup()
{
  uint16_t disp_width  = display->width();
  uint16_t box_width  = display->width()  / 3;
  uint16_t box_height = display->height() / 4;

  int16_t dy = 0;
  char buf[16];

#if defined(EPD_ASPECT_RATIO_2C1)
  if (display->epd2.panel == GxEPD2::DEPG0213BN) {
    if (display_width  == 128) display_width  = 122;
    if (display_height == 128) {
      display_height = 122;
      if (display->getRotation() == ROTATE_90 ) { dy = 6; }
    }
  }
#endif /* EPD_ASPECT_RATIO_2C1 */

  // Layout:
  // TIME     BAT%
  // QNHft QNEft SPDknot
  // QNHm  QNEm  SPDkmph
  // ACTFS ^ms   TRK

  navbox11.init("UTC",                                 0, 0 * box_height + dy, disp_width - 1 * (box_width) + 20, box_height, &OpenSansCondensed15pt7b, "00:00:00");
  navbox12.init("BAT%",  navbox11.x + navbox11.width - 1, 0 * box_height + dy, box_width - 20, box_height, &OpenSansCondensed15pt7b, "100");

  navbox21.init("QNHft",                               0, 1 * box_height + dy, disp_width - 2 * (box_width - 1) + 14, box_height, &OpenSansCondensed15pt7b, "00000?");
  navbox22.init("QNEft", navbox21.x + navbox21.width - 1, 1 * box_height + dy, box_width + 6, box_height, &OpenSansCondensed15pt7b, "00000");
  navbox23.init("knot",  navbox22.x + navbox22.width - 1, 1 * box_height + dy, box_width - 20, box_height, &OpenSansCondensed15pt7b, "000");

  navbox31.init("QNHm",                                0, 2 * box_height + dy, disp_width - 2 * (box_width - 1) + 14, box_height, &OpenSansCondensed15pt7b, "00000?");
  navbox32.init("QNEm",  navbox31.x + navbox31.width - 1, 2 * box_height + dy, box_width + 6, box_height, &OpenSansCondensed15pt7b, "00000");
  navbox33.init("km/h",  navbox32.x + navbox32.width - 1, 2 * box_height + dy, box_width - 20, box_height, &OpenSansCondensed15pt7b, "000");

  navbox41.init("ACTF",                               0, 3 * box_height + dy, disp_width - 2 * (box_width - 1) + 14, box_height, &OpenSansCondensed15pt7b, "0");
  navbox42.init("^ms",  navbox41.x + navbox41.width - 1, 3 * box_height + dy, box_width + 6, box_height, &OpenSansCondensed15pt7b, "-00");
  navbox43.init("TRK",  navbox42.x + navbox42.width - 1, 3 * box_height + dy, box_width - 20, box_height, &OpenSansCondensed15pt7b, "000");
}

static void EPD_Draw_NavBoxes()
{
    bool ble_has_client = false;

#if defined(USE_EPD_TASK)
  if (EPD_update_in_progress == EPD_UPDATE_NONE) {
//  if (SoC->Display_lock()) {
#else
  {
#endif

#if defined(ARDUINO_ARCH_NRF52)
    ble_has_client = Bluefruit.connected();
#endif /* ARDUINO_ARCH_NRF52 */

    display->fillScreen(GxEPD_WHITE);

    navbox11.redraw();
    navbox12.redraw();
    navbox21.redraw();
    navbox22.redraw();
    navbox23.redraw();
    navbox31.redraw();
    navbox32.redraw();
    navbox33.redraw();
    navbox41.redraw();
    navbox42.redraw();
    navbox43.redraw();

    if (ble_has_client) {
      int16_t  tbx, tby;
      uint16_t tbw, tbh;

      display->setFont(&FreeSerifBold9pt7b);
      //display->setTextColor(GxEPD_WHITE, GxEPD_BLACK);
      display->getTextBounds("BT", 0, 0, &tbx, &tby, &tbw, &tbh);
      display->setCursor(navbox11.x + navbox11.width - 3 - tbw, navbox11.y + 3 + tbh);
      display->print("BT");
      //display->setTextColor(GxEPD_BLACK);
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

void EPD_status1_loop()
{
  if (isTimeToEPD()) {

    RTC_Date now;
    now.year = 0;
    if (rtc && rtc->isVaild()) {
      now = rtc->getDateTime();
    }
    if (now.year >= fw_build_date_time.year && now.year < fw_build_date_time.year + 15) {
      snprintf(navbox11.value_str, sizeof(navbox11.value_str), "%02d:%02d:%02d", now.hour, now.minute, now.second);
    } else {
      snprintf(navbox11.value_str, sizeof(navbox11.value_str), "--:--:--");
    }

    snprintf(navbox12.value_str, sizeof(navbox12.value_str), "%u", Battery_charge());

    bool gnss_height_ok = (
      gnss.pdop.isValid() && gnss.pdop.age() <= NMEA_EXP_TIME && gnss.pdop.dop() <= 1.1 &&
      gnss.vdop.isValid() && gnss.vdop.age() <= NMEA_EXP_TIME && gnss.vdop.dop() <= 1.1);

    if (gnss.altitude.isValid() && gnss.altitude.age() <= NMEA_EXP_TIME) {
      snprintf(navbox21.value_str, sizeof(navbox21.value_str), "%-5.0f%c", gnss.altitude.feet(), gnss_height_ok ? ' ' : '?');
      snprintf(navbox31.value_str, sizeof(navbox31.value_str), "%-5.0f%c", gnss.altitude.meters(), gnss_height_ok ? ' ' : '?');
    } else {
      snprintf(navbox21.value_str, sizeof(navbox21.value_str), "--");
      snprintf(navbox31.value_str, sizeof(navbox31.value_str), "--");
    }

    if (baro_chip != NULL) {
      snprintf(navbox22.value_str, sizeof(navbox22.value_str), "%.0f", Baro_altitude() * _GPS_FEET_PER_METER);
      snprintf(navbox32.value_str, sizeof(navbox32.value_str), "%.0f", Baro_altitude());
      float vertical_speed = ThisAircraft.vs / (_GPS_FEET_PER_METER * 60.0);
      // avoid printing "-0"
      if (vertical_speed > -0.5 && vertical_speed < 0.5)
        vertical_speed = 0;
      snprintf(navbox42.value_str, sizeof(navbox42.value_str), "%.0f", vertical_speed);
    } else {
      snprintf(navbox22.value_str, sizeof(navbox22.value_str), "--");
      snprintf(navbox33.value_str, sizeof(navbox32.value_str), "--");
      snprintf(navbox42.value_str, sizeof(navbox42.value_str), "--");
    }

    if (gnss.speed.isValid() && gnss.speed.age() <= NMEA_EXP_TIME) {
      snprintf(navbox23.value_str, sizeof(navbox23.value_str), "%.0f", gnss.speed.knots());
      snprintf(navbox33.value_str, sizeof(navbox33.value_str), "%.0f", gnss.speed.kmph());
    } else {
      snprintf(navbox23.value_str, sizeof(navbox23.value_str), "--");
      snprintf(navbox33.value_str, sizeof(navbox33.value_str), "--");
    }

    if (gnss.course.isValid() && gnss.course.age() <= NMEA_EXP_TIME) {
      snprintf(navbox43.value_str, sizeof(navbox43.value_str), "%.0f", gnss.course.deg());
    } else {
      snprintf(navbox43.value_str, sizeof(navbox43.value_str), "--");
    }

    if (Traffic_Count() > 0) {
      navbox41.setValueLayout(&OpenSansCondensed15pt7b, "* 0 *");
      snprintf(navbox41.value_str, sizeof(navbox41.value_str), "* %d *", Traffic_Count());
    } else {
      navbox41.setValueLayout(&OpenSansCondensed15pt7b, "0");
      snprintf(navbox41.value_str, sizeof(navbox41.value_str), "%d", Traffic_Count());
    }

    EPD_Draw_NavBoxes();

    EPDTimeMarker = millis();
  }
}

#endif /* USE_EPAPER */
