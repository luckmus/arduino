#include <Wire.h>
#include "RTClib.h"
#include <IRremote.h>


//#include "TimerCls.h"
#include "Display.h"

int RECV_PIN = 0;

RTC_DS1307 rtc;
DateTime now;
IRrecv irrecv(RECV_PIN);
decode_results results;

Timer *timer = new Timer();
Display *dsply = new Display();

void setup() {
  Serial.begin(57600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  irrecv.enableIRIn();
  

}

int readInput(){
  if (irrecv.decode(&results)) {
    long val=results.value;
    irrecv.resume();
    Serial.println(val, HEX);
  }
  
}

void loop() {
  readInput();
  timer->applyInput(1);
  dsply->show(timer);

  timer->logger();
  dsply->logger();
   delay(1000);
   

}
