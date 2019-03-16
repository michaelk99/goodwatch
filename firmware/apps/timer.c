/*! \file timer.c
  \brief tea timer clock application.
  
*/

// for some reason this isn't in the msp430-gcc packages, but is in TI source
#define RTCAE (0x80) /* Real Time Clock timer enable */

#define BEEP_CNT 5

//#define TIMER_BASE              TIMER_A0_BASE
//#define TIMER_CLK_HZ            32768

#include <msp430.h>
#include "api.h"

void initTimer(void);
void startTimer(void);
void pauseTimer(void);
void stopTimer(void);
void setTimer(unsigned int sec, unsigned int min);
void resetTimer(void);

//! If non-zero, we are setting the timer.
static int settingtimer=0;
static unsigned char timerrunning = 0;
static volatile unsigned int sec = 0, min = 3;

/*********************** ISR ****************************/

// CCR0 ISR
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void){
  //Timer_A_stop(TIMER_BASE);
}

/********************************************************/


//! Gets timer set status
static int timer_enabled() {
  return timerrunning;
}

//! Toggle timer enable bits
static void toggle_timer(int enable) {
  switch(enable){
    case 1:
            startTimer(void);
            timerrunning = 1;
            break;
    case 2:
            pauseTimer(void);
            timerrunning = 0;
            break;
    case 0: 
            stopTimer(void);
            timerrunning = 0;
            break;
    default:
            //shouldn reach this point
  }
}


//! Draws the timer.
static void draw_timer(){
  
  lcd_digit(7,sec/10);
  lcd_digit(6,sec%10);
  lcd_cleardigit(5); //Space
  setcolon(1);
  lcd_digit(4,min/10);
  lcd_digit(3,min%10);
  lcd_cleardigit(2); //Space
  lcd_char(1, 'a');
  lcd_char(0, 'l');
  
  if(timer_enabled())
    setplus(1);
  else
    setplus(0);

}


//! Draws whatever is being set
static void draw_settingtimer(){
  
  lcd_digit(7,sec/10);
  lcd_digit(6,sec%10);
  lcd_cleardigit(5); //Space
  setcolon(1);
  lcd_digit(4,min/10);
  lcd_digit(3,min%10);
  lcd_cleardigit(2); //Space
  lcd_cleardigit(1); //Space
  lcd_cleardigit(0); //Space


  static int flicker=0;
  
  flicker^=1;

  switch(settingtimer){
  case 1:         //Hour
    if(flicker)
      lcd_cleardigit(7);
    break;
  case 2:
    if(flicker)
      lcd_cleardigit(6);
    break;
  case 3:         //Minute
    if(flicker)
      lcd_cleardigit(4);
    break;
  case 4:
    if(flicker)
      lcd_cleardigit(3);
    break;
  }
}


//! Entry to the timer app.
void timer_init(){
  lcd_zero();
  //initTimer();
}

//! Exit timer when the sidebutton is pressed, unless we are programming.
int timer_exit(){
  if(settingtimer){
    //Setting the timer, so jump to next digit.
    settingtimer++;
    if (settingtimer > 4)
      settingtimer=0;
    return 1;
  }else{
    //Not setting the timer, so just move on to next app.
    setplus(0);
    return 0;
  }
}

static char lastchar=0;

//! Draws the timer time in the main application.
void timer_draw(){
  /* The SET button will move us into the programming mode. */
  if(sidebutton_set()){
    settingtimer=!settingtimer;
  }

  if(settingtimer)
    draw_settingtimer();
  else
    draw_timer();
}

//! A button has been pressed for the timer.
int timer_keypress(char ch){
  unsigned char inputdigit=0;
  unsigned char i;
  lastchar=ch;
  
  if(settingtimer){

    //We only handle numbers here.
    if((ch&0x30)==0x30)
      inputdigit=ch&0x0F;
    else
      return 1;
    
    switch(settingtimer){
    case 1:         //Secound
      sec = inputdigit*10+sec%10;
      settingtimer++;
      break;
    case 2:
      sec = sec-sec%10+inputdigit;
      settingtimer++;
      break;
    case 3:         //Minute
      min = inputdigit*10+min%10;
      settingtimer++;
      break;
    case 4:
      min = min-min%10+inputdigit;
      settingtimer=0;
      toggle_timer(1);
      break;
    default:
      // Return to normal mode if we end up here accidentally
      settingtimer=0;
    }
  } else {
    switch(ch){
    case '4': //Toggle the timer.
      if (timer_enabled())
        toggle_timer(0);
      else
        toggle_timer(1);
      break;
    case '.': // beep a little
      for(i = 0; i < BEEP_CNT;i++){
        setam(1);
        setpm(1);
        while(tone(2048, 250));
        setam(1);
        setpm(1);
        while(tone(0, 500)); 
      }
      break;

    }
  }
  return 1;
}

void initTimer(void){
  /*
    Timer_A_initUpModeParam timerCfg;

    timerCfg.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
    timerCfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timerCfg.timerPeriod = TIMER_CLK_HZ;
    timerCfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    timerCfg.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;
    timerCfg.timerClear = TIMER_A_DO_CLEAR;
    timerCfg.startTimer = false;
    Timer_A_initUpMode(TIMER_BASE, &timerCfg);
  */
}

void startTimer(void){
    //Timer_A_clear(TIMER_BASE);
  
    //Timer_A_startCounter(TIMER_BASE, TIMER_A_UP_MODE);
}

void stopTimer(void){
  //Timer_A_stop(TIMER_BASE);
  //Timer_A_clear(TIMER_BASE);
}

