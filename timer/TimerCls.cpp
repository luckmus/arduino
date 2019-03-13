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
    case m_START_COUNT_DOWN:
      return getStartCountdownString();
    case m_WORKOUT:
      return getWorkoutString();
    case m_WORKOUT_FINISH:
      return "finish";
    case m_SET_CLOCK:
      return getSetTimeString();
  }
  if (! rtc.isrunning()) {
    return "unavai";
  }else{
    now = rtc.now();
    return ((now.hour() < 10) ? "0" : "") + String(now.hour()) + ":" + ((now.minute() < 10) ? "0" : "") + String(now.minute())/*+ ":" + ((now.second() < 10) ? "0" : "") + String(now.second())*/;
  }
}

String Timer::getSetTimeString() {
  return ((setHourVal < 10) ? "=0" : "=") + String(setHourVal) + ((setMinuteVal < 10) ? " :0" : " :") + String(setMinuteVal);
}

String Timer::getWorkoutString() {
  switch (currentWT) {
    case WT_AFAP:
      return getAFAPTimeString();
    case WT_AMRAP:
      return getAMRAPTimeString();
    case WT_EMOM:
      return getEMOMTimeString();
    case WT_TABATA:
      return getTABATATimeString();
  }
  return "go!!!";
}

String Timer::getEMOMTimeString() {
  unsigned long passedTime = getTime() - startTime;
  unsigned long totalseconds = passedTime / SECOND_TIME;
  int rounds = totalseconds / 60 + 1;
  int seconds = 60 - totalseconds % 60 - 1;
  return  ((rounds < 10) ? "R 0" : "R ") + String(rounds) + " :" + ((seconds < 10) ? "0" : "") + String(seconds);
}

String Timer::getTABATATimeString() {
  unsigned long passedTime = getTime() - startTime;
  int passedTimeSec = passedTime / SECOND_TIME;

  switch (woTabataState) {
    case T_WORK_TIME:
      if (passedTimeSec >= tabataWorkTime) {
        woTabataState = T_REST_TIME;
        startTime = startTime + passedTime;
        passedTimeSec = 0;

      }
      break;

    case T_REST_TIME:
      if (passedTimeSec >= tabataWaitTime) {
        woTabataState = T_WORK_TIME;
        startTime = startTime + passedTime;
        passedTimeSec = 0;
        woTabataRound++;
      }
      break;
  }
  String s = ((woTabataState == T_WORK_TIME) ? " W " : " R ");
  return ((woTabataRound + 1 < 10) ? "C0" : "C") + String(woTabataRound + 1) + s  + ((passedTimeSec < 10) ? " :0" : " :") + String(passedTimeSec);
}

/**
   Отображение на экране состояния тренировки AFAP
*/
String Timer::getAFAPTimeString() {
  unsigned long passedTime = getTime() - startTime;
  return milisToTimeString(passedTime);
}

String Timer::getAMRAPTimeString() {
  unsigned long passedTime = workoutTime - getTime();
  return milisToTimeString(passedTime);
}

void Timer::startWorkout() {
  mode = m_WORKOUT;
  switch (currentWT) {
    case WT_AFAP:
      startWorkoutAFAP();
      break;
    case WT_AMRAP:
      startWorkoutAMRAP();
      break;
    case WT_EMOM:
      startWorkoutEMOM();
      break;
    case WT_TABATA:
      startWorkoutTABATA();
      break;
  }
}
void Timer::finishWorkout() {
  mode = m_WORKOUT_FINISH;
  Serial.println("finishWorkout");
}

void Timer::shouldFinishWorkout() {
  if (getMode() != m_WORKOUT) {
    return;
  }

  switch (currentWT) {
    case WT_AFAP:
      Serial.print("  workoutTime: "); Serial.println(workoutTime);
      Serial.print("  startTime: "); Serial.println(startTime);
      Serial.print("  Time: "); Serial.println(getTime());
      if (workoutTime + startTime <= getTime()) {
        finishWorkout();
        //delay(5000);
      }
      break;
    case WT_AMRAP:
      if (workoutTime <= getTime()) {
        finishWorkout();
        //delay(5000);
      }
      break;
    case WT_EMOM:
      if (workoutTime + startTime <= getTime()) {
        finishWorkout();
        //delay(5000);
      }
      break;
    case WT_TABATA:
      if (woTabataRound >= tabataRounds) {
        finishWorkout();
      }
      break;

  }
}

