/*        Your Name & E-mail: Michael Tin, mtin002@ucr.edu

*          Discussion Section: 023

 *         Assignment: Custom Laboratory Project

 *         Exercise Description: [optional - include for your own benefit]

 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: 

 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"
#include "usart_ATmega328p.h"

#define NUM_TASKS 2 //TODO: Change to the number of tasks being used

char receivedChar = '2';
bool soundOn = false;
int time = -1;

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long receivePeriod = 100;
const unsigned long S_PERIOD = 100;
const unsigned long GCD_PERIOD = 100;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Declare your tasks' function and their states here
enum receiveStates {receiveOn};
enum S_STATES {S_OFF, S_ON};

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

//TODO: Create your tick functions for each task

int Ab = 4818;
int Bb = 4290;
int C = 7661;
int D = 6824;
int Eb = 6429;
int F = 5729;
int G = 5101;

//len = 624
int musicArray[] = {
    Eb, 0, Eb, 0, C, 0, 0, Eb, 0, Eb, 0, C, 0, 0,
    Eb, 0, Eb, 0, C, 0, Eb, Eb, Eb, 0, C, 0, 0, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, 0, 0,
    Bb, 0, C, 0, Bb, 0, C, 0,

    Eb, Eb, 0, C, C, 0, Eb, Eb, 0, C, C, 0,
    Eb, 0, Eb, 0, C, 0, Eb, Eb, Eb, 0, C, C, 0, 0, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, 0,
    C, 0, D, 0, Eb, 0,

    Ab, Ab, 0, Ab, 0, 0, Ab, 0, 0, G, 
    0, G, 0, G, 0, D, D, 0, 0, D, 
    D, 0, 0, Eb, Eb, 0, 0, F, F, 0, 0, G, 
    G, 0, G, G, 0, G, G, 0, F, F, 0, 
    F, 0, F, 0, F, F, 0, Eb, Eb, 0, 
    G, G, 0, 

    Ab, Ab, 0, Ab, 0, Ab, 0, G, 0, 0, G, 0, G, 
    0, D, 0, 0, D, 0, Eb, 0, 0, F, 0, G, 0, G, 
    0, G, 0, G, 0, G, 0, F, 0, Eb, Eb, 0, Bb, Bb,
    0, Eb, Eb, 0, D, D, 0, Eb, Eb, 0,

    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0, 
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    C, 0, 0, 0, Bb, 0, C, 0, Bb, 0, C, 0,

    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0, 
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    C, 0, 0, 0, Bb, 0, C, 0, Bb, 0, C, 0,

    Eb, 0, Eb, 0, Eb, 0, Eb, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0, 
    C, 0, Eb, Eb, Eb, 0, 0, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, 0, 0,
    Bb, 0, C, 0, Bb, 0, C, 0, 0, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, Eb, 0, Eb, Eb, 0, Eb, Eb, 0, Eb, Eb, 0, Eb, 0, C, 0, Eb, 0, Eb, Eb, 0, 0, 0,
    Eb, 0, Eb, 0, Eb, 0, Eb, 0, Eb, 0, 0, Eb, 0, Eb, 0, 0, 0, C, 0, D, 0, Eb, 0,

    Ab, Ab, 0, Ab, 0, 0, Ab, 0, 0, G, 
    0, G, 0, G, 0, D, D, 0, 0, D, 
    D, 0, 0, Eb, Eb, 0, 0, F, F, 0, 0, G, 
    G, 0, G, G, 0, G, G, 0, F, F, 0, 
    F, 0, F, 0, F, F, 0, Eb, Eb, 0, 
    G, G, 0, 

    Ab, Ab, 0, Ab, 0, Ab, 0, G, 0, 0, G, 0, G, 
    0, D, 0, 0, D, 0, Eb, 0, 0, F, 0, G, 0, G, 
    0, G, 0, G, 0, G, 0, F, 0, Eb, Eb, 0, Bb, Bb,
    0, Eb, Eb, 0, D, D, 0, Eb, Eb, 0,

    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0, 
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    C, 0, 0, 0, Bb, 0, C, 0, Bb, 0, C, 0,

    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0, 
    Bb, Bb, 0, Bb, 0, Bb, 0, Bb, 0, Bb, 0, 0,
    C, 0, 0, 0, Bb, 0, C, 0, Bb, 0, C, 0
};

int TickFct_Receive(int state) {
  switch (state) {
    case receiveOn:
      state = receiveOn;
      break;
  }

  switch (state) {
    case receiveOn:
      if (USART_HasReceived()) {
        receivedChar = USART_Receive();
        if (receivedChar == '1') {
          PORTB = SetBit(PORTB, 5, 0);
          soundOn = false;
        }
        else if (receivedChar == '0') {
          PORTB = SetBit(PORTB, 5, 1);
          soundOn = true;
        }
      }
      break;
  }
  return state;
}

int TickFct_S(int state) {
  switch (state) {
    case S_OFF:
      if (soundOn == true) {
        state = S_ON;
      }
      else {
        state = S_OFF;
      }
      break;

    case S_ON:
      if (soundOn == false) {
        state = S_OFF;
      }
      else {
        state = S_ON;
      }
      break;
  }

  switch (state) {
    case S_OFF:
      time = -1;
      ICR1 = 0;
      OCR1A = ICR1 / 2;
      break;

    case S_ON:
      time++;
      if (time > 623) {
        time = 0;
      }
      ICR1 = musicArray[time];
      OCR1A = ICR1 / 2;
      break;
  }
  
  return state;
}


int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRB = 0xFF;
    PORTB = 0x00;

    DDRC = 0x00;
    PORTC = 0xFF;

    DDRD = 0xFF;
    PORTD = 0x00;

    // ADC_init();   // initializes ADC
    initUSART();
    serial_init(9600);

    //TODO: Initialize timer1
    TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
    //WGM11, WGM12, WGM13 set timer to fast pwm mode
    ICR1 = 39999; //20ms pwm period

    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;

    tasks[0].period = receivePeriod;
    tasks[0].state = receiveOn;
    tasks[0].elapsedTime = tasks[0].period;
    tasks[0].TickFct = &TickFct_Receive;

    tasks[1].period = S_PERIOD;
    tasks[1].state = S_OFF;
    tasks[1].elapsedTime = tasks[0].period;
    tasks[1].TickFct = &TickFct_S;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}