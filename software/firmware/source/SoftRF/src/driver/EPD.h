/*
 * EPDHelper.h
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

#ifndef EPDHELPER_H
#define EPDHELPER_H

#if defined(USE_EPAPER)
#define ENABLE_GxEPD2_GFX       1
#include <GxEPD2_BW.h>
#include "U8g2_for_Adafruit_GFX.h"

extern const GFXfont FreeMono9pt7b PROGMEM;
extern const GFXfont FreeMonoBold9pt7b PROGMEM;
//extern const GFXfont FreeSerif9pt7b PROGMEM;
extern const GFXfont FreeSerifBold9pt7b PROGMEM;

extern const GFXfont FreeMonoBold12pt7b PROGMEM;
extern const GFXfont OpenSansCondensed12pt7b PROGMEM;
extern const GFXfont FreeSerifBold12pt7b PROGMEM;

//extern const GFXfont RobotoBold15pt7b PROGMEM;
//extern const GFXfont RobotoCondensedBold15pt7b PROGMEM;
extern const GFXfont OpenSansCondensed15pt7b PROGMEM;

//extern const GFXfont FreeMono18pt7b PROGMEM;
extern const GFXfont FreeMonoBold18pt7b PROGMEM;
//extern const GFXfont FreeSansBold18pt7b PROGMEM;

extern const GFXfont FreeMonoBold24pt7b PROGMEM;

extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

#endif /* USE_EPAPER */

#include "EPDdatabox.h"

#define EPD_EXPIRATION_TIME     5 /* seconds */

#define NO_DATA_TEXT            "NO DATA"
#define NO_FIX_TEXT             "NO FIX"

#define NAVBOX1_TITLE           "ACFTS"
#define NAVBOX2_TITLE           "BAT"
#define NAVBOX3_TITLE           "ID"
#define NAVBOX4_TITLE           "PROTOCOL"
#define NAVBOX5_TITLE           "RX"
#define NAVBOX6_TITLE           "TX"

#define isTimeToEPD()           (millis() - EPDTimeMarker > 1000)
#define maxof2(a,b)             (a > b ? a : b)

#define EPD_RADAR_V_THRESHOLD   50      /* metres */

#define TEXT_VIEW_LINE_LENGTH   13     /* characters */
#define TEXT_VIEW_LINE_SPACING  12     /* pixels */
#define INFO_1_LINE_SPACING     7      /* pixels */


//#define EPD_HIBERNATE         {}
#define EPD_HIBERNATE           display->hibernate()
//#define EPD_HIBERNATE         display->powerOff()

#ifndef EPD_POWEROFF
//#define	EPD_POWEROFF		      {}
#define EPD_POWEROFF            display->powerOff()
#endif /* EPD_POWEROFF */

enum
{
	EPD_UPDATE_NONE = 0,
	EPD_UPDATE_SLOW,
	EPD_UPDATE_FAST
};

enum
{
	VIEW_MODE_STATUS = 0,
	VIEW_MODE_RADAR,
	VIEW_MODE_TEXT,
	VIEW_MODE_BARO,
	VIEW_MODE_TIME,
	VIEW_MODE_IMU,

	VIEW_MODES_COUNT,
};

/*
 * 'Radar view' scale factor (outer circle diameter)
 *
 * Metric and Mixed:
 *  LOWEST - 60 KM diameter (30 KM radius)
 *  LOW    - 10 KM diameter ( 5 KM radius)
 *  MEDIUM -  4 KM diameter ( 2 KM radius)
 *  HIGH   -  2 KM diameter ( 1 KM radius)
 *
 * Imperial:
 *  LOWEST - 30 NM diameter ( 15 NM radius)
 *  LOW    -  5 NM diameter (2.5 NM radius)
 *  MEDIUM -  2 NM diameter (  1 NM radius)
 *  HIGH   -  1 NM diameter (0.5 NM radius)
 */
