// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

// load basic definitions and functions like animations
#include "atom_model_fkt.h"
/*
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
*/


void setup() {
  delay(1000);
  // debug-serial-connection
  Serial.begin(9600);

  Serial.println();
  Serial.println();
  Serial.println("---" CONTROLLER_NAME "---");
  Serial.println("last build: " __DATE__ ", " __TIME__);
  Serial.println();

  Serial.println("-WiFi-");

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  //server.on("/", handle_OnConnect);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    int paramsNr = request->params();
    Serial.println(paramsNr);

    for(int i=0;i<paramsNr;i++){
        //handle_OnConnect();
        AsyncWebParameter* p = request->getParam(i);
        Serial.print("Param name: ");
        Serial.println(p->name());
        Serial.print("Param value: ");
        Serial.println(p->value());
        Serial.println("------");
    }

    request->send(200, "text/html", SendHTML());
  });

  //server.on("/led1on", handle_led1on);
  //server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  Serial.println("HTTPServer ready: http://" + local_ip.toString() + "/update" );

/*
  // Prepare the ESP8266HTTPUpdateServer
  // The /update handler will be registered during this function.
  //httpUpdater.setup(&httpServer, USERNAME, PASSWORD);

  Config.autoReset = false;     // Not reset the module even by intentional disconnection using AutoConnect menu.
  Config.autoReconnect = false;  // Reconnect to known access points.
  //Config.reconnectInterval = 6; // Reconnection attempting interval is 3[min].
  Config.retainPortal = true;   // Keep the captive portal open.
  portal.config(Config);
  // deleteAllCredentials();

  // Load a custom web page for a sketch and a dummy page for the updater.
  //hello.load(AUX_AppPage);
  //portal.join({ hello, update });
  //if (portal.begin()) { // code stops if not able to connect to wifi... this sucks!
    if (MDNS.begin(host)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.println(" WiFi connected!?");
        Serial.println(" HTTPUpdateServer ready: http://" + WiFi.localIP().toString() + "/update" );
        Serial.println();
    }
    else
      Serial.println("Error setting up MDNS responder");
  //}
*/

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
    digitalWrite(direct_to_relais[i], HIGH);
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

  if (font_missing) {
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
  for (an = 1; an < 37; an++ ) {
    show_element( an, 1 );
    while(Serial.available() == 0) {
      delay(1000);
      break;
    }
    input = Serial.read();
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