#include "Display.h"
/*
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
*/
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    9

// Hardware SPI connection

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
String prevString;

void Display::show(Timer *timer){
  String str = timer->getDisplayString();

  if (prevString == str){    return;  }
  prevString = str;

  P.displayText(str.c_str(), PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  
  P.displayAnimate();
  }


void Display::init(){
  P.begin();
}
void Display::logger(){
  
}