void Timer::startWorkoutTABATA() {
  timerMode = TIMER_MODE_DOWN;
  startTime = getTime();
  workoutTime = (tabataWorkTime + tabataWaitTime) * SECOND_TIME * (float)tabataRounds;
  woTabataState = T_WORK_TIME;
  woTabataRound = 0;
}

void Timer::startWorkoutEMOM() {
  timerMode = TIMER_MODE_DOWN;
  startTime = getTime();
  workoutTime = emomRounds * (float)60 * SECOND_TIME;
}

void Timer::startWorkoutAFAP() {
  timerMode = TIMER_MODE_UP;
  startTime = getTime();
  workoutTime =  edt_minutes * (float)60 * SECOND_TIME + (float)edt_seconds * SECOND_TIME;

  //Serial.print("  workoutTime: "); Serial.println(workoutTime);
  //Serial.print("  startTime: "); Serial.println(startTime);

}

String Timer::milisToTimeString(unsigned long ms) {
  unsigned long totalseconds = ms / SECOND_TIME;
  int minutes = totalseconds / 60;
  int seconds = totalseconds % 60;
  return  ((minutes < 10) ? "0" : "") + String(minutes) + ":" + ((seconds < 10) ? "0" : "") + String(seconds);
}

void Timer::startWorkoutAMRAP() {
  timerMode = TIMER_MODE_DOWN;
  startTime = getTime();
  workoutTime =  edt_minutes * (float)60 * SECOND_TIME + (float)edt_seconds * SECOND_TIME;
  workoutTime += startTime; //время окончания тренировки
}

String Timer::getStartCountdownString() {
  int res = workoutTime - getTime();
  if (res <= SECOND_TIME - 1) {
    startWorkout();
    return getDisplayString();
  }
  res = res / SECOND_TIME;

  return ((res < 10) ? "0" : "") + String(res);
}
String Timer::getWorkoutWaitString() {
  switch (currentWT) {
    case WT_EMOM:
      return getSetWorkoutString() + " 00:00";
    case WT_AFAP:
      return "00:00";
    case WT_AMRAP:
      return getSetWorkoutString();
    case WT_TABATA:
      return ((tabataWorkTime < 10) ? ":0" : ":") + String(tabataWorkTime);

  }
}

String Timer::getSetWorkoutString() {
  switch (currentWT) {
    case WT_EMOM:
      if (emomRounds < 10) {
        return "0" + String(emomRounds);
      }
      return String(emomRounds);
    case WT_AMRAP:
    case WT_AFAP:
      return ((edt_minutes < 10) ? "0" : "") + String(edt_minutes) + ":" + ((edt_seconds < 10) ? "0" : "") + String(edt_seconds);
    case WT_TABATA:
      return getTabataSetWorkoutString();
    default:
      return "***";
  }
}

