#include "Display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 9;
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
int wait = 20;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


void Display::show(Timer *timer){
  String str = timer->getDisplayString();
  for ( int i = 0 ; i < str.length(); i++ ) {
        matrix.fillScreen(LOW);
        matrix.drawChar(0, 0, str[i], HIGH, LOW, 1);
        matrix.write();
        //delay(200);
    }
};

void Display::logger(){
  
}

