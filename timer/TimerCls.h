#include "RTClib.h"

enum TIMER_MODE{
 m_CLOCK,
 m_SET_CLOCK,
 /*Выбор тренировки*/
 m_SELECT_WORKOUT, 
 /*Настройки выбранной тренировки*/
 m_SET_WORKOUT,
/*настройка тренировки осуществлена, ожидание запуска*/
 m_WORKOUT_WAIT_FOR_START, 
/*отстчет 3-х секунд перед стартом*/
 m_START_COUNT_DOWN,
/*тренировка идет*/
 m_WORKOUT,

 m_WORKOUT_FINISH
};

enum WORKOUT_TYPE{
 WT_NONE,
 WT_EMOM,
 WT_AMRAP,
 WT_AFAP,
 WT_TABATA
};

enum TIME_EDIT_MODE{
 EM_HOURS,
 EM_MINUTES,
 EM_SECONDS
};

enum TABATA_EDIT_MODE{
 T_WORK_TIME,
 T_REST_TIME,
 T_ROUNDS
};

enum RUN_TIMER_MODE{
 TIMER_MODE_UP,
 TIMER_MODE_DOWN
};


/*выбор типа тренировок*/
#define SELECT_WORKOUT 0x42BD
#define OK_BUTTON 0x02FD
#define BTN_UP 0x629D
#define BTN_DOWN 0xA857
#define BTN_UP_10 0x6897
#define BTN_DOWN_10 0x9867
#define BTN_RIGHT 0xC23D
#define BTN_LEFT 0x22DD
#define BTN_RESET 0x52AD
#define WORKOUTS_SIZE 4
#define MIN_EMOM 1
#define MAX_EMOM 99
#define DEFAULT_MINUTES 20
#define DEFAULT_SECONDS 00
#define MAX_WO_MINUTES  99
#define MAX_WO_SECONDS  59
#define DEF_TABATA_WORK_TIME  20
#define DEF_TABATA_WAIT_TIME  10
#define DEF_TABATA_ROUNDS  8
#define MAX_TABATA_TIME 999
#define MIN_TABATA_TIME 0
#define COUNT_DOWN_TIME 11000
#define SECOND_TIME 1000

#define BUZZER_PIN  3

const String WO_EMOM = "EMOM";
const String WO_AMRAP = "AMRAP";
const String WO_AFAP = "AFAP";
const String WO_TABATA = "TABATA";


class Timer{
public:  
  RTC_DS1307 rtc;
  DateTime now;
  TIMER_MODE mode = m_CLOCK;
  //TIMER_MODE mode = m_SELECT_WORKOUT;
  WORKOUT_TYPE currentWT;
  int indxOfWO = 0;
  int emomRounds = MIN_EMOM;
  TIME_EDIT_MODE timeEditMode = EM_MINUTES;
  int edt_minutes = DEFAULT_MINUTES;
  int edt_seconds = DEFAULT_SECONDS;
  int bPin = BUZZER_PIN;
  int stopToneTime;
  boolean toneOn = false;
  RUN_TIMER_MODE timerMode = TIMER_MODE_UP;

  TABATA_EDIT_MODE tabataEditMode = T_WORK_TIME;
  int tabataWorkTime = DEF_TABATA_WORK_TIME;
  int tabataWaitTime = DEF_TABATA_WAIT_TIME;
  int tabataRounds = DEF_TABATA_ROUNDS;
  byte setMinuteVal;
  byte setHourVal;
  unsigned long startTime;
  unsigned long workoutTime;
  /**текушее состояние тренировки табата*/
  TABATA_EDIT_MODE woTabataState;
  /**текущий раунд табаты*/
  int woTabataRound;
  byte lastBeepFor = -1;
  short toneTime;
  boolean shouldTone;
  
  TIMER_MODE getMode();
  unsigned long getTime();
  void applyInput(int input);
  String getDisplayString();
  String getSetWorkoutString();
  String getTabataSetWorkoutString();

  void applayInputToClock(int input);
  void applayInputToSetClock(int input);
  void applayInputToSelectWorkOut(int input);
  void applayInputToSetWorkOut(int input);
  void applayInputToSetWorkOutEMOM(int input);
  void applayInputToSetWorkOutAMRAP(int input);
  void applayInputToSetWorkOutAFAP(int input);
  void applayInputToSetWorkOutTABATA(int input);
  void applayInputToWorkoutWaitForStart(int input);
  void nextTabataSetting();
  void checkTabataTime();
  void checkWOTime();
  String getWorkOutName();
  String getWorkoutWaitString();
  void logger();
  void reset();
  String stringFromTIMER_MODE(enum TIMER_MODE f);
  String stringFromWORKOUT_TYPE(enum WORKOUT_TYPE f);
  String getStartCountdownString();
  void startWorkout();
  void shouldFinishWorkout();
  void finishWorkout();
  void startWorkoutAFAP();
  void startWorkoutAMRAP();
  void startWorkoutEMOM();
  void startWorkoutTABATA();
  String getWorkoutString();
  String getAFAPTimeString();
  String getAMRAPTimeString();
  String getEMOMTimeString();
  String getTABATATimeString();
  String getSetTimeString();
  String milisToTimeString(unsigned long ms, RUN_TIMER_MODE tm);
  void startCountdown();
  void startFinishTone();
  void countTone();
  void tTone(int ms);
  void goTone();
  
};

