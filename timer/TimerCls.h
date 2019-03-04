#include "RTClib.h"

enum TIMER_MODE{
 m_CLOCK,
 /*Выбор тренировки*/
 m_SELECT_WORKOUT, 
 /*Настройки выбранной тренировки*/
 m_SET_WORKOUT
};

enum WORKOUT_TYPE{
 WT_NONE,
 WT_EMOM,
 WT_AMRAP,
 WT_AFAP,
 WT_TABATA
};


/*выбор типа тренировок*/
#define SELECT_WORKOUT 0x00
#define OK_BUTTON 0x01

const String WO_EMOM = "EMOM";
const String WO_AMRAP = "AMRAP";
const String WO_AFAP = "AFAP";
const String WO_TABATA = "TABATA";


class Timer{
public:  
  RTC_DS1307 rtc;
  TIMER_MODE mode = m_CLOCK;
  WORKOUT_TYPE currentWT;
  int indxOfWO = 0;

  TIMER_MODE getMode();
  void applyInput(int input);
  String getDisplayString();

  void applayInputToClock(int input);
  void applayInputToSelectWorkOut(int input);
  void applayInputToSetWorkOut(int input);
  String getWorkOutName();
  void logger();
  
};
