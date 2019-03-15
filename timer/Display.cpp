#include "Display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 9;
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
int wait = 20;
String prevString;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


void Display::show(Timer *timer){
  String str = timer->getDisplayString();
  if (prevString == str){
    return;
  }
  
  prevString = str;
/*
  if ((str.toInt()==1) || (str.toInt()==2)||(str.toInt()==3)){
    timer->tTone(300);
  }
  */
  ///*
  for ( int i = 0 ; i < str.length(); i++ ) {
        matrix.fillScreen(LOW);
        matrix.drawChar(0, 0, str[i], HIGH, LOW, 1);
        matrix.write();
        delay(0);
    }
    //*/
};

void Display::logger(){
  
}

