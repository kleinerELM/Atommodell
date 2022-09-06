// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

#include <Arduino.h>

#define CONTROLLER_NAME "Atommodell"

#if defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  //#include <ESPmDNS.h>
  //#include "HTTPUpdateServer.h"
  //#define HOSTIDENTIFY  "atommodell"
  //#define mDNSUpdate(c)  do {} while(0)
  //using WebServerClass = WebServer;
  //using HTTPUpdateServerClass = HTTPUpdateServer;
#endif
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"
//#include <AutoConnect.h>

// Fix hostname for mDNS. It is a requirement for the lightweight update feature.
//static const char* host = HOSTIDENTIFY "-webupdate";
#define HTTP_PORT 80


// Declare AutoConnectAux to bind the HTTPWebUpdateServer via /update url
// and call it from the menu.
// The custom web page is an empty page that does not contain AutoConnectElements.
// Its content will be emitted by ESP8266HTTPUpdateServer.
//HTTPUpdateServerClass httpUpdater;
//AutoConnectAux  update("/update", "Update");

// Declare AutoConnect and the custom web pages for an application sketch.
//AutoConnect       portal(httpServer);
//AutoConnectAux    hello;
//AutoConnectConfig Config;

//#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

// Font files are stored in SPIFFS, so load the library
#include <FS.h>

// pins & display type are defined in platformio.ini
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#include "PCF8574.h"
PCF8574 PCF[] = {PCF8574(0x25), PCF8574(0x22), PCF8574(0x20), PCF8574(0x21)};

// pins for 5V relais
#define R1 12
#define R2 14
#define R3 27
#define R4 26
int direct_to_relais[4] = { R1, R2, R3, R4 };

                    //          1           2
int element[36][2] = { {  4, R3 }, {  4, R4 }, // 1, 2

                    //          3           4           5           6           7           8           9          10
                       {  4, R1 }, {  0,  5 }, {  1,  6 }, {  1,  0 }, {  4, R2 }, {  0,  7 }, {  1,  7 }, {  0,  6 },

                    //         11          12          13          14          15          16          17          18          19
                       {  2,  5 }, {  3,  4 }, {  2,  4 }, {  3,  3 }, {  2,  3 }, {  3,  2 }, {  0,  0 }, {  1,  1 }, {  0,  1 },

                    //         20          21          22          23          24          25          26          27          28
                       {  1,  2 }, {  0,  2 }, {  1,  3 }, {  0,  3 }, {  3,  7 }, {  2,  7 }, {  3,  6 }, {  2,  6 }, {  3,  5 },

                    //         29          30          31          32          33          34          35          36
                       {  2,  1 }, {  3,  1 }, {  2,  2 }, {  1,  4 }, {  0,  4 }, {  1,  5 }, {  2,  0 }, {  3,  0 } };

String elements_short[36] = {  "H", "He", "Li", "Be",  "B",  "C",  "N",  "O",  "F",
                             "Ne", "Na", "Mg", "Al", "Si",  "P",  "S", "Cl", "Ar",
                              "K", "Ca", "Sc", "Ti",  "V", "Cr", "Mn", "Fe", "Co",
                             "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr" };

String elements_long[36] = { "Wasserstoff", "Helium", "Lithium", "Beryllium", "Bor", "Kohlenstoff", "Stickstoff", "Sauerstoff", "Fluor",
                             "Neon", "Natrium", "Magnesium", "Aluminium", "Silizium", "Phosphor", "Schwefel", "Chlor", "Argon",
                             "Kalium", "Calcium", "Scantium", "Titan", "Vanadium", "Chrom", "Mangan", "Eisen", "Cobalt",
                             "Nickel", "Kupfer", "Zink", "Gallium", "Germanium", "Arsen", "Selen", "Brom", "Krypton" };

//u_int8_t lamp_order[36] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 29, 30, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 31, 32, 33, 34, 35, 36 };

u_int8_t lamp_order[36] = { 1, 2, 5, 9, 3, 7, 4, 8, 10, 6, 12, 21, 18, 27, 14, 23, 16, 25, 29, 33, 11, 20, 28, 19, 15, 24, 13, 22, 17, 26, 31, 35, 30, 34, 32, 36 };

float elements_mass[36] = { 1.0080, 4.0026, 6.94, 9.0122, 10.81, 12.011, 14.007, 15.999, 18.998,
                            20.180, 22.990, 24.305, 26.982, 28.085, 30.974, 32.06, 35.45, 39.948,
                            39.098, 40.078, 44.954, 47.867, 50.942, 51.996, 54.938, 55.845, 58.933,
                            58.693, 63.546, 65.380, 69.723, 72.630, 74.922, 78.971, 79.904, 83.798 };

float elements_kl[36][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0.109,0,0,0}, {0.183,0,0,0}, {0.277,0,0,0}, {0.392,0,0,0}, {0.525,0,0,0}, {0.677,0,0,0},
                             {0.848,0,0,0}, {1.041,1.067,0,0}, {1.253,1.302,0,0}, {1.486,1.557,0,0}, {1.739,1.836,0,0}, {2.013,2.139,0,0}, {2.307,2.464,0,0}, {2.621,2.815,0,0}, {2.957,3.190,0,0},
                             {3.312,3.589,0,0}, {3.690,4.012,0.341,0.345}, {4.088,4.460,0.395,0.400}, {4.508,4.931,0.452,0.458}, {4.949,5.426,0.511,0.519}, {5.411,5.946,0.573,0.583}, {5.894,6.489,0.637,0.649}, {6.398,7.057,0.705,0.718}, {6.924,7.648,0.776,0.791},
                             {7.471,8.263,0.851,0.869}, {8.040,8.904,0.930,0.950}, {8.630,9.570,1.012,1.043}, {9.241,10.263,1.098,1.125}, {9.874,10.980,1.188,1.218}, {10.530,11.724,1.282,1.317}, {11.207,12.949,1.379,1.419}, {11.907,13.289,1.480,1.526}, {12.631,14.110,1.586,1.636} };

float elements_enw[36] = { 2.2, 0, 0.98, 1.57, 2.04, 2.55, 3.07, 3.58, 3.98,
                           0, 0.9, 1.31, 1.61, 1.90, 2.19, 2.58, 3.16, 0,
                           0.82, 1.00, 1.36, 1.54, 1.63, 1.66, 1.55, 1.8, 1.88,
                           1.71, 1.90, 1.65, 1.81, 2.01, 2.18, 2.55, 2.96, 0 };

// {d, s, p} - order changed for simpler display algo.
String ecn[3] = {"d", "s", "p"};
int8_t elements_ec[36][3] = { {0,1,0},{0,2,0},{0,1,0},{0,2,0},{0,2,1},{0,2,2},{0,2,3},{0,2,4},{0,2,5},
                             {0,2,6}, {0,1,0},{0,2,0},{0,2,1},{0,2,2},{0,2,3},{0,2,4},{0,2,5},
                             {0,2,6}, {0,1,0},{0,2,0},{1,2,0},{2,2,0},{3,2,0},{5,1,0},{5,2,0},{6,2,0},{7,2,0},
                             {8,2,0},{10,1,0},{10,2,0},{10,2,1},{10,2,2},{10,2,3},{10,2,4},{10,2,5},{10,2,6} };

#define shell_K  2
#define shell_L  8
#define shell_M 18
#define shell_N  8


// current atom number
int an = 0;

/* Put your SSID & Password */
const char* ssid = CONTROLLER_NAME;  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(HTTP_PORT);