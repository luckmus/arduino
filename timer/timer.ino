/**
 * для нормальной (шириной в 5 пикселей) единицы в файле MD_MAX72xx_font.cpp строку:
 * 3, 0x42, 0x7f, 0x40,  // 49 - '1'
 * заменить на:
 * 5, 0x44, 0x42, 0x7f, 0x40, 0x40,  // 49 - '1'
 */
#include <Wire.h>
#include "RTClib.h"
#include <IRremote.h>


///#include "TimerCls.h"
#include "Display.h"

int RECV_PIN = 2;

//RTC_DS1307 rtc;
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
  timer->rtc.begin();

  if (! timer->rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  irrecv.enableIRIn();
  //pinMode (timer->bPin, OUTPUT);
  dsply->init();


}

void readInput() {

  if (irrecv.decode(&results)) {

    long val = results.value;

    irrecv.resume();
    Serial.println(val, HEX);
    val = val & 0xFFFF;
    Serial.println(val, HEX);
    timer->applyInput(val);

  }

}

void loop() {
  readInput();

  dsply->show(timer);
  timer->shouldFinishWorkout();
  //timer->logger();
  //dsply->logger();
  //timer->stopTone();
  delay(50);


}
