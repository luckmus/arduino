#include <Wire.h>
#include <hx711.h>
#include "RTClib.h"
#include <LiquidCrystal.h> 

int FEED_DATA_SIZE = 4;
int FEED_COUNT = 2;
RTC_DS1307 rtc;
DateTime now;
Hx711 scale(A1, A0);
int MIN_WEIGTH = 50;
int LCD_UPDATE_TIMEOUT = 3000;
// select the pins used on the LCD panel 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 
// define some values used by the panel and buttons 
int lcd_key     = 0; 
int adc_key_in  = 0; 
#define btnRIGHT  0 
#define btnUP     1 
#define btnDOWN   2 
#define btnLEFT   3 
#define btnSELECT 4 
#define btnNONE   5 
/**
  СОСТОЯНИЯ УСТАНОВЩИКА Feed
*/
enum PARAM_NUM
{
    p_hour,  
    p_minutes, 
    p_weigh,

};

enum LCD_SHOW_MODE{
 m_TIME,
 m_CURRENT_WEIGTH,
 m_TEMPERATURE, 
 //установка значения выбранного кормления
 m_SET_FEED_VAL,
 m_NEXT_FEEDING_AFTER,
 //выбор кормления из списка
 m_SELECT_FEED_ITEM
};



// read the buttons 
int read_LCD_buttons() 
{ 
  adc_key_in = analogRead(0);      // read the value from the sensor  
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741 
  // we add approx 50 to those values and check to see if we are close 
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result 
  if (adc_key_in < 50)   return btnRIGHT;   
  if (adc_key_in < 195)  return btnUP;  
  if (adc_key_in < 380)  return btnDOWN;  
  if (adc_key_in < 555)  return btnLEFT;  
  if (adc_key_in < 790)  return btnSELECT;    
  return btnNONE;  // when all others fail, return this... 
}

class Feeder{
public:  
  boolean shouldStop = true;
 void openFeeder(){
 
 }
 
 void closeFeeder(){
 
 }
 
 int getAvailableWeigth(){
   return scale.getGram();
 }
 
 void pour(int weigth){
   Serial.println("pour");
   int startWeigth = getAvailableWeigth();
   if (startWeigth<MIN_WEIGTH){
     return;
   }   
   int finishWeigth = startWeigth - weigth;
   if (finishWeigth<MIN_WEIGTH){
     finishWeigth = MIN_WEIGTH;
   }
   shouldStop = false;
   openFeeder();
   while(!shouldStop){
     delay(50);
     if (getAvailableWeigth()<=finishWeigth)
     shouldStop = (getAvailableWeigth()<=finishWeigth);
   }
   closeFeeder();
   //задержка на 60 секунд, чтобы убедиться что в данное время (в эту минуту) не будет осуществляться попыток покормить
   delay(60000);
 }
};  

Feeder *feeder = new Feeder();
class Feed{
public:  
  //время кормления час
  byte ftHour;
 //время кормления минуты
  byte ftMinute;
  //время последнего кормления
  DateTime lastFT;
  short weigth;
  
  Feed(){
  }
  
   
 void applyNewData(Feed *newData){
   this->ftHour = newData->ftHour;
   this->ftMinute = newData->ftMinute;
   this->weigth = newData->weigth;   
 }
  
  void writeToArray(uint8_t *data){
    data[0] = this->ftHour;
    data[1] = this->ftMinute;
    data[2] = (uint8_t)(this->weigth & 0x00FF);
    data[3] = (uint8_t)((this->weigth & 0xFF00)>>8);        
  }
  
  bool isTimeToFeed(){
    /*
    if (lcdObject->showMode == m_SET_FEED_VAL){
      return false;  
    }
    */
    now = rtc.now();
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.println();
    if ((this->ftHour == (byte)(now.hour())) && 
        (this->ftMinute == (byte)(now.minute()))){
      Serial.println("TimeToFeed");        
      return true; 
    }   
      Serial.println("is not TimeToFeed");            
    return false;
  }
 
  bool runFeed(){
    feeder->pour(weigth);
    lastFT = rtc.now();
    return true;
  }
  /*
    Данные хранятся в виде (1byte)hh, (1byte)mm, (1byte)weight
  */  
  void readFromRAM(uint8_t *data){
    this->ftHour = data[0];
    this->ftMinute = data[1];
    uint8_t hi = data[2];
    uint8_t lo = data[3];    
    this->weigth = (short)( ((hi&0xFF)<<8) | (lo&0xFF) );    
  }
  
  void showToSerial(){
    Serial.print(this->ftHour); Serial.print(":");
    Serial.print(this->ftMinute); Serial.print(" w:");
    Serial.print(this->weigth);
    Serial.println();
  
  }
  /**
    возвращает время следующего кормления
  */
  DateTime getAsDateTime(){
    DateTime now = rtc.now();
    //uint32_t unixtime = now.unixtime();
    uint16_t nyear = now.year(); 
    uint8_t nmonth = now.month(); 
    uint8_t nday = now.day();
    uint8_t nhour = this->ftHour; 
    uint8_t nmin = this->ftMinute;
    uint8_t nsec = now.second();
    uint32_t unixtime = DateTime(nyear, nmonth, nday, nhour, nmin, nsec).unixtime();
    //если сегодня время кормления уже прошло, значит это уже след. день, поэтому прибавлю 1 день
    if ((now.hour()>this->ftHour) ||
        ((now.hour()==this->ftHour) && (now.minute()>this->ftMinute))){
      unixtime+=24*60*60*1000;
    }
    
    return DateTime(unixtime); 
  }
  
