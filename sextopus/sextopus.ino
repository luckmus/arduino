//#include <Servo.h>
#include <SoftwareServo.h> 
//#include <NewPing.h>
#include <IRremote.h>

#define PLUS 1;
#define MINUS -1;
uint16_t MAX_PULSE_VALUE = 2400;
int clawsCount = 6;
int const clawDelay = 60;
int xStepAngle = 12;
int yStepAngle = 50;
float MIN_TURN_DISTANCE = 15.0;
int RECV_PIN = 0;
long prevCMD;
//*
#define TRIGGER_PIN  0  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     1  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
//NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Decoded value for NEC when a repeat code is received
//#define FORWARD_SGN 0xFF629D  //simple
#define FORWARD_SGN 0x38A85051
#define FORWARD1_SGN 0x511DBB

//#define BACK_SGN 0xFFA857   //simple
#define BACK_SGN 0xD17D7D93
#define BACK1_SGN 0xA3C8EDDB

//#define LEFT_SGN 0xFF22DD  //simple
#define LEFT_SGN 0x6888BCC3
#define LEFT1_SGN 0x52A3D41F

//#define RIGHT_SGN 0xFFC23D    //simple
#define RIGHT_SGN 0x449257F7
#define RIGHT1_SGN 0x20FE4DBB

#define STAY_SGN 0xB65E2481

IRrecv irrecv(RECV_PIN);
decode_results results;
/*/
/*Направление движения
*/
enum MoveDirection
{
    d_forward,  
    d_backward, 
    d_right,
    d_left,
    d_stay
};
/*Сторона, на которой находится лапа*/
enum ClawSide
{
    s_unknow,
    s_right,
    s_left
};
/*положение лапы*/
enum ClawPosition
{
    p_forward,
    p_center,
    p_backward
};

/*положение лапы*/
enum ClawYPosition
{
    p_up,
    p_cdown
};

enum TripleGaitPart{
  FIRST,
  SECOND
};
/*этап движения трипода*/
enum ActionPart{
  PART_1,
  PART_2,
  PART_3,
  PART_4,
  PART_5,
  PART_6,
  PART_7,
  PART_8,
  PART_9  
};
MoveDirection DEFAULT_TURN_DIRCTION = d_right;

class Claw{
  friend class Spider;
  public:
    Claw(int, int, ClawSide, TripleGaitPart, int, int, int, int);
    void doStep(MoveDirection);
    void initPorts();
    void initClaw();
    void up();
    void down();
    int getXAngle(ClawPosition);
    void step(ClawPosition);
    
    int port1;
    int port2;
    int servo1InitAngle;
    int servo2InitAngle;      
    ClawPosition cp;
    TripleGaitPart gaitPart;
    //char Str4[ ];
    int stepCounter;
    Claw& operator=(const Claw&);
    
  private:
    Claw();
    void forwardStep();
    void backwardStep();  
    int getUpAngle();
    int getDownAngle();
    /*перемещается вперед/назад*/
    SoftwareServo servo1;
//      Servo servo1;
    /*перемещается вверх/вниз*/
    SoftwareServo servo2; 
//      Servo servo2; 
    
    int servo1AngleSign;
    int servo2AngleSign;    
  

    ClawSide cs;
};

Claw& Claw::operator=(const Claw& r)
{
  port1 = r.port1;
  port2 = r.port2;
  servo1InitAngle = r.servo1InitAngle;
  servo2InitAngle = r.servo2InitAngle;
  cp = r.cp;
  cs = r.cs;
  gaitPart = r.gaitPart;
  stepCounter = r.stepCounter;
  servo1AngleSign = r.servo1AngleSign;
  servo2AngleSign = r.servo2AngleSign;  
  return *this;
}

Claw::Claw(int p1, int p2, ClawSide vCS, TripleGaitPart vTGP, int angle1, int angle2, int vServo1AngleSign, int vServo2AngleSign){
  port1 = p1;
  port2 = p2;  

  cs = vCS;
  gaitPart = vTGP;

  servo1InitAngle = angle1;
  servo2InitAngle = angle2;

  servo1AngleSign = vServo1AngleSign;
  servo2AngleSign = vServo2AngleSign;
  
  cp = p_center;
  stepCounter = 0;
}
Claw::Claw(){
}

void Claw::initPorts(){
  servo1.attach(port1);
  servo2.attach(port2);  
}

