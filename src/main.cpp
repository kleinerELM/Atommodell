// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266HTTPUpdateServer.h>
  #define HOSTIDENTIFY  "esp8266"
  #define mDNSUpdate(c)  do { c.update(); } while(0)
  using WebServerClass = ESP8266WebServer;
  using HTTPUpdateServerClass = ESP8266HTTPUpdateServer;
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
  #include "HTTPUpdateServer.h"
  #define HOSTIDENTIFY  "atommodell"
  #define mDNSUpdate(c)  do {} while(0)
  using WebServerClass = WebServer;
  using HTTPUpdateServerClass = HTTPUpdateServer;
#endif
#include <WiFiClient.h>
#include <AutoConnect.h>

// Fix hostname for mDNS. It is a requirement for the lightweight update feature.
static const char* host = HOSTIDENTIFY "-webupdate";
#define HTTP_PORT 80

// ESP8266WebServer instance will be shared both AutoConnect and UpdateServer.
WebServerClass  httpServer(HTTP_PORT);

#define USERNAME "Florian"           //*< Replace the actual username you want */
#define PASSWORD "legitupdatepass"   //*< Replace the actual password you want */

// Declare AutoConnectAux to bind the HTTPWebUpdateServer via /update url
// and call it from the menu.
// The custom web page is an empty page that does not contain AutoConnectElements.
// Its content will be emitted by ESP8266HTTPUpdateServer.
HTTPUpdateServerClass httpUpdater;
AutoConnectAux  update("/update", "Update");

// Declare AutoConnect and the custom web pages for an application sketch.
AutoConnect     portal(httpServer);
AutoConnectAux  hello;

static const char AUX_AppPage[] PROGMEM = R"(
{
  "title": "Atommodell",
  "uri": "/",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "<h2>Atommodell</h2>",
      "style": "text-align:center;color:#2f4f4f;padding:10px;"
    },
    {
      "name": "content",
      "type": "ACText",
      "value": "Atommodell NodeMCU32s."
    }
  ]
}
)";


//#define AA_FONT_SMALL "NotoSansBold15"
#define AA_FONT_LARGE "NotoSansBold36"

// Font files are stored in SPIFFS, so load the library
#include <FS.h>

// pins & display type are defined in platformio.ini
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#include "PCF8574.h"
/*PCF8574 PCF_1(0x25);
PCF8574 PCF_2(0x22);
PCF8574 PCF_3(0x20);
PCF8574 PCF_4(0x23);*/
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

float elements_mass[36] = { 1.0080, 4.0026, 6.94, 9.0122, 10.81, 12.011, 14.007, 15.999, 18.998,
                            20.180, 22.990, 24.305, 26.982, 28.085, 30.974, 32.06, 35.45, 39.948,
                            39.098, 40.078, 44.954, 47.867, 50.942, 51.996, 54.938, 55.845, 58.933,
                            58.693, 63.546, 65.380, 69.723, 72.630, 74.922, 78.971, 79.904, 83.798 };

#define shell_K  2
#define shell_L  8
#define shell_M 18
#define shell_N  8

void enable_lamp( int pcf_id, int pos, int state, int wait ) {
    if ( pcf_id != 4 ) {
      if ( state == 1 ) {
        PCF[pcf_id].write(pos, 1);
      } else {
        PCF[pcf_id].write(pos, 0);
      }
    } else {
      if ( state == 1 ) {
        digitalWrite(pos, HIGH);
      } else {
        digitalWrite(pos, LOW);
      }
    }
    delay( wait );

}