enum
{
	ZOOM_LOWEST,
	ZOOM_LOW,
	ZOOM_MEDIUM,
	ZOOM_HIGH
};

enum
{
	UNITS_METRIC,
	UNITS_IMPERIAL,
	UNITS_MIXED     // almost the same as metric, but all the altitudes are in feet
};

enum
{
	PROTOCOL_NONE,
	PROTOCOL_NMEA, /* FTD-12 */
	PROTOCOL_GDL90,
	PROTOCOL_MAVLINK_1,
	PROTOCOL_MAVLINK_2,
	PROTOCOL_D1090,
	PROTOCOL_UATRADIO
};

enum
{
	ID_REG,
	ID_TAIL,
	ID_MAM,
	ID_TYPE
};

enum
{
	VOICE_OFF,
	VOICE_1,
	VOICE_2,
	VOICE_3
};

enum
{
	ANTI_GHOSTING_OFF,
	ANTI_GHOSTING_AUTO,
	ANTI_GHOSTING_2MIN,
	ANTI_GHOSTING_5MIN,
	ANTI_GHOSTING_10MIN
};

enum
{
	TRAFFIC_FILTER_OFF,
	TRAFFIC_FILTER_500M,
	TRAFFIC_FILTER_1500M
};

enum
{
	DB_NONE,
	DB_AUTO,
	DB_FLN,
	DB_OGN,
	DB_ICAO
};

enum
{
	ROTATE_0,
	ROTATE_90,
	ROTATE_180,
	ROTATE_270
};

typedef struct UI_Settings {
    uint8_t  adapter;

    uint8_t  connection:4;
    uint8_t  units:2;
    uint8_t  zoom:2;

    uint8_t  protocol;
    uint8_t  baudrate;
    char     server  [18];
    char     key     [18];

    uint8_t  rotate:2;
    uint8_t  orientation:1;
    uint8_t  adb:3;
    uint8_t  idpref:2;

    uint8_t  vmode:3;
    uint8_t  voice:2;
    uint8_t  aghost:3;

    uint8_t  filter:4;
    uint8_t  power_save:4;

    uint32_t team;

    uint8_t  resvd12;
    uint8_t  resvd3;
    uint8_t  resvd4;
    uint8_t  resvd5;
    uint8_t  resvd6;
    uint8_t  resvd7;
    uint8_t  resvd8;
    uint8_t  resvd9;
} __attribute__((packed)) ui_settings_t;


bool EPD_setup(bool);
void EPD_loop();
void EPD_fini(int, bool);
void EPD_info1();
void EPD_info2(int, char *, char *, char *);

void EPD_Mode();
void EPD_Up();
void EPD_Down();
void EPD_Message(const char *, const char *);

void EPD_status_setup();
void EPD_status_loop();
void EPD_status_next();
void EPD_status_prev();

void EPD_radar_setup();
void EPD_radar_loop();
void EPD_radar_zoom();
void EPD_radar_unzoom();

void EPD_text_setup();
void EPD_text_loop();
void EPD_text_next();
void EPD_text_prev();

void EPD_baro_setup();
void EPD_baro_loop();
void EPD_baro_next();
void EPD_baro_prev();

void EPD_imu_setup();
void EPD_imu_loop();
void EPD_imu_next();
void EPD_imu_prev();

void EPD_time_setup();
void EPD_time_loop();
void EPD_time_next();
void EPD_time_prev();

#if defined(USE_EPAPER)
EPD_Task_t EPD_Task(void *);
extern GxEPD2_GFX *display;
#endif /* USE_EPAPER */

extern unsigned long EPDTimeMarker;
extern bool EPD_vmode_updated;
extern uint16_t EPD_pages_mask;
extern volatile uint8_t EPD_update_in_progress;
extern ui_settings_t ui_settings;
extern ui_settings_t *ui;

#endif /* EPDHELPER_H */