void Claw::initClaw(){
  servo1.write(servo1InitAngle);
  servo1.setMaximumPulse(MAX_PULSE_VALUE);
  
  servo2.write(servo2InitAngle);  
  servo2.setMaximumPulse(MAX_PULSE_VALUE);
}

int Claw::getUpAngle(){
  switch (cs){
    case s_right:
      return servo2InitAngle + yStepAngle*servo2AngleSign;
    case s_left:
      return servo2InitAngle - yStepAngle*servo2AngleSign;
  }
  return servo2InitAngle;
}

int Claw::getDownAngle(){
  return servo2InitAngle;
}

void Claw::up(){
  servo2.write(getUpAngle());
}

void Claw::down(){
  servo2.write(getDownAngle());
}

int Claw::getXAngle(ClawPosition pos){
  int angle = servo1InitAngle;
  switch(pos){
    case p_forward:
      //Serial.print("forward ");
        if(cs == s_left){
          angle = servo1InitAngle+xStepAngle*servo1AngleSign;
          //Serial.print("left ");          
        }
        else{
          //Serial.print("right: ");                    
          angle = servo1InitAngle-xStepAngle*servo1AngleSign;
        }      
     break;
     
    case p_center:     
      angle = servo1InitAngle;
    break;
    case p_backward:
    //Serial.print("backward ");
        if(cs == s_left){
          //Serial.print("left ");
          angle = servo1InitAngle-xStepAngle*servo1AngleSign;
        }
        else{
          //Serial.print("right: ");
          angle = servo1InitAngle+xStepAngle*servo1AngleSign;
        }      
     break;     
  }
 
  //Serial.println(angle); 
  return angle;
}

void Claw::step(ClawPosition pos){
  int angle = getXAngle(pos);
  //Serial.println(angle); 
  servo1.write(angle);
  SoftwareServo::refresh();

}

void Claw::forwardStep(){
  stepCounter++;
  /*
  Serial.print("forwardStep: current position ");
  Serial.print(cp);
  Serial.println();
  */

  int angle;
  Serial.print("position ");
  Serial.print(cp);
  Serial.println();  
  switch(cp){
    //лапа впереди
    case p_forward:
    /*
        //поднять
        up();
        delay(clawDelay);        
    */

        //передвинуть назад на полную амплитуду
        if(cs == s_left){
          angle = servo1InitAngle-xStepAngle*servo1AngleSign;
        }
        else{
          angle = servo1InitAngle+xStepAngle*servo1AngleSign;
        }
        /*
        Serial.print("p_forward angle: ");        
        Serial.println(angle);        
        servo1.write(angle);
        */

    /*        
        delay(clawDelay);        
        //опустить
        getDownAngle();      
    */
        servo1.write(angle);
        cp = p_backward;
    break;  
    //лапа позади или по центру
    case p_center:
    case p_backward:
    /*
        //поднять
        servo2.write(getUpAngle());
        delay(clawDelay);
    */      
        //передвинуть вперед на полную амплитуду
        if(cs == s_left){
          angle = servo1InitAngle+xStepAngle*servo1AngleSign;
        }
        else{
          angle = servo1InitAngle-xStepAngle*servo1AngleSign;
        }
        /*
        Serial.print("p_backward angle: ");                
        Serial.println(angle);
        */
        servo1.write(angle);
   /*
        delay(clawDelay);        
        //опустить
      servo2.write(getDownAngle()); 
   */ 
        cp = p_forward;
    break;      
    default:
      Serial.println("!!!!! unknow position !!!!");
    break;
    
  }
  /*
  Serial.print("set new position: ");
  Serial.println(cp);        
  */

}

void Claw::backwardStep(){
  int angle;
  /*
  Serial.print("position ");
  Serial.print(cp);
  Serial.println();
  Serial.println();
  */
 switch(cp){
    
    //лапа позади или по центу
    case p_center:
    case p_backward:
    /*
        //поднять
        servo2.write(getUpAngle());
        delay(clawDelay);        
    */
        //передвинуть назад на полную амплитуду
        angle = servo1InitAngle+xStepAngle;
        /*
        Serial.println(angle);
        */
        servo1.write(angle);
    /*
        delay(clawDelay);        
        //опустить
        servo2.write(getDownAngle());      
        delay(clawDelay);        
     */
        cp = p_forward;
    break;  
    
    //лапа впереди
    case p_forward:
    /*
        //поднять
        servo2.write(getUpAngle());
        delay(clawDelay);        
    */
        //передвинуть вперед на полную амплитуду
        angle = servo1InitAngle-xStepAngle;
        /*
        Serial.println(angle);
        */
        servo1.write(angle);
    /*  
        delay(clawDelay);        
        //опустить
        servo2.write(getDownAngle());  
    */
        cp = p_backward;    
    break;      
  }
}

