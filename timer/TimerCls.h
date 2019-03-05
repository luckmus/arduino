#include "RTClib.h"

enum TIMER_MODE{
 m_CLOCK,
 /*Выбор тренировки*/
 m_SELECT_WORKOUT, 
 /*Настройки выбранной тренировки*/
 m_SET_WORKOUT,
/*настройка тренировки осуществлена, ожидание запуска*/
 m_WORKOUT_WAIT_FOR_START, 
};

enum WORKOUT_TYPE{
 WT_NONE,
 WT_EMOM,
 WT_AMRAP,
 WT_AFAP,
 WT_TABATA
};


/*выбор типа тренировок*/
#define SELECT_WORKOUT 0x42BD
#define OK_BUTTON 0x02FD
#define BTN_UP 0x629D
#define BTN_DOWN 0xA857
#define WORKOUTS_SIZE 4
#define MIN_EMOM 1
#define MAX_EMOM 99

const String WO_EMOM = "EMOM";
const String WO_AMRAP = "AMRAP";
const String WO_AFAP = "AFAP";
const String WO_TABATA = "TABATA";


class Timer{
public:  
  RTC_DS1307 rtc;
  TIMER_MODE mode = m_CLOCK;
  //TIMER_MODE mode = m_SELECT_WORKOUT;
  WORKOUT_TYPE currentWT;
  int indxOfWO = 0;
  int emomRounds = MIN_EMOM;

  TIMER_MODE getMode();
  void applyInput(int input);
  String getDisplayString();
  String getSetWorkoutString();

  void applayInputToClock(int input);
  void applayInputToSelectWorkOut(int input);
  void applayInputToSetWorkOut(int input);
  void applayInputToSetWorkOutEMOM(int input);
  void applayInputToSetWorkOutAMRAP(int input);
  void applayInputToSetWorkOutAFAP(int input);
  void applayInputToSetWorkOutTABATA(int input);
  String getWorkOutName();
  String getWorkoutWaitString();
  void logger();
  String stringFromTIMER_MODE(enum TIMER_MODE f);
  String stringFromWORKOUT_TYPE(enum WORKOUT_TYPE f);
  
};
