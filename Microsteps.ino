// COde partly taken  from: Microsteppin guide:
// http://www.jangeox.be/2013/12/microstepping-with-arduino.html

//////////////////////////////////////////////////////////////////
// Simple EQ-5 mount RA 

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// RTC ( https://github.com/smz/Arduino-RTCtime )
#include <avr/sleep.h>
#include <Wire.h>
#include "Sodaq_DS3231.h"
#define LED_PIN 13
static uint8_t prevSecond=0; 
volatile unsigned int countSeconds = 0;
volatile unsigned int countMilliSeconds = 0;
unsigned int intervalSeconds = 0;
unsigned int intervalMilliSeconds = 0;
intdoStepper = 1;


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// stepper guiding modes
intguidingMode = 0;
#define STOP     0
#define GUIDING  1
#define EASTx2   2
#define EASTx4   3
#define EASTx8   4
#define EASTmax  5
#define WESTx2   6
#define WESTx4   7
#define WESTx8   8
#define WESTmax  9


// astronomical constants
// 23h 56min 4.091sec = 86164.091sec
// milliseconds:
unsigned long starTime24hMillisec = 861640910;

// EQ-5 Data
int gearWheelToothRAEQ5 = 144;

// stepper
// duty will be used as factor how long to wait between microsteps
long duty = 50;
unsigned int waitMicroSeconds = 0;
unsigned int pulseCount = 0;
unsigned int microstepsGuiding = 0;
unsigned int microsecondsPerStep = 0;
int deviderRA = 1;

// stepperRA

// Transmission ratio gears (e.g. 16 teeth / 50 teeth => 1:3.125)
const float transmissionRatioRA  b= 3.125;

// Degrees with the specified stepper rotates in the given microstepmode per microstep
// will be calculated in setup():
float stepperRADegreePerStep16MicrostepsRA = 0.0;
float stepperRADefaultDegreePerStepRA = 1.8;
// Mode used for guiding
unsigned int stepperRAGuidingMicrostepMode = 0;
// Time for one step when 16 microsteps are used
unsigned long millisecondsPer16MicrostepsDegreeRA = 0;

// Delay for guiding - calculated in setup()
unsigned int waitMicroSecondsGuiding = 0; 
unsigned int pulseCountGuiding = 0;
unsigned int delayGuiding = 0;

