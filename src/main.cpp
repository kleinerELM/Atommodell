// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

// load basic definitions and functions like animations
#include "atom_model_fkt.h"

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

  // get main html with PSE
  server.serveStatic("/", SPIFFS, "/");//.setDefaultFile("PSE.html");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("html request");

    int paramsNr = request->params();
    for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        if ( p->name() == "e" ) {
          an = p->value().toInt();
        }
    }
    if (!( an > 0 && an < 37 ) ) {
      an = 1;
    }
    request->send(200, "text/html", SendHTML( an ));
    // doing the animation here causes watchdog issues with a delay
    // -> do the animation in the main loop
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time
    an_wifi = an;

  });

  // Ajax request for element change
  server.on("/element", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("ajax request");
    if ( animation_finished ) {
      int paramsNr = request->params();
      int an = 0;
      for(int i=0;i<paramsNr;i++){
          AsyncWebParameter* p = request->getParam(i);
          if ( p->name() == "e" ) {
            an = p->value().toInt();
          }
      }
      if (!( an > 0 && an < 37 ) ) {
        an = 1;
      }
     request->send(200, "text/html", process_element_html( an ));
      // doing the animation here causes watchdog issues with a delay
      // -> do the animation in the main loop
      an_wifi = an;
    }

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
    Serial.println("\nSPIFFS initialisation failed!");
  } else {
    Serial.println("\nSPIFFS available!");

    // ESP32 will crash if any of the fonts are missing
    bool font_missing = false;
    if (SPIFFS.exists("/NotoSansBold36.vlw")    == false) font_missing = true;

    if (font_missing) {
      Serial.println("Font missing in SPIFFS, did you upload it?");
    }
    else Serial.println("Fonts found.");
  }


  Serial.println();
  Serial.println("-- Setup done --");
  Serial.println();

  delay(1000);
}

/*
 * Main loop
 */
void loop() {
  // standard function, iterate through elements

  if ( WiFi.softAPgetStationNum() == 0 ) {
    iterate_through_elements();

    if ( input > 0 ) {
      select_element_by_serial();
    } else {
      Serial.println("\n---loop done---");
      delay(5000);
    }
    input = 0;
  } else {
    if ( an_wifi > 0 && an_wifi < 37 ) {
      animation_finished = false;
      Serial.print( "Request by wifi detected: " );
      Serial.print( elements_short[an_wifi-1] );
      show_element( an_wifi, 1, 50 );
      an_wifi = 0;
      animation_finished = true;
      Serial.println("\n---loop done---");
    }
    //Serial.println(an_wifi);
  }
}