  String getParamString(){
    String res = String(this->ftHour)+":"+String(this->ftMinute)+ " "+String(this->weigth);
    return res;
  }
  
};


/**
  редактирует условия кормления
*/
class FeedSetter{
  Feed *feed;
  PARAM_NUM paramNum = p_hour;
  
  public:
  FeedSetter(){
  }
  
  void setFeed(Feed *newFeed){
    this->feed = newFeed;
  }
  
  PARAM_NUM getNextParam(){
    switch(this->paramNum){
      case p_hour:
        return p_minutes;
      case p_minutes:
        return p_weigh;
      case p_weigh:
        return p_hour;  
    }
    return p_hour;
  }
  
  PARAM_NUM getPrevParam(){
    switch(this->paramNum){
      case p_hour:
        return p_weigh;
      case p_minutes:
        return p_hour;
      case p_weigh:
        return p_minutes;  
    }
    return p_hour;
  }  
  
  void setNextParam(){
    this->paramNum = this->getNextParam();
  }
  
  void setPrevParam(){
    this->paramNum = this->getPrevParam();
  }
  void updateParam(byte setMode){
    short val;
    byte incVal;
    if (setMode==btnDOWN){
      incVal = -1;
    }
    else if (setMode==btnUP){
      incVal = 1;      
    }
    else{
       return; 
    }
    
    switch(this->paramNum){
      case p_hour:
        val = this->feed->ftHour+(1*incVal);
        if (val>23){
          this->feed->ftHour = 0;
        }
        else if (val<0){
          this->feed->ftHour = 23;
        }
        else{
          this->feed->ftHour = val;
        }
      break;
      
      case p_minutes:
        val = this->feed->ftMinute+(1*incVal);
        if (val>59){
          this->feed->ftMinute = 0;
        }

        else if (val<0){
          this->feed->ftMinute = 59;
        }
        else{
          this->feed->ftMinute = val;
        }
      break;        
      
      case p_weigh:
        val = this->feed->weigth+(50*incVal);
        if (val<0){
          this->feed->weigth = 0;
        }
        else{
          this->feed->weigth = val;
        }      
      break; 
    }
  }
  
  
  String getParamString(){
    String res = String(this->feed->ftHour)+":"+String(this->feed->ftMinute)+ " "+String(this->feed->weigth);
    return res;
  }
  
  void save(uint8_t *data, int feedIndex){
    uint8_t writeData[FEED_DATA_SIZE];
    this->feed->writeToArray(writeData);
    rtc.writenvram(feedIndex*FEED_DATA_SIZE, writeData, 4);
  }
};


Feed *feeds[2] = {new Feed(), new Feed()};
FeedSetter *feedSetter = new FeedSetter();

/**
управление экраном и клавиатурой
*/
class LCD{
  LCD_SHOW_MODE showMode = m_TIME;
  uint32_t nextUpdateTime;//now.unixtime() +LCD_UPDATE_TIMEOUT
  int selectedFeedItem = 0; //выбранный из массива объект редактирования
  String printData[2] = {String(""), String("")};
  public:
  LCD(){
  
  }
  
  void setNextShowMode(){
    switch(this->showMode){
      case m_TIME:
        this->showMode = m_CURRENT_WEIGTH;
      break;
      case m_CURRENT_WEIGTH:
        this->showMode = m_NEXT_FEEDING_AFTER;
      break;
      case m_NEXT_FEEDING_AFTER:
        this->showMode = m_TIME;
      break;
    
    }
  
  }
  
  boolean isSetValueMode(){
    return this->showMode==m_SET_FEED_VAL;
  }

