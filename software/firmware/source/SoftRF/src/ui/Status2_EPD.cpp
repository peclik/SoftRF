/*
 * Status2_EPD.cpp
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
#include "../driver/EEPROM.h"
#include "../driver/RF.h"
#include <protocol.h>

extern RTC_Date fw_build_date_time;

static databox_s navbox11;
static databox_s navbox12;
static databox_s navbox13;
static databox_s navbox21;
static databox_s navbox22;
static databox_s navbox23;
static databox_s navbox31;
static databox_s navbox32;
static databox_s navbox33;
static databox_s navbox34;
static databox_s navbox41;
static databox_s navbox42;
static databox_s navbox43;
static databox_s navbox51;
static databox_s navbox52;
static databox_s navbox53;
static databox_s navbox54;

void EPD_status2_setup()
{
  uint16_t disp_width  = display->width();
  uint16_t box_width  = display->width()  / 3;
  uint16_t box_height = display->height() / 5;

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
  // TIME     BAT  TEMP
  // QNHft   mbar  QNEft
  // SPDkn   TRK ^ms  SAT
  // Prot/Id     VDOP PDOP
  // TX  RX RSSI ACTFS

  navbox11.init("UTC",                                 0, 0 * box_height + dy, disp_width - 2 * (box_width - 20 - 1), box_height, &FreeSerifBold12pt7b, "00:00:00");
  navbox12.init("BAT",   navbox11.x + navbox11.width - 1, 0 * box_height + dy, box_width - 20, box_height, &FreeSerifBold12pt7b, "0.0");
  navbox13.init("TMP",   navbox12.x + navbox12.width - 1, 0 * box_height + dy, box_width - 20, box_height, &FreeSerifBold12pt7b, "-00");

  navbox21.init("QNHft",                               0, 1 * box_height + dy, disp_width - 2 * (box_width - 1) + 10, box_height, &FreeSerifBold12pt7b, "00000?");
  navbox22.init("mbar",  navbox21.x + navbox21.width - 1, 1 * box_height + dy, box_width - 10, box_height, &FreeSerifBold12pt7b, "1000");
  navbox23.init("QNEft", navbox22.x + navbox22.width - 1, 1 * box_height + dy, box_width, box_height, &FreeSerifBold12pt7b, "00000");

  navbox31.init("knot",                                0, 2 * box_height + dy, disp_width - 3 * (box_width - 15 - 1), box_height, &FreeSerifBold12pt7b, "000");
  navbox32.init("TRK",   navbox31.x + navbox31.width - 1, 2 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "000");
  navbox33.init("^ms",   navbox32.x + navbox32.width - 1, 2 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "-00");
  navbox34.init("SAT",   navbox33.x + navbox33.width - 1, 2 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "00");

  // init constant value
  snprintf(buf, sizeof(buf), "%06X", ThisAircraft.addr);
  navbox41.init(Protocol_ID[settings->rf_protocol],
                                                       0, 3 * box_height + dy, disp_width - 2 * (box_width - 15 - 1), box_height, &FreeSerifBold12pt7b, buf);
  navbox42.init("VDOP",  navbox41.x + navbox41.width - 1, 3 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "00.0");
  navbox43.init("PDOP",  navbox42.x + navbox42.width - 1, 3 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "00.0");

  snprintf(navbox41.value_str, sizeof(navbox41.value_str), "%s", buf);

  navbox51.init("TX",                                  0, 4 * box_height + dy, disp_width - 3 * (box_width - 15 - 1), box_height, &FreeSerifBold12pt7b, "000");
  navbox52.init("RX",    navbox51.x + navbox51.width - 1, 4 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "000");
  navbox53.init("RSSI",  navbox52.x + navbox52.width - 1, 4 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "00");
  navbox54.init("ACFT",  navbox53.x + navbox53.width - 1, 4 * box_height + dy, box_width - 15, box_height, &FreeSerifBold12pt7b, "0");
}

static void EPD_Draw_NavBoxes()
{
#if defined(USE_EPD_TASK)
  if (EPD_update_in_progress == EPD_UPDATE_NONE) {
//  if (SoC->Display_lock()) {
#else
  {
#endif
    display->fillScreen(GxEPD_WHITE);

    navbox11.redraw();
    navbox12.redraw();
    navbox13.redraw();
    navbox21.redraw();
    navbox22.redraw();
    navbox23.redraw();
    navbox31.redraw();
    navbox32.redraw();
    navbox33.redraw();
    navbox34.redraw();
    navbox41.redraw();
    navbox42.redraw();
    navbox43.redraw();
    navbox51.redraw();
    navbox52.redraw();
    navbox53.redraw();
    navbox54.redraw();

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

void EPD_status2_loop()
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

    snprintf(navbox12.value_str, sizeof(navbox12.value_str), "%.1f", Battery_voltage());

    bool gnss_height_ok = (
      gnss.pdop.isValid() && gnss.pdop.age() <= NMEA_EXP_TIME && gnss.pdop.dop() <= 1.1 &&
      gnss.vdop.isValid() && gnss.vdop.age() <= NMEA_EXP_TIME && gnss.vdop.dop() <= 1.1);

    if (gnss.altitude.isValid() && gnss.altitude.age() <= NMEA_EXP_TIME) {
      snprintf(navbox21.value_str, sizeof(navbox21.value_str), "%-5.0f%c", gnss.altitude.feet(), gnss_height_ok ? ' ' : '?');
    } else {
      snprintf(navbox21.value_str, sizeof(navbox21.value_str), "--");
    }

    if (baro_chip != NULL) {
      snprintf(navbox13.value_str, sizeof(navbox13.value_str), "%.0f", Baro_temperature());
      snprintf(navbox22.value_str, sizeof(navbox22.value_str), "%.0f", Baro_pressure() / 100);
      snprintf(navbox23.value_str, sizeof(navbox23.value_str), "%.0f", Baro_altitude() * _GPS_FEET_PER_METER);
      float vertical_speed = ThisAircraft.vs / (_GPS_FEET_PER_METER * 60.0);
      // avoid printing "-0"
      if (vertical_speed > -0.5 && vertical_speed < 0.5)
        vertical_speed = 0;
      snprintf(navbox33.value_str, sizeof(navbox33.value_str), "%.0f", vertical_speed);
    } else {
      snprintf(navbox13.value_str, sizeof(navbox13.value_str), "--");
      snprintf(navbox22.value_str, sizeof(navbox22.value_str), "--");
      snprintf(navbox23.value_str, sizeof(navbox23.value_str), "--");
      snprintf(navbox33.value_str, sizeof(navbox33.value_str), "--");
    }

    if (gnss.speed.isValid() && gnss.speed.age() <= NMEA_EXP_TIME) {
      snprintf(navbox31.value_str, sizeof(navbox31.value_str), "%.0f", gnss.speed.knots());
    } else {
      snprintf(navbox31.value_str, sizeof(navbox31.value_str), "--");
    }

    if (gnss.course.isValid() && gnss.course.age() <= NMEA_EXP_TIME) {
      snprintf(navbox32.value_str, sizeof(navbox32.value_str), "%.0f", gnss.course.deg());
    } else {
      snprintf(navbox32.value_str, sizeof(navbox32.value_str), "--");
    }

    snprintf(navbox34.value_str, sizeof(navbox34.value_str), "%u", gnss.satellites.value());

    if (gnss.vdop.isValid() && gnss.vdop.age() <= NMEA_EXP_TIME) {
      snprintf(navbox42.value_str, sizeof(navbox42.value_str), "%.1f", gnss.vdop.dop());
    } else {
      snprintf(navbox42.value_str, sizeof(navbox42.value_str), "--");
    }

    if (gnss.pdop.isValid() && gnss.pdop.age() <= NMEA_EXP_TIME) {
      snprintf(navbox43.value_str, sizeof(navbox43.value_str), "%.1f", gnss.pdop.dop());
    } else {
      snprintf(navbox43.value_str, sizeof(navbox43.value_str), "--");
    }

    if (settings->mode        == SOFTRF_MODE_RECEIVER ||
        settings->rf_protocol == RF_PROTOCOL_ADSB_UAT ||
        settings->txpower     == RF_TX_POWER_OFF) {
      snprintf(navbox51.value_str, sizeof(navbox51.value_str), "%s", "OFF");
    } else {
      snprintf(navbox51.value_str, sizeof(navbox51.value_str), "%u", tx_packets_counter % 1000);
    }

    snprintf(navbox52.value_str, sizeof(navbox52.value_str), "%u", rx_packets_counter % 1000);

    snprintf(navbox53.value_str, sizeof(navbox53.value_str), "%d", ThisAircraft.rssi);
    snprintf(navbox54.value_str, sizeof(navbox54.value_str), "%d", Traffic_Count());

    EPD_Draw_NavBoxes();

    EPDTimeMarker = millis();
  }
}

#endif /* USE_EPAPER */