void Claw::doStep(MoveDirection sDirection){
  /*
  Serial.print("direction ");
  Serial.println(sDirection);
  */
  switch(sDirection){
    case d_forward:
       forwardStep();
    break;
    case d_backward:
       backwardStep();
    break;
    case d_left:
       if (cs == s_right){
         forwardStep();
       }else{
         backwardStep();             
       }
    break;
    case d_right:
       if (cs == s_right){
         backwardStep();
       }else{
         forwardStep();
       }
    break;       
  }

}
 
class Spider{
  public:
  Spider();
  void   init();
  void   initClaws();
  void tripleStep();
  void tripleStepNew();
  int getNextClawId();
  void upPart(TripleGaitPart part);
  void downPart(TripleGaitPart part);  
  void movePart(TripleGaitPart, ClawPosition);
  void movePart(TripleGaitPart, ClawPosition, ClawPosition);
  void upFirstPart();
  void downFirstPart();  
  void upSecondPart();
  void downSecondPart();    
  void moveFirstPart();
  void moveSecondPart();
  Claw getNextClaw();
  Claw claws[6];
  
/*
  Claw firstPart[3];    
  Claw secondPart[3];      
*/  
  MoveDirection md;

  private:
  int nextClaw;
  TripleGaitPart tgp;
  ActionPart actionPart;
  TripleGaitPart getAnotherPart(TripleGaitPart part);


};


Spider::Spider()
{
  /*
  claws[0] = Claw( 11, 8, s_left, FIRST,  88, 70, 1, 1);  //left
  claws[1] = Claw( 9, 10, s_right, SECOND, 82, 70, 1, 1 );  //right
  claws[2] = Claw( 6, 7, s_left, SECOND, 60, 65, -1, -1);  //left 
  claws[3] = Claw( 5, 4, s_right, FIRST, 110, 70, -1, -1);  //right  
  claws[4] = Claw( 12, 13,s_left , FIRST, 75, 95, -1, -1 );    
  claws[5] = Claw( 3, 2,s_right , SECOND, 110, 80, -1, -1 );    
  */
claws[0] = Claw( 2, 3, s_left, FIRST,  73, 45, 1, 1);  //left
  claws[1] = Claw( 4, 5, s_right, SECOND, 120, 110, 1, 1 );  //right
  claws[2] = Claw( 6, 7, s_left, SECOND, 70, 110, -1, 1);  //left 
  claws[3] = Claw( 8, 9, s_right, FIRST, 70, 110, -1, 1);  //right  
  claws[4] = Claw( 10, 11,s_left , FIRST, 87, 40, -1, 1 );    
  claws[5] = Claw( 12, 13,s_right , SECOND, 93, 90, -1, -1 );      

/*  
   firstPart[0] = claws[0];
   firstPart[1] = claws[3];

   firstPart[2] = claws[4];    

   secondPart[0] = claws[1];
   secondPart[1] = claws[2];
   secondPart[2] = claws[5];       
*/   
//  md = d_forward;
  md = d_stay;
  nextClaw = -1;
  tgp = FIRST;
}

void Spider::init(){
    for (int i=0; i<clawsCount; i++){
      claws[i].initPorts();
    }
    initClaws();
    irrecv.enableIRIn();
}

void Spider::initClaws(){
     for (int i=0; i<clawsCount; i++){
      claws[i].initClaw();
    }
}

void Spider::upPart(TripleGaitPart part){
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == part){
      claws[i].up();
    }
  }
}

void Spider::downPart(TripleGaitPart part){
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == part){
      claws[i].down();
    }
  }
}

TripleGaitPart Spider::getAnotherPart(TripleGaitPart part){
  if (part == FIRST){
    return SECOND;
  }
  else{
    return FIRST;
  }
}