// Pins
int A = 4;
int B = 5;
int C = 6;
int D = 7;


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void setup() {

  //////////////////////////////////////////
  // RTC
  //rtc.begin(DateTime(__DATE__, __TIME__));
  /*Initialize INT0 for accepting interrupts */
  PORTD |= 0x04; 
  DDRD &=~ 0x04;

  Serial.begin(9600);
  Wire.begin();

  rtc.begin();
  attachInterrupt(0, INT0_ISR, FALLING); 

  //Enable Interrupt 
  rtc.enableInterrupts(EverySecond); //interrupt at  EverySecond, EveryMinute, EveryHour
  // or this
  //rtc.enableInterrupts(0,0,5);    // interrupt at (h,m,s)
  //////////////////////////////////////////

  //////////////////////////////////////////
  // PINS
  pinMode(A, OUTPUT);     
  pinMode(B, OUTPUT);     
  pinMode(C, OUTPUT);     
  pinMode(D, OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  //////////////////////////////////////////

  //////////////////////////////////////////
  // Microsteps: 4 (full), 8(half), 16(quarter) 
  // - the more, the less torque
  // - the more microsteps, the less speed
  stepperRAGuidingMicrostepMode = 4;

  double deviderRA = 0.0;
  switch (stepperRAGuidingMicrostepMode) {
    case 16:
      deviderRA = 0.25;
      break;
    case 8:
      deviderRA = 0.5;
      break;
    case 4:
      deviderRA = 1;
      break;
    default:
      deviderRA = 1;
      break;
  };
  //////////////////////////////////////////

  //////////////////////////////////////////
  // Degrees with the specified stepper rotates in given microstepmode 
  // defaults to 0.1125° per step
  // stepperRADegreePerStep16MicrostepsRA = 
  //    ((stepperRADefaultDegreePerStepRA * stepperRAGuidingMicrostepMode) / 360 );
  // Time for one step when 16 microsteps are used
  // THIS IS THE GOAL -  equals to ~26.2628sec
  // millisecondsPer16MicrostepsDegreeRA = starTime24hSec / ( 360 * ( 1 / stepperRADegreePerStep16MicrostepsRA));
  
  waitMicroSecondsGuiding = 100;      // MICRO (!) seconds

  // Calculate for one revelution per second
  // 1: within startime RA makes exactely one revolution
  unsigned long stageOne   = starTime24hMillisec;
  Serial.println("Stage One: " + (String)stageOne);
  // 2. divide with 2,5 for what the gear wheel has to do in the same time 
  //    -> the gear wheel must run slower for the same speed
  unsigned long stageTwo   = stageOne * ( 360 / gearWheelToothRAEQ5 );
  Serial.println("Stage Two: " + (String)stageTwo);
  // 3. the gearwheel has to comensate the transmission ratio
  unsigned long stageThree = stageTwo * transmissionRatioRA;  
  Serial.println("Stage Three: " + (String)stageThree);
  // 4. if microsteps are used, speed must increase
  // Speed with 4000 here is a good value (4000 ~ 4 seconds)
  unsigned long stageFour  = 4000 * deviderRA;
  Serial.println("Stage Four: " + (String)stageFour);

  // Pulsecount for guiding with a factor
  pulseCountGuiding = (int)stageFour;

  intervalSeconds = (int)((pulseCountGuiding * waitMicroSecondsGuiding));
  Serial.println("IntervalSeconds: " + (String)intervalSeconds);


  microstepsGuiding = 1;
  //delayGuiding = 1000 / microstepsGuiding;
  delayGuiding = 0;

}

void one(){
  digitalWrite(A, HIGH);   
  digitalWrite(B, LOW);   
  digitalWrite(C, HIGH);   
  digitalWrite(D, LOW);   
}

void two(){
  digitalWrite(A, HIGH);   
  digitalWrite(B, LOW);   
  digitalWrite(C, LOW);   
  digitalWrite(D, HIGH);   
}

void three(){
  digitalWrite(A, LOW);   
  digitalWrite(B, HIGH);   
  digitalWrite(C, LOW);   
  digitalWrite(D, HIGH);   
}

void four(){
  digitalWrite(A, LOW);   
  digitalWrite(B, HIGH);   
  digitalWrite(C, HIGH);   
  digitalWrite(D, LOW);   
}


void oneB(){
  digitalWrite(A, HIGH);   
  digitalWrite(B, LOW);   
  digitalWrite(C, LOW);   
  digitalWrite(D, LOW);   
}

void twoB(){
  digitalWrite(A, LOW);   
  digitalWrite(B, LOW);   
  digitalWrite(C, LOW);   
  digitalWrite(D, HIGH);   
}

void threeB(){
  digitalWrite(A, LOW);   
  digitalWrite(B, HIGH);   
  digitalWrite(C, LOW);   
  digitalWrite(D, LOW);   
}

void fourB(){
  digitalWrite(A, LOW);   
  digitalWrite(B, LOW);   
  digitalWrite(C, HIGH);   
  digitalWrite(D, LOW);   
}


// main routine to microstep
void doStep(int st){
  
  long dt1 = waitMicroSeconds * duty / 100;
  long dt2 = waitMicroSeconds * (100-duty) / 100;

  for (int j = 0; j < pulseCount; j++){
    switch (st){
    case 1: one();break;
    case 2: two();break;
    case 3: three();break;
    case 4: four();break;
    case 11: oneB();break;
    case 12: twoB();break;
    case 13: threeB();break;
    case 14: fourB();break;

    case 21: one();break;
    case 22: two();break;
    case 23: three();break;
    case 24: four();break;
    case 31: oneB();break;
    case 32: twoB();break;
    case 33: threeB();break;
    case 34: fourB();break;

    }

    delayMicroseconds(dt1);

    switch (st){
    case 1: one();break;
    case 2: two();break;
    case 3: three();break;
    case 4: four();break;
    case 11: oneB();break;
    case 12: twoB();break;
    case 13: threeB();break;
    case 14: fourB();break;

    case 21: oneB();break;
    case 22: twoB();break;
    case 23: threeB();break;
    case 24: fourB();break;
    case 31: two();break;
    case 32: three();break;
    case 33: four();break;
    case 34: one();break;
    }
    delayMicroseconds(dt2);
    
  }
}

// disable motor
void motorOff(){
  /* Important note:
       Turning off the motor will make it go into a 'rest' state. 
       When using microsteps (or even full steps), this may not be the last active step. 
       So using this routine may change the position of the motor a bit.
  */
  
  digitalWrite(A, LOW);   
  digitalWrite(B, LOW);   
  digitalWrite(C, LOW);   
  digitalWrite(D, LOW);   
}

// full stepping 4 steps :
void do4Steps(int cnt, boolean forwards){
  for (int i = 0; i < cnt; i++){
    duty = 50;
    if (forwards)
      {for (int j = 1; j <= 4; j++){doStep(j);}}
    else
      {for (int j = 4; j >= 1; j--){doStep(j);}}

  }
}

// half stepping 8 steps :
void do8Steps(int cnt, boolean forwards){
  const int list[] = {1,11,2,12,3,13,4,14};
  for (int i = 0; i < cnt; i++){
    duty = 50;
    if (forwards)
      {for (int j = 0; j <= 7; j++){doStep(list[j]);}}
    else
      {for (int j = 7; j >= 0; j--){doStep(list[j]);}}
  }
}


// microstepping 16 steps :
void do16Steps(int cnt, boolean forwards){

  const int list[] = {1,21,11,31,2,22,12,32,3,23,13,33,4,24,14,34};
  for (int i = 0; i < cnt; i++){
    duty = 50;
    if (forwards)
      {for (int j = 0; j <= 15; j++){doStep(list[j]);}}
    else
      {for (int j = 15; j >= 0; j--){doStep(list[j]);}}
  }  
}
  
// microstepping >16 steps :
void doMoreSteps(int cnt, boolean forwards){
  const int list1[] = {1,11,2,12,3,13,4,14};
  const int list2[] = {21,31,22,32,23,33,24,34};
  
  for (int i = 0; i < cnt; i++){

    duty = 50;
    if (forwards)
      {for (int j = 0; j <= 7; j++){doStep(list1[j]); doSteps(list2[j], forwards);}}
    else
      {for (int j = 7; j >= 0; j--){doSteps(list2[j], forwards); doStep(list1[j]);}}
     
  }
}

// this routine handles >16 microsteps 
// uncomment sections to choose # steps
void doSteps(int st, boolean forwards){
  
// *********************** 24 steps 
/*
  if (forwards){
    duty = 66;    doStep(st);
    duty = 33;    doStep(st);
  }
  else{
    duty = 33;    doStep(st);
    duty = 66;    doStep(st);
  }
*/     


// *********************** 32 steps 
/*
  if (forwards){
    duty = 75;    doStep(st);
    duty = 50;    doStep(st);
    duty = 25;    doStep(st);
  }
  else{
    duty = 25;    doStep(st);
    duty = 50;    doStep(st);
    duty = 75;    doStep(st);
  }
*/     

// *********************** 48 steps 

/*  if (forwards){
    for (int i = 5; i >= 1; i--){duty = 17 * i; doStep(st);}
  }
  else{
    for (int i = 1; i <= 5; i++){duty = 17 * i; doStep(st);}
  }
*/     


// *********************** 64 steps 
/*
  if (forwards){
    for (int i = 7; i >= 1; i--){duty = 12 * i; doStep(st);}
  }
  else{
    for (int i = 1; i <= 7; i++){duty = 12 * i; doStep(st);}
  }
*/  
     
// *********************** 96 steps 
  if (forwards){
    for (int i = 9; i >= 1; i--){duty = 10 * i; doStep(st);}
  }
  else{
    for (int i = 1; i <= 9; i++){duty = 10 * i; doStep(st);}
  }

}

///////////////////////////////////////////////////////////////
/// PRINT TIME ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void printTime() {

    DateTime now = rtc.now(); //get the current date-time

    //print only when there is a change in seconds
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.date(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println(' ');

}


//Interrupt service routine for external interrupt on INT0 pin conntected to /INT
// void INT0_ISR()
// {
//   Serial.println(" External Interrupt detected ");
//   if (countSeconds >= 5) {

//     countSeconds = 0;
//     //print only when there is a change in seconds
//     printTime();
//     // rtc.clearINTStatus();
//   } else {
//       countSeconds += 1;
//   };
// }

void loop() {

   rtc.clearINTStatus();

};

///////////////////////////////////////////////////////////////
/// INTERRUPT /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void INT0_ISR() {

  countSeconds -= (int)( intervalSeconds * deviderRA );
  Serial.println(" External Interrupt detected (" 
          + (String)countSeconds + "/" 
          + (int)deviderRA + ")");

  if ( countSeconds >= intervalSeconds ) {
    doStepper = 1;
  } else {
    doStepper = 0;
  }

  ///////////////////////////////////////////////////////////////
  guidingMode = GUIDING;
  // guidingMode = WESTmax;

  // Do stepping - this goes far down to the end
  if ( doStepper == 1 ) {

    // uncomment this to disable motor  
    //  motorOff();return;

    /// SPEED /////////////////////////////////////////////////////
    /* control the speed of the motor with a waitMicroseconds/pulseCount pair
       e.g. waitMicroSeconds / pulseCount
            500 / 5 --> one step takes 2500 microseconds
            50 / 50 --> one step also takes 2500 microseconds
            but in second pair the fequency of the wave is 20kHz, not audible..
            in 500 / 5 you may hear a high tone in the motor in microsteps  
            some motors may not respond well on higher frequencies 
       note: these parameters also control speed in normal stepping (full or half) although there's no pulses in those cases, 
                setting pulseCount = 1 might be more readable
    */
    
    if (guidingMode == GUIDING) {
   
      waitMicroSeconds = waitMicroSecondsGuiding; 
      pulseCount = pulseCountGuiding;

    } else {

          switch (guidingMode) {
        case WESTx2:
          waitMicroSeconds = 50; pulseCount = 20;
          break; 
        case WESTx4:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        case WESTx8:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        case WESTmax:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        case EASTx2:
          waitMicroSeconds = 50; pulseCount = 20;
          break; 
        case EASTx4:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        case EASTx8:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        case EASTmax:
          waitMicroSeconds = 50; pulseCount = 20;
          break;
        default:
          waitMicroSeconds = 50; pulseCount = 20;
          break;

    } // end switch
  } // end if guidingmode == GUIDING

    /// STEPPING MODE /////////////////////////////////////////////
    /*  uncomment one of the next routines to choose stepping mode
      do4Steps : full stepping / Standard mode
      do8Steps : half stepping
      do16Steps : microstepping with 16 steps
      doMoreSteps : microstepping with more than 16 steps
         --> check routine 'doSteps' for extra uncomments      
         
      two parameters:
        1. number of steps
        2. forwards = true --> move forwards   /  forwards = false --> move backwards
        
      note: there's no speed correction for different modes, so do4Steps makes the motor go twice as fast as do8Steps and so on... 
            use waitMicroseconds and pulseCount to adapt speed when changing modes
         
    */
     
    if (guidingMode == GUIDING ) { 

      switch (guidingMode) {
      case GUIDING:
        switch (stepperRAGuidingMicrostepMode) {
          case 16:
            do16Steps(microstepsGuiding, true);
            break;
          case 8:
            do8Steps(microstepsGuiding, true);
            break;
          case 4:
            do4Steps(microstepsGuiding, true);
            break;
          default:
            do16Steps(microstepsGuiding, true);
            break;

        } // end switch
      } // end switch

      // delay until next step
      if ( delayGuiding > 0 ) {
         delay(delayGuiding);
      };

    // guiding Mode is else than GUIDING
    } else {
    
      switch (guidingMode) {
        case WESTx2:
          do16Steps(1, true);
          break; 
        case WESTx4:
          do8Steps(1, true);
          break;
        case WESTx8:
          do4Steps(1, true);
          break;
        case WESTmax:
          do4Steps(1, true);
          break;
        case EASTx2:
          do16Steps(1, false);
          break; 
        case EASTx4:
          do8Steps(1, false);
          break;
        case EASTx8:
          do4Steps(1, false);
          break;
        case EASTmax:
          do4Steps(1, false);
          break;
        default:
          do4Steps(1, false);
          break;

      } // end switch
    } // end if 

    doStepper = 0;
    Serial.println("wait ...");

  // no stepping mode
  } else {

    doStepper = 1;
    
  };

} // end ISR
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

