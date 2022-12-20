//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef Settings_h
#define Settings_h

#define PERFECT_LIGHTS_CD_DEVICE_USER_ID "62affe0601b993743c0ebe4a"
#define PERFECT_LIGHTS_CD_DEVICE_ID "548c719e-f90b-5e0a-8bcc-9bc8a6d82f42"
#define PERFECT_LIGHTS_CD_MQTT_USERNAME "j!6d&DHqz8QcJpcH"
#define PERFECT_LIGHTS_CD_MQTT_PASSWORD "cPb6KSNGeA!6@DJc"
#define PERFECT_LIGHTS_CD_MQTT_HOST "mqtt.oguzkagan.xyz"
#define PERFECT_LIGHTS_CD_MQTT_PORT 1883

#define TIME_BETWEEN_SYNC_REQUESTS_MS 1000

#define HTTP_USERNAME "admin"
#define HTTP_PASSWORD "pass"

#define LOGIN_REQUIRED_CONTROL_PANEL
#define LOGIN_REQUIRED_UPDATE

#define WARM_K 3000
#define COLD_K 9000

#define MAX_WEB_SOCKET_CLIENT 4

//#define DISABLE_POT_CONTROL
//#define DISABLE_DISPLAY

//#define REVERSE_POT_1 960
//#define REVERSE_POT_2 960
#define POT_UPDATE_TRIGGER 2
#define POT_UNLOCK_TRIGGER 5
#define POT_LOCK_TIME 10000

#define ANALOG_READ_INTERVAL 5
#define NTC_DISPLAY_UPDATE_INTERVAL 1000
#define LED_DISPLAY_UPDATE_INTERVAL 100
//#define DISPLAY_REDRAW_INTERVAL 1000

#define ANALOG_INPUT A0
#define Cold_Led_Pin D1
#define Warm_Led_Pin D2
#define ANALOG_MULTIPLEXER_A D3
#define ANALOG_MULTIPLEXER_B D4
#define ANALOG_MULTIPLEXER_INH D0
#define SH_CP D5 // clockPin
#define DS D7    // dataPin
#define ST_CP D8 // larchPin

#define JSON_BUFFER 1024
#define JSON_IN_BUFFER 256

#define HEAT_PROTECTION_MAX_TEMP 70

//#define DEBUG_MODE

//#define PRINT_LITTLEFS_DEBUG
//#define PRINT_WEBSOCKET_DEBUG
//#define PRINT_WEBSOCKET_HIGH_DEBUG
//#define ANALOG_READER_DEBUG
//#define PRINT_SERVER_DEBUG
//#define PRINT_WIFI_DEBUG
//#define PRINT_TEMP_DEBUG

// todo add a feature that  cuts power if heat goes up to much
//  stein-hart equation values
#define val_A 0.0033540164346805299346
#define val_B 0.00039935076695237303774
#define val_C (-0.000091951591249617820454)
#define val_D (-0.000044278444660598605357)
// end of stein-hart equation values

#define RES_OHM 4700

#define USE_WS_UPDATE_LOOP
#define WS_UPDATE_EVERY_X_MILLIS 1000

#define AP_SSID "PerfectLights"
#define AP_PASS "123456789"

#endif