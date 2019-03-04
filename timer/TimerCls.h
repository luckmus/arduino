#include "RTClib.h"

enum TIMER_MODE{
 m_CLOCK
};

class Timer{
public:  
  RTC_DS1307 rtc;
  TIMER_MODE mode = m_CLOCK;
  TIMER_MODE getMode();

  void applyInput(int input);
  String getDisplayString();
  
};
