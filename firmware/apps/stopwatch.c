/*! \file stopwatch.c
  
  \brief Stopwatch application.
   
  This is a simple stop watch, which begins counting as the + key is
  hit and clears the count when the 0 key is pressed.  Because the
  RTC is busy with the real time, we count in quarter seconds as the
  display is updated.
  
  Hold the / key to briefly show the time of day without leaving the
  stopwatch or abandoning the count.

  We do not count time or store the count when in other applications.
  It would be nice to change that, so that something could be timed in
  the background while the user is doing other things.
*/

#include "api.h"
#include "stopwatch.h"
#include "apps/clock.h"

static int counting=0, showtime=0, settingtimer=0;

/* It's a crying shame, but you'll break the code if you increase this
   count to a long, because repeated divisions in rendering will take
   longer than the ~8k cycles we have per interrupt when running at
   32kHz and the stopwatch will run too slow.

   To work around this, we maintain the count of hours separately from
   the general second count.
 */
static uint16_t count, hour;
static uint8_t hourhex;
static uint8_t min, minhex;
static uint8_t sec, sechex;

static uint8_t hourtimer;
static uint8_t mintimer;
static uint8_t sectimer;

static uint8_t enabletimer=0;


//! Renders the low bits of the count in quarter seconds.
static const char subs[]={0, 0x25, 0x50, 0x75};

static void timer_set_draw(void);

//! Entry to the stopwatch app.
void stopwatch_init(){
  //Zero the count.
  count=hour=hourhex=min=minhex=sec=sechex=0;
  mintimer=3;
  hourtimer=sectimer=0;

  enabletimer=0;
  
  //Start off without counting.
  counting=0;
  
  //Force a draw on startup.
  stopwatch_draw(1);
}

//! Exit from the stopwatch app.
int stopwatch_exit(){
  //Give up without a fight when the mode button is pressed.
  return 0;
}

//! A button has been pressed for the stopwatch.
int stopwatch_keypress(char ch){
  unsigned char inputdigit=0;
  /* For now, we support only two buttons.  + stops and starts the
     count, while 0 resets the counter.
   */
  if(settingtimer<1){
    switch(ch){
    case '+':  //Pause/Resume the count.
      counting=!counting;
      break;
    case '0':  //Zero the count.
      count=hour=hourhex=min=minhex=sec=sechex=0;
      showtime=0;
      break;
    case '/':  //Briefly show the clock time.
      showtime=1;
      break;
    case '=': //go into set timer mode
      if(!counting)
        settingtimer=1;

      break;
    case '4':
      if(!enabletimer)
        enabletimer=1;
      else
        enabletimer=0;

      break;
    default:
      showtime=0;
      return 1;  //Redraw the whole screen on key-up.
      break;
    }
  } else{
     //We only handle numbers here.
    if(ch=='=') // exit set timer mode
      settingtimer=0;
    else if((ch&0x30)==0x30)
      inputdigit=ch&0x0F;
    else
      return 1;

    switch(settingtimer){
       case 1:         //Hour
        hourtimer=inputdigit*10+hourtimer%10;
        settingtimer++;
        break;
      case 2:
        hourtimer=hourtimer-hourtimer%10+inputdigit;
        settingtimer++;
        break;
      case 3:         //Minute
        mintimer=inputdigit*10+mintimer%10;
        if(mintimer>59)
          mintimer = 59;

        settingtimer++;
        break;
      case 4:
        mintimer=mintimer-mintimer%10+inputdigit;
        if(mintimer>59)
          mintimer = 59;

        settingtimer++;
        break;
      case 5:         //Second
        sectimer=inputdigit*10+sectimer%10;
        if(sectimer>59)
          sectimer=59;

        settingtimer++;
        break;
      case 6:
        sectimer=sectimer-sectimer%10+inputdigit;
        if(sectimer>59)
          sectimer=59;

      default:
        hourhex=minhex=sechex=0; // reset display
        settingtimer=0; // exit set timer mode after last digit and in any other case
        break;
    }
    return 1; // force redraw after setting the timer
  }
  /* Stopwatch uses rendering frequency to count time, so we don't
     redraw after a keypress when we are counting. */
  return !counting;
}


//! Draw the stopwatch app and handle its input.
void stopwatch_draw(int forced){
  uint8_t subhex;
  
  /* The stopwatch is special in that it never times out.  Be very
     careful when doing this, because a minor bug might kill the
     battery.
   */
  app_cleartimer();  
  
  if(settingtimer<1){
    //If we aren't counting and there's not been a keypress, don't
    //bother drawing.
    if(!forced && !counting)
      return;

      //Increment the count if we're counting.
    if(counting)
      count++;

    //Update the subhex field.
    subhex=subs[count&3];

    //Handle the second rollover every forth tick.
    if((count&3)==3){
      sec++;
      sechex=int2bcd(sec);
    }

    //Handle the minute rollover every 60 seconds.
    if(sec>=60){
      sec-=60;
      sechex=int2bcd(sec);
      min++;
      minhex=int2bcd(min);
    }
    
    //Handle hour rollover every 60 minutes
    if(min>=60){
      min-=60;
      minhex=int2bcd(min);
      hour++;
      hourhex=int2bcd(hour);
    }

    if((enabletimer)&&(hourtimer==hour)&&(mintimer==min)&&(sectimer==sec)){
        tone(2048, 250);
        tone(1024, 250);
        tone(512, 250);
        tone(256, 250);
    }
    
    //When / is held, we always show the time and exit.
    if(showtime){
      draw_time(1);
      return;
    }
  } 

  if(forced){
    //Draw these once, rather than every frame.
    //lcd_cleardigit(5); //Space
    //lcd_cleardigit(2); //Space
    lcd_zero();
  }

  setplus(enabletimer);
  
  //Blink the colon once a second.
  setcolon((count>>1)&1);

  if(settingtimer>0){
    timer_set_draw();
    return;
  }

  
  //We either draw hhmmss or mmssSS.
  if(hour){ //hhmmss
    lcd_digit(1,sechex>>4);
    lcd_digit(0,sechex&0xF);
    
    //Draw minutes and hours
    if(!sec || forced){
      //Draw minutes
      lcd_digit(4,minhex>>4);
      lcd_digit(3,minhex&0xF);
      
      //Draw hours
      lcd_digit(7,hourhex>>4);
      lcd_digit(6,hourhex&0xF);
    }
  }else{ // mmssSS
    //Draw the subsecond first.
    lcd_digit(1,subhex>>4);
    lcd_digit(0,subhex&0xF);
    
    //Only draw the rest if the subseconds have changed.
    if(!subhex || count==1 || forced){
      lcd_digit(4,sechex>>4);
      lcd_digit(3,sechex&0xF);
      
      //Update minutes if the seconds are zero.
      if(!sec || forced){
      	lcd_digit(7,minhex>>4);
      	lcd_digit(6,minhex&0xF);
      }
    }
  }
}

static void timer_set_draw(void){
  static int flicker=0;
  flicker ^=1;

  sechex=int2bcd(sectimer);
  minhex=int2bcd(mintimer);
  hourhex=int2bcd(hourtimer);

  lcd_digit(1,sechex>>4);
  lcd_digit(0,sechex&0xF);
  lcd_digit(4,minhex>>4);
  lcd_digit(3,minhex&0xF);
  lcd_digit(7,hourhex>>4);
  lcd_digit(6,hourhex&0xF);

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
      
    case 5:        //Second
      if(flicker)
        lcd_cleardigit(1);
      break;
    case 6:
      if(flicker)
        lcd_cleardigit(0);
      break;
  }
}
