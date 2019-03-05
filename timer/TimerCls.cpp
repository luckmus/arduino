#include "TimerCls.h"




WORKOUT_TYPE workOuts[WORKOUTS_SIZE] = { WT_EMOM,
                                         WT_AMRAP,
                                         WT_AFAP,
                                         WT_TABATA
                                       };

TIMER_MODE Timer::getMode() {

  return mode;
}

String Timer::getDisplayString() {
  switch (getMode()) {
    case m_SELECT_WORKOUT:
      return  getWorkOutName();
    case m_SET_WORKOUT:
      return  getSetWorkoutString();
      break;
    case m_WORKOUT_WAIT_FOR_START:
      return getWorkoutWaitString();
  }
  return "hello";
}

String Timer::getWorkoutWaitString(){
  switch(currentWT){
    case WT_EMOM:
      return getSetWorkoutString()+" 00:00";
  }
}

String Timer::getSetWorkoutString(){
  switch(currentWT){
    case WT_EMOM:
    /*
      char *s;
      sprintf(s, "%d", emomRounds);
      Serial.print(" s:  "); Serial.println(s);
      return "+++";
      */
      if (emomRounds<10){
        return "0"+String(emomRounds);
      }
      return String(emomRounds);

    default:
      return "***";
  }
}

String Timer::getWorkOutName() {
  switch (currentWT) {
    case WT_EMOM:
      return WO_EMOM;
    case WT_AMRAP:
      return WO_AMRAP;
    case WT_TABATA:
      return WO_TABATA;
    case WT_AFAP:
      return WO_AFAP;
    default:
      currentWT = WT_AMRAP;
      return WO_AMRAP;
  }
}

void  Timer::applyInput(int input) {
  switch (getMode()) {
    case m_CLOCK:
      applayInputToClock(input);
      break;

    case m_SELECT_WORKOUT:
      applayInputToSelectWorkOut(input);
      break;

    case m_SET_WORKOUT:
      applayInputToSetWorkOut(input);
      break;
  }
}

/**
   установка настроек тренировки
*/
void Timer::applayInputToSetWorkOut(int input) {
  switch (input){
    case OK_BUTTON:
      mode = m_WORKOUT_WAIT_FOR_START;
      return;
  }
  
  switch (currentWT) {
    case WT_EMOM:
      applayInputToSetWorkOutEMOM(input);
    break;
    case WT_AMRAP:
      applayInputToSetWorkOutAMRAP(input);
    break;
    case WT_AFAP:
      applayInputToSetWorkOutAFAP(input);
    break;
    case WT_TABATA:
      applayInputToSetWorkOutTABATA(input);
    break;
  }
}

void Timer::applayInputToSetWorkOutEMOM(int input) {
  switch(input){
    case BTN_UP:
        emomRounds++;
      break;
    case BTN_DOWN:
      emomRounds--;
      break;
    default:
      Serial.print("SetWorkOutEMOM unknow input ");
      Serial.println(input, HEX);
      break;
  }
  if ((emomRounds<MIN_EMOM) || (emomRounds>MAX_EMOM)){
    emomRounds = 1;
  }
}
void Timer::applayInputToSetWorkOutAMRAP(int input) {

}
void Timer::applayInputToSetWorkOutAFAP(int input) {

}
void Timer::applayInputToSetWorkOutTABATA(int input) {

}

void Timer::applayInputToClock(int input) {
  Serial.println("applayInputToSelectWorkOut");
  switch (input) {
    case SELECT_WORKOUT:
      indxOfWO = 0;
      mode = m_SELECT_WORKOUT;
      applayInputToSelectWorkOut(input);
      break;
    default:
      Serial.print("unknow input ");
      Serial.println(input, HEX);
      break;
  }
}

void Timer::applayInputToSelectWorkOut(int input) {
  Serial.println("applayInputToSelectWorkOut");


  switch (input) {
    case SELECT_WORKOUT:
      currentWT = workOuts[indxOfWO++];
      if (indxOfWO >= WORKOUTS_SIZE) {
        indxOfWO = 0;
      }
      break;

    case OK_BUTTON:
      mode = m_SET_WORKOUT;
      emomRounds = MIN_EMOM;
      break;
    default:
      Serial.print("unknow input ");
      Serial.println(input, HEX);
      break;
  }

}

void Timer::logger() {
  Serial.println("TIMER");
  Serial.println("  mode: " + stringFromTIMER_MODE(mode));
  Serial.println("  currentWT: " + stringFromWORKOUT_TYPE(currentWT));
  Serial.print("  indxOfWO: "); Serial.println(indxOfWO);
  Serial.print("  emomRounds: "); Serial.println(emomRounds);
  Serial.println("  displayString: " + getDisplayString());
  Serial.println("___________________");

}


String Timer::stringFromTIMER_MODE(enum TIMER_MODE f)
{
  static const String strings[] = { "m_CLOCK", "m_SELECT_WORKOUT", "m_SET_WORKOUT", "m_WORKOUT_WAIT_FOR_START"  };

  return strings[f];
}
String Timer::stringFromWORKOUT_TYPE(enum WORKOUT_TYPE f)
{
  static const String stringsWT[] = {  "WT_NONE", "WT_EMOM", "WT_AMRAP", "WT_AFAP", "WT_TABATA" };

  return stringsWT[f];
}
