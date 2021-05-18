#include <Wire.h>

#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// define some values used by the panel and buttons 
int lcd_key = 0;
int adc_key_in = 0;
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

#define DEF_DELAY 1

////hall
const int ledPin = 13;//the led attach to pin13
int sensorPin = 5; // select the input pin for the potentiometer
int digitalPin=2; //D0 attach to pin2
int sensorValue = 0;// variable to store the value coming from A0
boolean digitalValue=0;// variable to store the value coming from pin2

////hall

float spinDistance = 0.6;

enum LCD_SHOW_MODE {

  m_WAIT

};

enum TM_STATE {

  s_WAIT,
  s_STARTING, //переходит после нажатия кнопки старт
  s_STARTED,  //Наступает после первого оборота
  s_PAUSED

};

// read the buttons 
int read_LCD_buttons() {
  //return btnNONE;
  adc_key_in = analogRead(0); // read the value from the sensor  
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741 
  // we add approx 50 to those values and check to see if we are close 
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result 
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 195) return btnUP;
  if (adc_key_in < 380) return btnDOWN;
  if (adc_key_in < 555) return btnLEFT;
  if (adc_key_in < 790) return btnSELECT;
  return btnNONE; // when all others fail, return this... 
}
class LCD {
  LCD_SHOW_MODE showMode = m_WAIT;
  String printData[2] = {
    String(""),
    String("")
  };
  public:
    LCD() {

    }
    
  

  void repaint(String line1, String line2) {
    lcd.clear();
    lcd.setCursor(0,0);            // move cursor to second line "1" and 9 spaces over
    lcd.print(line1);
    lcd.setCursor(0,1); 
    lcd.print(line2);
  }
};


class Meter {
  TM_STATE state = s_WAIT;
  unsigned long startTime=0;
  //время в течении которого таймер был на паузе. это значение будет вычититься из общего времени
  unsigned long totalPauseTime=0;
  unsigned long lastSpinTime = 0;
  unsigned long lastRepaint = 0;
  byte lastHallVal;
  int prevBtn = btnNONE;
  unsigned long prevBtnTime = 0;
  unsigned long spinCnt = 0;
  //необходимое количество боротов для измерения средней скорости
  byte spinCountForAWGCalc = 3;
  byte awgSpinCounter;
  float avgSpeed = 0;
  float curSpeed = 0;
  boolean halfSpin = false;
  
  //время последнего измерения средней скорости
  long lastMeasTime;
  LCD *lcdOb;
  public :
  Meter(LCD *val){
    lcdOb = val;
  }

  void start(){
    state = s_STARTING;
    startTime = millis();
    lastMeasTime = startTime;
    awgSpinCounter = spinCountForAWGCalc;
    spinCnt = 0;
    curSpeed = 0;
    avgSpeed = 0;
    totalPauseTime=0;
    Serial.println("start");
  }

  void addPauseTime(int t){
    totalPauseTime += t;
  }

  boolean isPaused(){
    return state == s_PAUSED;
  }
  
  void readHallValue(){
    if (isPaused()){
        return;
    }
    byte rh = readHall();
    if (rh != lastHallVal){
      lastHallVal = rh;
      halfSpin = !halfSpin;
      if (halfSpin){
        spin();
      }
    }else if (millis()-lastSpinTime>3000){
      calcSpeed();
      curSpeed = 0;
    }
    
  }
  void spin(){
    Serial.print("  S P I N   ");
    Serial.print(spinCnt);
    Serial.print("   ");
    Serial.println( milisToTimeString(millis()));
    lastSpinTime = millis();
    switch(state){
      case s_STARTING:
        state = s_STARTED;
        break;
      case s_PAUSED:
        return;
      case s_WAIT:
      //если в режиме ожидания начали бежать, то автоматически стартуем
        if (spinCnt>5){
          //state = s_STARTING
          start();
        }
        break;
    }

    spinCnt++;

    if (state!=s_STARTED){
      return;
    }
    Serial.print("spin: ");
    awgSpinCounter--;
    if (awgSpinCounter <=0){
      calcSpeed();
    }

  }

String milisToTimeString(unsigned long ms) {
  unsigned long totalseconds = ms / 1000;
  int hours = totalseconds / 3600;
  int minutes = (totalseconds / 60)%60;
  int seconds = totalseconds % 60;
  String s = ((hours < 10) ? "0" : "") + String(hours) + ":"+ ((minutes < 10) ? "0" : "") + String(minutes) + ":" + ((seconds < 10) ? "0" : "") + String(seconds);
  return s;
}

  

