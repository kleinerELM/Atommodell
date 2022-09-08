// written by Florian Kleiner 2022
// https://github.com/kleinerELM/Atommodell

// load basic hardware and PSE definitions
#include "atom_model_defs.h"

String process_element_html( int atomic_number ) {
  String element_main      = "";
  File element_file   = SPIFFS.open("/element.html");

  Serial.println("loading Element html");
  if (!element_file) {
    element_main = "Error loading template!";
  } else {
    int arr_pos = atomic_number-1;
    element_main = element_file.readString();

    element_main.replace( "%1%", String(elements_enw[arr_pos], 2U) );
    element_main.replace( "%2%", elements_short[arr_pos] );
    element_main.replace( "%3%", elements_long[arr_pos] );
    element_main.replace( "%4%", String( atomic_number ) );
    element_main.replace( "%5%", String( elements_mass[arr_pos] ) );

    String klline_names[4] = {"K<sub>&alpha;</sub>", "K<sub>&beta;</sub>", "L<sub>&alpha;</sub>", "L<sub>&beta;</sub>"};
    String line_strings[4] = {"","","",""};
    for (int i = 0; i < 4; i++) {
      if ( elements_kl[arr_pos][i] > 0 ) {
        line_strings[i] = klline_names[i] + ": " + String(elements_kl[arr_pos][i],3U);
      } else {break;}
    }
    element_main.replace( "%6%", String( line_strings[0] ) );
    element_main.replace( "%7%", String( line_strings[1] ) );
    element_main.replace( "%8%", String( line_strings[2] ) );
    element_main.replace( "%9%", String( line_strings[3] ) );

    Serial.println(elements_long[arr_pos] + " (" + elements_short[arr_pos] + ")");


    // electron configuration
    String e_config = "";
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
        e_config += "<sup>"+String(elements_ec[arr_pos][p])+"</sup>";
      }
    }

    element_main.replace( "%10%", e_config );


  }
  element_file.close();

  return element_main;
}

String SendHTML( int an ){
  String content      = "";
  File template_file  = SPIFFS.open("/PSE.html");

  Serial.println("SendHTML");
  if (!template_file) {
    content = "Error loading template!";
  } else {
    content      = template_file.readString();
    content.replace( "%1%", String(CONTROLLER_NAME) );
    content.replace( "/?e=", "http://" + local_ip.toString() + "/?e=" );
    content.replace( "%2%", process_element_html( an ) );
    if ( !animation_finished ) content += "Vorheriges Element noch nocht fertig angezeigt.";
  }
  template_file.close();

  return content;
}
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

  // element enw, right
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
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


  Serial.println("tft display ready");
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

  Serial.println("all required lapmps on");
  // make sure, all other lamps are off
  for (int i = start; i < atomic_number; i++) {
    int k = lamp_order[i]-1;
    enable_lamp( element[k][0], element[k][1], 0, electron_delay );
  }

  //delay( shell_delay*(3-shell_pos) + electron_delay * ( 18- (atomic_number - start)) );

  Serial.println("----------------");
}

void show_element_by_short( char el_short[2] ) {
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
}

void iterate_through_elements( boolean force = false ) {
    for (an = 1; an < 37; an++ ) {
      // stop animation if a client is connected
      if ( WiFi.softAPgetStationNum() == 0 || force ) {
        // show the element
        show_element( an, 1 );
        // check for serial input
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
}

void select_element_by_serial() {
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
}