void show_element( int atomic_number, int animation = 0, int electron_delay = 100 ) {
  Serial.println();
  Serial.println("----------------");
  Serial.println("Zeige " + elements_long[atomic_number-1] + " / " + elements_short[atomic_number-1] + " (#" + String( atomic_number ) + ")");

  for (int k = 0; k < 36; k++) {
    enable_lamp( element[k][0], element[k][1], 1, 0 );
  }

  tft.fillScreen(TFT_BLACK);

  tft.drawRect(1,1,tft.width()-1,tft.height()-1,TFT_WHITE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(5,5,1);
  tft.println(String(atomic_number));

  tft.setTextDatum(TR_DATUM); // Top Centre datum
  String mass = String(elements_mass[atomic_number-1],4U);
  tft.setCursor(tft.width()-5-tft.textWidth(mass,1),5,1);
  tft.println(mass);

  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font colour and the background colour

  tft.setCursor((tft.width()-tft.textWidth(elements_long[atomic_number-1], 1))/2,75,1);
  tft.println(elements_long[atomic_number-1]);


  tft.setTextDatum(TC_DATUM); // Top Centre datum
  int xpos = tft.width() / 2; // Half the screen width
  tft.loadFont(AA_FONT_LARGE); // Load another different font

  tft.setTextColor(TFT_GREEN, TFT_BLACK); // Change the font colour and the background colour
  tft.drawString(elements_short[atomic_number-1] , xpos-3, 40);
  tft.unloadFont(); // Remove the font to recover memory used

  delay(50);

  int start = 0;
  if ( animation == 1 ) {
    int shell_delay = 500;
    if ( atomic_number > shell_K ) {
      start = shell_K;
      for (int k = 0; k < start; k++) {
        enable_lamp( element[k][0], element[k][1], 0, 0 );
      }
      delay(shell_delay);

      if ( atomic_number > shell_L + shell_K) {
        start += shell_L;
        for (int k = 0; k < start; k++) {
          enable_lamp( element[k][0], element[k][1], 0, 0 );
        }
        delay(shell_delay);

        if ( atomic_number > shell_M + shell_L + shell_K ) {
          start += shell_M;
          for (int k = 0; k < start; k++) {
            enable_lamp( element[k][0], element[k][1], 0, 0 );
          }
          delay(shell_delay);
        }
      }
    }
  }

  for (int k = start; k < atomic_number; k++) {
    enable_lamp( element[k][0], element[k][1], 0, electron_delay );
  }

  Serial.println("----------------");
}

void setup() {
  delay(1000);
  // debug-serial-connection
  Serial.begin(9600);

  Serial.println();
  Serial.println();
  Serial.println("---Atommodell---");
  Serial.println("last build: " __DATE__ ", " __TIME__);
  Serial.println();


  Serial.println("-WiFi-");

  // Prepare the ESP8266HTTPUpdateServer
  // The /update handler will be registered during this function.
  httpUpdater.setup(&httpServer, USERNAME, PASSWORD);

  // Load a custom web page for a sketch and a dummy page for the updater.
  hello.load(AUX_AppPage);
  portal.join({ hello, update });
  if (portal.begin()) { // code stops if not able to connect to wifi... this sucks!
    Serial.println("1");
    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.println(" WiFi connected!");
        Serial.printf( " HTTPUpdateServer ready: http://%s.local/update\n", host);
        Serial.println();
    }
    else
      Serial.println("Error setting up MDNS responder");
  }

  Serial.println("-PCF8574-");
  Serial.print("PCF8574_LIB_VERSION:\t");
  Serial.println(PCF8574_LIB_VERSION);

  int j = 0;
  for (int i = 0 ; i < 4 ; i++) {
    if (!PCF[i].begin()) {
      Serial.print("could not initialize PCF: ");
      Serial.println(i);
    } else if (!PCF[i].isConnected()) {
      Serial.print("could not connect PCF: ");
      Serial.println(i);
    } else {
      j++;
    }
  }
  Serial.print( "finished loading PCF8574: " );
  Serial.println(j);

  for (int i = 0 ; i < 4 ; i++) {
    pinMode(direct_to_relais[i], OUTPUT);
    digitalWrite(direct_to_relais[i], LOW);
  }

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
  }
  Serial.println("\r\nSPIFFS available!");

  // ESP32 will crash if any of the fonts are missing
  bool font_missing = false;
  if (SPIFFS.exists("/NotoSansBold15.vlw")    == false) font_missing = true;
  if (SPIFFS.exists("/NotoSansBold36.vlw")    == false) font_missing = true;

  if (font_missing)
  {
    Serial.println("\r\nFont missing in SPIFFS, did you upload it?");
  }
  else Serial.println("\r\nFonts found OK.");


  Serial.println();
  Serial.println("-- Setup done --");
  Serial.println();

  delay(1000);
}

void loop() {
  /*
  for (int k = 0; k < 2; k++ ) {
    for (int i = 0; i < 8; i++) {
      Serial.println(String(k) + ", " + String(i));
      PCF[k].write(i, 0); // an
      PCF[k+2].write(i, 0); // an
      delay(2000);
      while(Serial.available() == 0) {
      }
      int mydata = Serial.read();
      PCF[k].write(i, 1); // aus
      PCF[k+2].write(i, 1); // aus

    }
    delay(500);
  }
  delay(1000);
  */
  // standard function, iterate through elements






  int input = 0;
  int an = 0;
  for (an = 1; an < 37; an++ ) {
    show_element( an, 1 );
    while(Serial.available() == 0) {
      delay(1000);
      break;
    }
    input = Serial.read();
    Serial.println(input);
    if ( input > 0 ) {
      an = 36;
    } else {
      input = 0;
    }
  }

  if ( input > 0 ) {
    char line[3];
    char el_short[2];
    int done  = 0;
    int count = 0;

    Serial.println( "Welches Element soll angezeigt werden? [c] zum Abbrechen" );
    while (done == 0) {
      if (Serial.available() > 0) {
          line[count] = (char)Serial.read();

          if (line[count] == '\r'){
            done = 1;
          } else {
            el_short[count] = line[count];
            if (count == 1) {
              done = 1;
            } else {
              el_short[count+1]= '\0';
            }
          }
          count += 1;
      }
    }

    if (el_short != "c") {
      bool found = false;
      for (an = 1; an < 37; an++ ) {
        if( elements_short[an-1] == el_short) {
          found = true;
          show_element( an, 0, 50 );
          delay(10000);
          break;
        }
      }

      if (!found) {
        Serial.println("ELement '" + String(el_short) + "' nicht gefunden! Verfügbare Elemente:" );
        for (an = 1; an < 37; an++ ) {
          Serial.print(elements_short[an-1] + ' ');
        }
      Serial.println("");
      }
    } else {
      Serial.println("canceling operation. Back to loop animation.");
    }

  } else {
    Serial.println();
    Serial.println("---loop done---");
    delay(5000);
  }
}