  void calcSpeed(){
    unsigned long currTime = millis();
    long timeBetwSpins = currTime-lastMeasTime;
    /*
    Serial.print("timeBetwSpins: ");
    Serial.println(timeBetwSpins);
    */
    float dist = spinDistance*spinCountForAWGCalc;
    curSpeed = dist/(timeBetwSpins/1000.0);
    float d = getDistance();
    avgSpeed = d/((currTime - startTime - totalPauseTime)/1000.0);
    lastMeasTime=currTime;
    
    awgSpinCounter = spinCountForAWGCalc;
    String totalTime = "";
    /*
    Serial.print("spinCnt: ");
    Serial.println(spinCnt);
    Serial.print("distance: ");
    Serial.println(d);
    */
/*
    Serial.print("curSpeed: ");
    Serial.print(curSpeed);
*/
    curSpeed = (curSpeed/1000.0)*3600.0;
    /*
    Serial.print(" km/h: ");
    Serial.println(curSpeed);
    
    Serial.print("avgSpeed: ");
    Serial.print(avgSpeed);
    /*
    avgSpeed = (avgSpeed/1000.0)*3600.0;
    /*
    Serial.print(" km/h: ");
    Serial.println(avgSpeed);
    Serial.print(" total time: ");
   */
    //Serial.println( milisToTimeString(currTime - startTime));

  }

  void display(){
    String line1 = "";
    String line2 = "";
    switch(state){
      case s_WAIT:
          line1 = "      WAIT";
        break;
        case s_STARTING:
          line1 = "   STARTING...";
        break;
        case s_STARTED:
        case s_PAUSED:
             unsigned long currTime = millis();
              if ((currTime-lastRepaint)<500 ){
                return;
              }
              lastRepaint = currTime;
              unsigned long allTime = currTime - startTime-totalPauseTime;
              String t = milisToTimeString(allTime);
              if (isPaused()){
                if ((currTime % 60)%2 ==0){
                  t = "PAUSED  ";
                }
              }
           line1=format(String(avgSpeed,2), String(getDistance(),1));
           line2=format(String(curSpeed, 2), t);
        break;
    }
    //char TextBuffer[16];
    //snprintf  (TextBuffer,  sizeof(buff), "%f", avgSpeed);

    lcdOb->repaint (
      line1,line2);
  }

  String format(String s1, String s2){
    byte l1 = s1.length();
    byte l2 = s2.length();
    if (l1+l2<16){
      for (byte i=0; i<16-l1-l2; i++){
        s1+=" ";
      }
      s1 += s2;
    }else{
      s1+=' '+s2;
    }
    return s1;
  }

  float getDistance(){
    float d = spinDistance*spinCnt; return d;
    //return spinDistance*spinCnt;
  }

  void onButtonClick() {

    int btn = read_LCD_buttons();
    if (btn!=btnNONE){
      if (prevBtn == btn && (millis()-prevBtnTime)<500){

        return;  
      }
      prevBtn = btn;
      prevBtnTime = millis();
      Serial.print("button:");
      Serial.println(btn);
    }

    switch (btn) {
      case btnSELECT:
      if (state == s_PAUSED){
          state = s_STARTED; 
      }else{
        start();
      }
      break;
      case btnLEFT:
        state = s_PAUSED;
      break;
    }
    
  }
};


LCD *lcdObject = new LCD();
Meter *meter = new Meter(lcdObject);

byte readHall(){
  //https://osoyoo.com/2017/07/28/arduino-lesson-hall-effect-sensor-module/
  sensorValue = analogRead(sensorPin); //read the value of A0
  digitalValue=digitalRead(digitalPin); //read the value of D0
  /*
  Serial.print("Sensor Value "); // print label to serial monitor 
  Serial.println(sensorValue); //print the value of A0
  */
  //Serial.print("Digital Value "); // print label to serial monitor 
  //Serial.println(digitalValue); //print the value of D0 in the serial

  if( digitalValue==HIGH )//if the value of D0 is HIGH
  {
    digitalWrite(ledPin,LOW);//turn off the led
  }
  if( digitalValue==LOW)//else
  {
    digitalWrite(ledPin,HIGH);//turn on the led
  }
  return digitalValue;
}

void setup() {
  Serial.begin(57600);
  
  #ifdef AVR
  Wire.begin();
  #else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  lcd.begin(16, 2); // start the library 
  lcd.setCursor(0, 0);

  
  
  //hall
  pinMode(digitalPin,INPUT);//set the state of D0 as INPUT
  pinMode(ledPin,OUTPUT);//set the state of pin13 as OUTPUT
  //meter->start();
}

void loop() {
  unsigned long t = millis();
  
  meter->readHallValue();
  meter->display();
  meter -> onButtonClick();
  //Serial.println('*');
  delay(DEF_DELAY);
  if (meter->isPaused()){
    meter->addPauseTime(millis()-t);  
  }
  
}