  void onButtonClick(){
    
    int btn = read_LCD_buttons();
    switch(btn){
       case btnRIGHT:
         if ( !this->isSetValueMode() ){ return; }
         feedSetter->setNextParam();
       break;
       case btnLEFT:
         if ( !this->isSetValueMode() ){ return; }       
         feedSetter->setPrevParam();         
       break;       
       case btnUP:
       case btnDOWN:
         if ( this->isSetValueMode() ){       
           feedSetter->updateParam(btn);
         }
         else if  (this->showMode == m_SELECT_FEED_ITEM){
             this->setNextSelectFeedId(btn);
         }
         else{
           return;  
         }
       break;
       /*
         настройка ведется по кнопке select
         если находимся в режиме просмотра текущих состяний  m_TIME, m_CURRENT_WEIGTH, m_NEXT_FEEDING_AFTER, m_TEMPERATURE,
         то при нажатии переходим в режим отображения списка кормлений: m_SELECT_FEED_ITEM.
         если находимся в m_SELECT_FEED_ITEM, то при нажатии переходим в режим установки значений выбранного кормления m_SET_FEED_VAL.
         если находимся в m_SET_FEED_VAL, то при нажатии сохраняем текущие значения и переходим в один из режимов просмотра текущих состяний (m_TIME)
       */
       case btnSELECT:
         switch(this->showMode){
           case m_TIME:  
           case m_CURRENT_WEIGTH:  
           case m_NEXT_FEEDING_AFTER: 
           case m_TEMPERATURE:
             this->showMode = m_SELECT_FEED_ITEM;
           break;
           
           case m_SELECT_FEED_ITEM:
             this->showMode = m_SET_FEED_VAL;
           break;
           
           case m_SET_FEED_VAL:
             this->showMode = m_TIME;
             //сохранять вроде не надо, т.к. изменения записываются сразу в обраотчике кнопок
           break;
           
         }
       break;
       case btnNONE:
          return;
    }
    this->repaint();
  }
  
  
  void setAllFeedForSelectString(){
    for (int i=0; i<2; i++){
      printData[i] = (i==this->selectedFeedItem)?">":""+feeds[i]->getParamString();
    }
  }
  
  void setNextSelectFeedId(int btn){
   int koef;
   if(btn == btnUP)    {
     koef = 1;
   }else if (btn == btnDOWN){
     koef = -1;
   }else{
     return; 
   }
    
    int i = this->selectedFeedItem+(1*koef);
    if (i<0){
      i = FEED_COUNT-1;
    }
    else if (i>(FEED_COUNT-1)){
      i = 0;  
    }
  
    this->selectedFeedItem = i;
  }
  
  void showPrintData(){
    lcd.clear();
    for(int i=0; i<2; i++){
      lcd.setCursor(0,i);
      lcd.print(this->printData[i]);
    }
  }
  
  void repaint(){
    if (this->showMode==m_SET_FEED_VAL){
      lcd.clear();
      lcd.print(feedSetter->getParamString());    
      return;
    }
    if (this->showMode==m_SELECT_FEED_ITEM){
      this->showPrintData();
      return;
    }
    
    DateTime now = rtc.now();
    if (nextUpdateTime<now.unixtime()){
      return;
    }
    this->setNextShowMode();
    lcd.clear();
    switch(this->showMode){
      case m_TIME:
        lcd.print(this->getTimeStr(now));
      break;
      case m_CURRENT_WEIGTH:
        lcd.print(this->getWeigthStr());
      break;
      case m_NEXT_FEEDING_AFTER:
        lcd.print(this->getTimeToNextFeed(now));
      break;
    }
    this->nextUpdateTime=now.unixtime() +LCD_UPDATE_TIMEOUT;
  }
  
  String getTime(DateTime now){
    String res = String(now.year(), DEC)+"/"+String(now.month(), DEC)+"/"+String(now.day(), DEC)+" "+String(now.hour(), DEC)+":"+String(now.minute(), DEC)+":"+String(now.second(), DEC);
    return res;
  }
  
  String getTimeStr(DateTime now){
    String res = String(now.year(), DEC)+"/"+String(now.month(), DEC)+"/"+String(now.day(), DEC)+" "+String(now.hour(), DEC)+":"+String(now.minute(), DEC)+":"+String(now.second(), DEC);
    return res;
  }
  
  String getWeigthStr(){
    String res = String(scale.getGram());
    return res;
  }
  
  String getTimeToNextFeed(DateTime now){
    int nearestTimeIndx = 0;
    uint32_t t=0;
    uint32_t bufTime = 0;
    for(int i=0; i<FEED_COUNT; i++){
      bufTime = feeds[i]->getAsDateTime().unixtime()-now.unixtime();
      if (i==0) {
        t=bufTime;
        continue;
      }
      if (bufTime<t){
        t = bufTime;
        nearestTimeIndx = i;
      }
    }
    return getTimeStr(feeds[nearestTimeIndx]->getAsDateTime());
  }
  
  
};

/**
  управление настройкой параметров кормления
*/


LCD *lcdObject = new LCD();


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
  
  lcd.begin(16, 2);              // start the library 
  lcd.setCursor(0,0); 
  
  rtc.writenvram(0, 20);
  rtc.writenvram(1, 45);  
  rtc.writenvram(2, 0x00);
  rtc.writenvram(3, 0x10);
  
  uint8_t readData[3] = {0};
  
  for(int i=0; i<FEED_COUNT; i++){
    rtc.readnvram(readData, 4, FEED_DATA_SIZE*i);
    feeds[i]->readFromRAM(readData);
    feeds[i]->showToSerial();
  }
  feedSetter->setFeed(feeds[0]);
}

void loop() {

  for (int i=0; i<FEED_COUNT; i++){
    if (feeds[i]->isTimeToFeed()){
      feeds[i]->runFeed();
    }
  }
  
  lcdObject->repaint();
  lcdObject->onButtonClick();
  
  Serial.println(scale.getGram());
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  delay(200);
}