/*
1. Зафиксировать ноги
2. Поднять первую группу
3. Поднятую первую группу передвинуть вперед
4. Опущенную вторую группу передвинуть назад, передвинут тем самым тело вперед
5. Опустить первую группу вниз
6. Поднять вторую группу
7. Поднятую вторую группу передвинуть вперед
8. Опущенную первую группу передвинуть назад, передвинут тем самым тело вперед
9. Опустить вторую группу вниз
source: http://www.societyofrobots.com/robotforum/index.php?topic=10562.0

*/
/**
  active part та часть, которая двигает тело
  passive part та часть, которая просто поднимает ноги
*/
void Spider::tripleStepNew(){
  TripleGaitPart passivePart = getAnotherPart(tgp);
  TripleGaitPart activePart = tgp;
  ClawPosition passivePosition; //по-умолчанию левая сторона
  ClawPosition activePosition;  //по-умолчанию левая сторона
  ClawPosition passiveRightPosition; //по-умолчанию левая сторона
  ClawPosition activeRightPosition;  //по-умолчанию левая сторона  
  //неучтено движение вправо и влево
  switch(md){
    case d_backward:
      passivePosition = p_backward;
      activePosition = p_forward;      
      passiveRightPosition = p_backward;
      activeRightPosition = p_forward;      
    break;    
    
    case d_forward:
    default:
      passivePosition = p_forward;
      activePosition = p_backward;      
      passiveRightPosition = p_forward;
      activeRightPosition = p_backward;      
    break;
    
    case d_left:
      passivePosition = p_backward;
      activePosition = p_forward;      
      passiveRightPosition = p_forward;
      activeRightPosition = p_backward;         
    break;
    
    case d_right:
      passivePosition = p_forward;
      activePosition = p_backward;      
      passiveRightPosition = p_backward;
      activeRightPosition = p_forward;  
    break;    
    
    case d_stay:
        initClaws();
        return;
    break;
  }
  /*
  upPart(passivePart);                          intServoRefreshDelay(clawDelay);
  movePart(passivePart, passivePosition);       intServoRefreshDelay(clawDelay);
  movePart(activePart, activePosition);         intServoRefreshDelay(clawDelay);
  downPart(passivePart);                        intServoRefreshDelay(clawDelay);
  */  
  upPart(passivePart);                                                intServoRefreshDelay(clawDelay);
  movePart(passivePart, passivePosition, passiveRightPosition);       intServoRefreshDelay(clawDelay);
  movePart(activePart,  activePosition,  activeRightPosition);        intServoRefreshDelay(clawDelay);
  downPart(passivePart);                                              intServoRefreshDelay(clawDelay);  
  
  tgp = passivePart;
  
  //Serial.println("______________________________________");
}


void Spider::movePart(TripleGaitPart part, ClawPosition leftPos, ClawPosition rightPos){    
    for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == part){
      //Serial.print(i);
      //Serial.print(": ");
      if (claws[i].cs==s_right){
        claws[i].step(rightPos);         
      }
      else if (claws[i].cs==s_left){        
        claws[i].step(leftPos);  
      }
      else{
        Serial.print("unknow claw side");
      }
      //intServoRefreshDelay(clawDelay);
    }
  }
}

void Spider::movePart(TripleGaitPart part, ClawPosition pos){    
    for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == part){
      Serial.print(i);
      Serial.print(": ");
      claws[i].step(pos);  
      //intServoRefreshDelay(clawDelay);
    }
  }
}

void Spider::upFirstPart(){
  upPart(FIRST);
  /*
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == FIRST){
      claws[i].up();
    }
  }
  */
}

void Spider::downFirstPart(){
  downPart(FIRST);
  /*
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == FIRST){
      claws[i].down();
    }
  }
  */
}

void Spider::upSecondPart(){
  upPart(SECOND);
  /*
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == SECOND){
      claws[i].up();
    }
  }
  */  
  
}

void Spider::downSecondPart(){
  downPart(SECOND);
  /*
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == SECOND){
      claws[i].down();
    }
  }
  */  
}

void Spider::moveFirstPart(){
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == FIRST){
      claws[i].doStep(md);
    }
  }
}

void Spider::moveSecondPart(){
  for(int i=0; i<=clawsCount; i++){
    if (claws[i].gaitPart == SECOND){
      claws[i].doStep(md);
    }
  }  
}

void Spider::tripleStep(){
  switch(tgp){
    case FIRST:
      upSecondPart();
      intServoRefreshDelay(clawDelay);
      moveFirstPart();
      intServoRefreshDelay(clawDelay);
      downSecondPart();      
      tgp = SECOND;
    break;
    case SECOND:
      upFirstPart();
       intServoRefreshDelay(clawDelay);
      moveSecondPart();
       intServoRefreshDelay(clawDelay);
      downFirstPart();
      tgp = FIRST;    
    break;    
  }
}


