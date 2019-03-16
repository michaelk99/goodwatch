/*! \file timer.h
  \brief tea timer application.
*/

//! Initialize the timer.
void timer_init();
//! Exit to the next application.
int timer_exit();
//! Draw the timer.
void timer_draw();

//! A button has been pressed for the timer.
int timer_keypress(char ch);

//! Toggle the timer
void toggle_timer(int enable);