String Timer::getTabataSetWorkoutString() {
  switch (tabataEditMode) {
    case T_WORK_TIME:
      return ((tabataWorkTime < 10) ? "W 0" : "W ") + String(tabataWorkTime);
    case T_REST_TIME:
      return ((tabataWaitTime < 10) ? "R 0" : "R ") + String(tabataWaitTime);
    case T_ROUNDS:
      return ((tabataRounds < 10)   ? "C 0" : "C ") + String(tabataRounds);
  }
  return "---";
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

/**
   Сбрасывает все текущие режимы  в режим часов
*/
void Timer::reset() {
  mode = m_CLOCK;
  indxOfWO = 0;
  emomRounds = MIN_EMOM;
  timeEditMode = EM_MINUTES;
  edt_minutes = DEFAULT_MINUTES;
  edt_seconds = DEFAULT_SECONDS;
  tabataEditMode = T_WORK_TIME;
  tabataWorkTime = DEF_TABATA_WORK_TIME;
  tabataWaitTime = DEF_TABATA_WAIT_TIME;
  tabataRounds = DEF_TABATA_ROUNDS;
  timerMode = TIMER_MODE_DOWN;
}

void  Timer::applyInput(int input) {
  switch (input) {
    case BTN_RESET:
      reset();
      return;
  }
  switch (getMode()) {
    case m_CLOCK:
      applayInputToClock(input);
      break;
    case m_SET_CLOCK:
      applayInputToSetClock(input);
      break;

    case m_SELECT_WORKOUT:
      applayInputToSelectWorkOut(input);
      break;

    case m_SET_WORKOUT:
      applayInputToSetWorkOut(input);
      break;

    case m_WORKOUT_WAIT_FOR_START:
      applayInputToWorkoutWaitForStart(input);
  }
}

void Timer::applayInputToWorkoutWaitForStart(int input) {
  switch (input) {
    case OK_BUTTON:
      mode = m_START_COUNT_DOWN;
      startCountdown();
      break;
  }

}

/**
   установка настроек тренировки
*/
void Timer::applayInputToSetWorkOut(int input) {
  switch (input) {
    case OK_BUTTON:
      if (currentWT == WT_TABATA) {
        nextTabataSetting();
      } else {
        mode = m_WORKOUT_WAIT_FOR_START;
      }
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
  switch (input) {
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

  if ((emomRounds < MIN_EMOM)) {
    emomRounds = MAX_EMOM;
  }

  if ((emomRounds > MAX_EMOM)) {
    emomRounds = MIN_EMOM;
  }
}
void Timer::applayInputToSetWorkOutAMRAP(int input) {
  switch (input) {
    case BTN_DOWN:
      switch (timeEditMode) {
        case EM_MINUTES:
          edt_minutes--;
          break;
        case EM_SECONDS:
          edt_seconds--;
          break;
      }
      break;
    case BTN_UP:
      switch (timeEditMode) {
        case EM_MINUTES:
          edt_minutes++;
          break;
        case EM_SECONDS:
          edt_seconds++;
          break;
      }
      break;
    case BTN_LEFT:
    case BTN_RIGHT:
      timeEditMode = (timeEditMode == EM_MINUTES) ? EM_SECONDS : EM_MINUTES;
      break;
  }
  checkWOTime();
}

/**
   Проверяет время тренировки в момент установки, и если значения выходят за границы, то устанавливает их
*/
void  Timer::checkWOTime() {
  if (edt_minutes > MAX_WO_MINUTES) {
    edt_minutes = DEFAULT_MINUTES;
  }

  if (edt_minutes < DEFAULT_MINUTES) {
    edt_minutes = MAX_WO_MINUTES;
  }

  if (edt_seconds > MAX_WO_SECONDS) {
    edt_seconds = DEFAULT_SECONDS;
  }

  if (edt_seconds < DEFAULT_SECONDS) {
    edt_seconds = MAX_WO_SECONDS;
  }
}
void Timer::applayInputToSetWorkOutAFAP(int input) {
  applayInputToSetWorkOutAMRAP(input);
}
void Timer::applayInputToSetWorkOutTABATA(int input) {
  int value = 0;
  switch (input) {
    case OK_BUTTON:
      nextTabataSetting();
      return;
    case BTN_UP:
      value = 1;
      break;
    case BTN_DOWN:
      value = -1;
      break;
  }


  switch (tabataEditMode) {
    case T_WORK_TIME:
      tabataWorkTime += value;
      break;
    case T_REST_TIME:
      tabataWaitTime += value;
      break;

    case T_ROUNDS:
      tabataRounds += value;
      break;
  }

  checkTabataTime();
}

/**
   проверяет значения табаты на попадание в границы, если что, т возвращает в границы
*/
void Timer::checkTabataTime() {
  if (tabataWorkTime > MAX_TABATA_TIME) {
    tabataWorkTime = MIN_TABATA_TIME;
  }
  if (tabataWorkTime < MIN_TABATA_TIME) {
    tabataWorkTime = MAX_TABATA_TIME;
  }

  if (tabataWaitTime > MAX_TABATA_TIME) {
    tabataWaitTime = MIN_TABATA_TIME;
  }
  if (tabataWaitTime < MIN_TABATA_TIME) {
    tabataWaitTime = MAX_TABATA_TIME;
  }

  if (tabataRounds > MAX_TABATA_TIME) {
    tabataRounds = MIN_TABATA_TIME;
  }
  if (tabataRounds < MIN_TABATA_TIME) {
    tabataRounds = MAX_TABATA_TIME;
  }
}

void Timer::nextTabataSetting() {
  switch (tabataEditMode) {
    case T_WORK_TIME:
      tabataEditMode = T_REST_TIME;
      break;
    case T_REST_TIME:
      tabataEditMode = T_ROUNDS;
      break;
    case T_ROUNDS:
      mode = m_WORKOUT_WAIT_FOR_START;
      break;
  }
}

void Timer::applayInputToClock(int input) {
  Serial.println("applayInputToSelectWorkOut");
  switch (input) {
    case SELECT_WORKOUT:
      indxOfWO = 0;
      mode = m_SELECT_WORKOUT;
      applayInputToSelectWorkOut(input);
      break;
    case OK_BUTTON:
      mode = m_SET_CLOCK;

      now = rtc.now();
      setMinuteVal = now.minute();
      setHourVal = now.hour();

      timeEditMode = EM_HOURS;
      //applayInputToSetClock(input);
      break;
    default:
      Serial.print("unknow input ");
      Serial.println(input, HEX);
      break;
  }
}

void Timer::applayInputToSetClock(int input) {
  switch (input) {
    case BTN_UP:
      if (timeEditMode == EM_HOURS) setHourVal++; else setMinuteVal++;
      break;
    case BTN_DOWN:
      if (timeEditMode == EM_HOURS) setHourVal--; else setMinuteVal--;
      break;

    case BTN_RIGHT:
    case BTN_LEFT:
      if (timeEditMode == EM_HOURS) timeEditMode = EM_MINUTES; else timeEditMode = EM_HOURS;
      break;
    case OK_BUTTON:
      now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(),setHourVal,  setMinuteVal, 0));
      mode = m_CLOCK;
      break;
  }
  if (setHourVal>MAX_WO_SECONDS){
    setHourVal = 0;
  }
  if (setHourVal<0){
    setHourVal = MAX_WO_SECONDS;
  }
  if (setMinuteVal>MAX_WO_SECONDS){
    setMinuteVal = 0;
  }
  if (setMinuteVal<0){
    setMinuteVal = MAX_WO_SECONDS;
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

unsigned long Timer::getTime() {
  return millis();
}

void Timer::startCountdown() {
  timerMode = TIMER_MODE_DOWN;
  startTime = getTime();
  workoutTime = startTime + COUNT_DOWN_TIME;
}

void Timer::logger() {
  Serial.println("TIMER");
  Serial.println("  mode: " + stringFromTIMER_MODE(mode));
  Serial.println("  currentWT: " + stringFromWORKOUT_TYPE(currentWT));
  //Serial.print("  indxOfWO: "); Serial.println(indxOfWO);
  //Serial.print("  emomRounds: "); Serial.println(emomRounds);
  //Serial.print("  time: "); Serial.println(getTime());
  Serial.println("  displayString: " + getDisplayString());
  Serial.println("___________________");

}


String Timer::stringFromTIMER_MODE(enum TIMER_MODE f)
{
  static const String strings[] = { "m_CLOCK","m_SET_CLOCK", "m_SELECT_WORKOUT", "m_SET_WORKOUT", "m_WORKOUT_WAIT_FOR_START", "m_START_COUNT_DOWN", "m_WORKOUT", "m_WORKOUT_FINISH"  };

  return strings[f];
}


String Timer::stringFromWORKOUT_TYPE(enum WORKOUT_TYPE f)
{
  static const String stringsWT[] = {  "WT_NONE", "WT_EMOM", "WT_AMRAP", "WT_AFAP", "WT_TABATA" };

  return stringsWT[f];
}
