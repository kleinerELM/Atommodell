// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

// load basic hardware and PSE definitions
#include "atom_model_defs.h"

String SendHTML(){
  String content = "";
  File template_file = SPIFFS.open("/PSE.html");
  if (!template_file) {
    content = "Error loading template!";
  } else {
    content = template_file.readString();

    content.replace( "%1%", String(CONTROLLER_NAME) );
    content.replace( "%2%", elements_long[an-1] + " (" + elements_short[an-1] + ")" );
    content.replace( "%3%", String( an ) );
  }
  template_file.close();
/*
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>"+String(CONTROLLER_NAME)+"</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>"+String(CONTROLLER_NAME)+"</h1>\n";
  ptr +="<h3>" + elements_long[an-1] + "(" + elements_short[an-1] + ")</h3>\n";
  ptr += "Elementnummer: " + String( an ) + "<br>";
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  ptr +="<script>setTimeout(function () {location.reload;}, 2000)</script>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  */
  return content;
}

/*
void handle_OnConnect() {
  Serial.println("blubb");
  server.send(200, "text/html", SendHTML());
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}*/



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
    //server.handleClient();
}

void show_element( int atomic_number, int animation = 0, int electron_delay = 100 ) {
  int arr_pos = atomic_number-1;
  int shell_delay = 500;
  int fontsize = 1;
  int padding = 5;
  int xpos = padding;
  int ypos = padding;

  Serial.println();
  Serial.println("----------------");
  Serial.println("Zeige " + elements_long[arr_pos] + " / " + elements_short[arr_pos] + " (#" + String( atomic_number ) + ")");

  for (int k = 0; k < 36; k++) {
    enable_lamp( element[k][0], element[k][1], 1, 0 );
  }

  tft.fillScreen(TFT_BLACK);

  tft.drawRect(1,1,tft.width()-1,tft.height()-1,TFT_WHITE);

  u_int8_t char_width = tft.textWidth(" ", fontsize);
  // atomic number, top left
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(padding,padding,fontsize);
  tft.println(String(atomic_number));

  // element mass, top right
  String mass = String(elements_mass[arr_pos],4U);
  tft.setCursor(tft.width()-tft.textWidth(mass,1)-padding,padding,1);
  tft.println(mass);

  // element name, center
  ypos = 75;
  tft.setCursor((tft.width()-tft.textWidth(elements_long[arr_pos], fontsize))/2,ypos,fontsize);
  tft.println(elements_long[arr_pos]);

  // electron configuration
  String e_config = "";
  String pos = "";
  int glyph_width = tft.textWidth("a"); // font is monospace, therefore constant
  // {d, s, p} - order changed for simpler display algo.
  int ec_pos[3] = {3,1,2};
  if ( atomic_number > 2 ) {
    ec_pos[1] += 1;
    e_config = "[He] ";
    if ( atomic_number > 10) {
      ec_pos[1] += 1; // s
      ec_pos[2] += 1; // p
      e_config = "[Ne] ";
      if ( atomic_number > 18 ) {
        ec_pos[1] += 1; // s
        ec_pos[2] += 1; // p
        e_config = "[Ar] ";
      }
    }
  }
  u_int8_t ec_coord[3] = {0,0,0};
  for (int p = 0; p < 3; p++) {
    if (elements_ec[arr_pos][p] > 0 ) {
      e_config += String(ec_pos[p])+ecn[p];
      ec_coord[p] = e_config.length()*char_width;
      e_config += (elements_ec[arr_pos][p] > 9) ? "  ": " " ;
    }
  }

  ypos = 90;
  xpos = (int)((tft.width()-tft.textWidth(e_config, fontsize))/2);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK); // Change the font colour and the background colour
  tft.setCursor(xpos,ypos,fontsize);
  tft.println( e_config );

  tft.setTextColor(TFT_RED, TFT_BLACK, false); // Change the font colour and the background colour
  for (int p = 0; p < 3; p++) {
    tft.setCursor(xpos+ec_coord[p], ypos-4,fontsize);
    if (elements_ec[arr_pos][p] > 0) tft.print( String(elements_ec[arr_pos][p]) );
  }


  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // element enw, right
  if (elements_enw[arr_pos] > 0) {
    String enw = String(elements_enw[arr_pos], 2U);
    ypos = 108;
    tft.setCursor(tft.width()-tft.textWidth(enw)-padding,ypos,fontsize);
    tft.println( enw );
  }

  // k alpha, bottom left
  ypos = 100;
  String klline_names[4] = {"Ka", "Kb", "La", "Lb"};
  for (int i = 0; i < 4; i++) {
    if (i>1) {
      // L-lines are less relevant for Elements below atomic number of 52
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    }
    ypos += 8;
    tft.setCursor(padding,ypos,fontsize);
    String kllines = klline_names[i] + ": ";
    if ( elements_kl[arr_pos][i] > 0 ) {
      kllines += String(elements_kl[arr_pos][i],3U);
      tft.println(kllines);
    } else {
      break;
    }
  }

  // colored element symbol
  tft.setTextDatum(TC_DATUM); // Top Centre datum
  xpos = tft.width() / 2; // Half the screen width
  tft.loadFont(AA_FONT_LARGE); // Load another different font

  tft.setTextColor(TFT_GREEN, TFT_BLACK); // Change the font colour and the background colour
  tft.drawString(elements_short[arr_pos] , xpos-3, 35);
  tft.unloadFont(); // Remove the font to recover memory used


  delay(50);

  // start lamp animation
  int start = 0;
  int shell_pos = 0;
  if ( animation == 1 ) {
    if ( atomic_number > 2 ) {
      start = 2;
      for (int i = 0; i < start; i++) {
        int k = lamp_order[i]-1;
        enable_lamp( element[k][0], element[k][1], 0, 0 );
      }
      shell_pos++;
      delay(shell_delay);

      if ( atomic_number > 10) {
        start += 8;
        for (int i = 0; i < start; i++) {
          int k = lamp_order[i]-1;
          enable_lamp( element[k][0], element[k][1], 0, 0 );
        }
        shell_pos++;
        delay(shell_delay);

        if ( atomic_number > 18 ) {
          start += 8;
          for (int i = 0; i < start; i++) {
            int k = lamp_order[i]-1;
            enable_lamp( element[k][0], element[k][1], 0, 0 );
          }
          shell_pos++;
          delay(shell_delay);

          if ( atomic_number > 28 ) {
            start += 10;
            for (int i = 0; i < start; i++) {
              int k = lamp_order[i]-1;
              enable_lamp( element[k][0], element[k][1], 0, 0 );
            }
            shell_pos++;
            delay(shell_delay);

            if ( atomic_number > 30 ) {
              start += 2;
              for (int i = 0; i < start; i++) {
                int k = lamp_order[i]-1;
                enable_lamp( element[k][0], element[k][1], 0, 0 );
              }
              shell_pos++;
              delay(shell_delay);
            }
          }
        }
      }
    }
  }

  for (int i = start; i < atomic_number; i++) {
    int k = lamp_order[i]-1;
    enable_lamp( element[k][0], element[k][1], 0, electron_delay );
  }

  delay( shell_delay*(3-shell_pos) + electron_delay * ( 18- (atomic_number - start)) );

  Serial.println("----------------");
}