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

////hall
const int ledPin = 13;//the led attach to pin13
int sensorPin = A0; // select the input pin for the potentiometer
int digitalPin=2; //D0 attach to pin2
int sensorValue = 0;// variable to store the value coming from A0
boolean digitalValue=0;// variable to store the value coming from pin2

////hall

int spinDistance = 60;

enum LCD_SHOW_MODE {

  m_WAIT,

};

// read the buttons 
int read_LCD_buttons() {
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

  void onButtonClick() {

    int btn = read_LCD_buttons();

    switch (btn) {

    }
    
  }

  void repaint() {
    lcd.clear();
    lcd.print("TEST");
    
  }
};


class Meter {
  int startTime;
  byte lastHallVal;
  int spinCnt = 0;
  //необходимое количество боротов для измерения средней скорости
  byte spinCountForAWGCalc = 3;
  byte awgSpinCounter;
  float avgSpeed = 0;
  //время последнего измерения средней скорости
  int lastMeasTime;
  LCD lcd;
  public :
  Meter(LCD val){
    lcd = val;
  }

  void start(){
    startTime = millis();
    lastMeasTime = startTime;
    awgSpinCounter = spinCountForAWGCalc;
    Serial.println("start");
  }
  void readHallValue(){
    byte rh = readHall();
    if (rh != lastHallVal){
      lastHallVal = rh;
      spin();
    }
    
  }
  void spin(){
    spinCnt++;
    Serial.print("spin: ");
    awgSpinCounter--;
    if (awgSpinCounter <=0){
      calcAVGSpeed();
    }

  }

  void calcAVGSpeed(){
    int currTime = millis();
    int timeBetwSpins = currTime-lastMeasTime;
        Serial.print("timeBetwSpins: ");
    Serial.println(timeBetwSpins);
    int dist = spinDistance*spinCountForAWGCalc;
    avgSpeed = dist/(timeBetwSpins/1000.0);
    lastMeasTime=currTime;
    awgSpinCounter = spinCountForAWGCalc;
      Serial.print("distance: ");
    Serial.println(getDistance());
    Serial.print("avgSpeed: ");
    Serial.println(avgSpeed);
  }

  int getDistance(){
    return spinDistance*spinCnt;
  }
};


LCD *lcdObject = new LCD();
Meter *meter = new Meter(*lcdObject);

byte readHall(){
  //https://osoyoo.com/2017/07/28/arduino-lesson-hall-effect-sensor-module/
  sensorValue = analogRead(sensorPin); //read the value of A0
  digitalValue=digitalRead(digitalPin); //read the value of D0
 // Serial.print("Sensor Value "); // print label to serial monitor 
//  Serial.println(sensorValue); //print the value of A0
//  Serial.print("Digital Value "); // print label to serial monitor 
//  Serial.println(digitalValue); //print the value of D0 in the serial
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
  meter->start();
}

void loop() {
  meter->readHallValue();
  lcdObject -> repaint();
  lcdObject -> onButtonClick();
  Serial.println('/');
  delay(200);

}