int Spider::getNextClawId(){
  if ((nextClaw>=0) &&(nextClaw<clawsCount)){
  Claw c = claws[nextClaw];
  /*
  Serial.print("prev claw position:");
  Serial.println(c.cp);
  */
  }

  if (nextClaw==-1){
    return ++nextClaw;
  }
  if( nextClaw == clawsCount-1 ){
    nextClaw = 0;
    return nextClaw;
  }
    return ++nextClaw;
  
}

  Claw Spider::getNextClaw(){
    int id = getNextClawId();
//    Serial.print(id);
//    Serial.println();
    return claws[id];
  }
/**
поворачивает привод с начального угла дло конечного 
servo[] - масив сервоприводов, которые надо повернуть
startAngle[] - массив начальных (оцентрированных) значений сервоприводов
addAngleValue - модуль значение угла поворота приводов
direct[] - массив направлений движения. значения [-1, 1] -1: уменьшаем значение угла на addAngleValue, 1: увеличиваем значение угла на addAngleValue
arrSize - количество сервоприводов
arrSize - дискретное значение угла поворота
stepDelay - задержка после  каждого прохода цикла по приводам

void speedControlMotion(Servo servo[], int startAngle[], int addAngleValue, byte direct[], int arrSize, int stepAngle, int stepDelay){

  for (int i=stepAngle; i<=addAngleValue; i+=stepAngle){
      for (int j=0; i<arrSize; j++){
        int newAngle = startAngle[j]+i*direct[j];
        servo[j].write(newAngle);
      }
    delay(stepDelay);
  }
}
*/  
/*
Возвращает растояние до препядствия
*/
float getDistance(){
  /*
  unsigned int uS = sonar.ping(); // 
  return uS / US_ROUNDTRIP_CM;
  */
  return 200;
 }

Spider spider;
void setup() {
  Serial.begin(9600);
  spider.init();
  intServoRefreshDelay(100);  

}

void intServoRefreshDelay(int value){
  int v = 0;
  int constDelay = 25;
  while(value>v){
    v += constDelay;
    delay(constDelay);
    SoftwareServo::refresh();
    readDirection();
  }
}

void readDirection(){
  if (irrecv.decode(&results)) {
    long val=results.value;
    irrecv.resume();
    if (val!=prevCMD){
      prevCMD = val;
      //return;
    }
    prevCMD = val; 
    Serial.println(val, HEX);
    switch(val){
      
           
      case 0xFFFFFFFF:
      //case -780305005:
      //case 950554705:
      //case 1150441463:
      //case 1753791683:
      //irrecv.resume(); // Receive the next value
        return;
      
      
      case FORWARD_SGN:
      case FORWARD1_SGN:
        spider.md = d_forward;
      break;
      case BACK_SGN:
      case BACK1_SGN:
        spider.md = d_backward;
      break;
      case LEFT_SGN:
      case LEFT1_SGN:
        spider.md = d_left;
      break;
      case RIGHT_SGN:
      case RIGHT1_SGN:
        spider.md = d_right;
      break;
      
      case  STAY_SGN:
      default:
        spider.md = d_stay;
      
    }
    //Serial.println(val);
    //Serial.println(spider.md);
    //irrecv.resume(); // Receive the next value
  }
}

void loop() { 
/*
  float intd = getDistance();

  Serial.print("distance: ");
  Serial.println(intd);
  if (intd == 0 ){
    Serial.println("zero distance");
  }
  if ((intd !=0 ) && (intd < MIN_TURN_DISTANCE)){
     spider.md = DEFAULT_TURN_DIRCTION; 
      Serial.println("Set move direction to right");
  }
  else{
     spider.md = d_forward; 
     //Serial.println("Set move direction to forward");
  }
*/  
  readDirection();
  /*
    d_forward,  
    d_backward, 
    d_right,
    d_left  
  */
  //spider.md = d_backward;
  //spider.md = d_forward;
  //spider.md = d_right;
  //spider.md = d_left;
  //spider.initClaws();
  spider.tripleStepNew();
  /*
  int pos = spider.getNextClawId();
  spider.claws[pos].doStep(spider.md);
  Serial.println();  
  */
  intServoRefreshDelay(clawDelay);
}

