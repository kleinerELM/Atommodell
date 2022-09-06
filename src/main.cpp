// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

// load basic definitions and functions like animations
#include "atom_model_fkt.h"

int input = 0;
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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    Serial.println("blubb");
    int paramsNr = request->params();

    for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        Serial.print("Parameter: ");
        Serial.println(p->name());
        Serial.println(p->value());
        Serial.println("------");

        if ( p->name() == "e" ) {
          an = p->value().toInt();
          if ( an > 0 && an < 37 ) {
            input = 10;
            show_element( an, 0, 50 );
          }
        }
    }

    Serial.println("html request");
    request->send(200, "text/html", SendHTML());
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.println("HTTPServer ready: http://" + local_ip.toString() + "/update" );

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

  if ( WiFi.softAPgetStationNum() == 0 ) {
    for (an = 1; an < 37; an++ ) {
      if ( WiFi.softAPgetStationNum() == 0 ) {

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
      } else {
        Serial.print( WiFi.softAPgetStationNum() );
        Serial.println( " client(s) are connected! Stopping animation." );
        break;
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
        show_element_by_short( el_short );

      } else {
        Serial.println("canceling operation. Back to loop animation.");
      }

    } else {
      Serial.println();
      Serial.println("---loop done---");
      delay(5000);
    }
    input = 0;
  }
  delay(50);
}