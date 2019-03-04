#include "TimerCls.h"




WORKOUT_TYPE workOuts[4] = { WT_EMOM,
                             WT_AMRAP,
                             WT_AFAP,
                             WT_TABATA};

TIMER_MODE Timer::getMode(){

  return mode;
}

String Timer::getDisplayString(){
  switch(getMode()){
    case m_SELECT_WORKOUT:
      return  getWorkOutName();
    break;
  }
  return "hello";
}

String Timer::getWorkOutName(){
  switch (currentWT){
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

void  Timer::applyInput(int input){
  switch(getMode()){
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
 * установка настроек тренировки
 */
void Timer::applayInputToSetWorkOut(int input){
  
}

void Timer::applayInputToClock(int input){
   switch(input){
      case SELECT_WORKOUT:
         mode = m_SELECT_WORKOUT;
         applayInputToSelectWorkOut(input);
         break;
   }
}

void Timer::applayInputToSelectWorkOut(int input){
 
  indxOfWO = 0;
  switch(input){
    case SELECT_WORKOUT:
      currentWT = workOuts[indxOfWO++];
      if (indxOfWO == sizeof(workOuts)){
        indxOfWO=0;
      }
      break;    

      case OK_BUTTON:
         mode = m_SET_WORKOUT;
         break;
  }   
  
}

void Timer::logger(){
  Serial.println("TIMER");
  Serial.println("  mode: "+mode);
  Serial.println("  currentWT: "+currentWT);
  Serial.println("  indxOfWO: "+indxOfWO);

}

