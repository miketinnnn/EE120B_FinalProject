/*        Your Name & E-mail: Michael Tin, mtin002@ucr.edu

*          Discussion Section: 023

 *         Assignment: Custom Laboratory Project

 *         Exercise Description: [optional - include for your own benefit]

 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: 

 */

/*  
    For the checkGame() function, I used information about the algorithm from this site: 
    https://stackoverflow.com/questions/39062111/java-how-to-check-diagonal-connect-four-win-in-2d-array
    
    I mentioned this to RB (the TA for this class) and also included the reference in my Project Report.
*/

#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"
#include "spiAVR.h"
#include "usart_ATmega328p.h"

char message0[] = "Connect 4: Idle";
char message1[] = "Connect 4: Play";
char message2[] = "Connect 4: End";
char message3[] = "Player 1";
char message4[] = "Player 2";
char message5[] = "Player 1 Wins!";
char message6[] = "Player 2 Wins!";
char message7[] = "Game Was Tied!";
char message8[] = "Col: ";

bool gameIdle = true;
bool gameEnd = false;
bool player1Turn = false;
bool player2Turn = false;
bool player1Won = false;
bool player2Won = false;
bool gameWasTied = false;

int time = 15;
int selection = 1;
int board[6][7] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}
};

#define NUM_TASKS 8 //TODO: Change to the number of tasks being used

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long displayPeriod = 1000;
const unsigned long lcdPeriod = 200;
const unsigned long timerdisplayPeriod = 100;
const unsigned long leftbuttonPeriod = 200;
const unsigned long gamePeriod = 200;
const unsigned long rightbuttonPeriod = 200;
const unsigned long ledPeriod = 200;
const unsigned long transmitPeriod = 100;
const unsigned long GCD_PERIOD = 100; //TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Declare your tasks' function and their states here
enum displayStates {displayOn};
enum lcdStates {lcdIdle, lcdGamePlayer1, lcdGamePlayer2, lcdEndPlayer1, lcdEndPlayer2, lcdEndTied};
enum timerdisplayStates {timerdisplayOn};
enum leftbuttonStates {leftbuttonIdle, leftbuttonPress};
enum gameStates {gameStart, gamePlayer1, gamePlayer2, gameOver};
enum rightbuttonStates {rightbuttonIdle, rightbuttonPress};
enum ledStates {ledOn};
enum transmitStates {transmitOn};

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

void Send_Command(char c) {
  PORTC = SetBit(PORTC, 5, 0); // Set A0 to 0 to send a command.
  SPI_SEND(c);
}

void Send_Data(char c) {
  PORTC = SetBit(PORTC, 5, 1); // Set A0 to 1 to send data.
  SPI_SEND(c);
}

void HardwareReset() {
  PORTB = SetBit(PORTB, 0, 0); // Set Reset Pin to LOW.
  _delay_ms(200);
  PORTB = SetBit(PORTB, 0, 1); // Set Reset Pin to HIGH.
  _delay_ms(200);
}

void ST7735_init() {
  HardwareReset(); // Perform a hardware reset.
  Send_Command(0x01); // SWRESET
  _delay_ms(150);
  Send_Command(0x11); // SLPOUT
  _delay_ms(200);
  Send_Command(0x3A); // COLMOD
  Send_Data(0x05); // Send 0x05 for 16-bit color mode.
  _delay_ms(10);
  Send_Command(0x29); // DISPON
  _delay_ms(200);
}

void ST7735_write() {
    // 16-bit color values from: https://demmel.com/ilcd/help/16BitColorValues.htm

    // Initialize the entire screen to white
    Send_Command(0x2A); // CASET
    Send_Data(0x00); // XS15 - XS8 (Start column high byte = 0)
    Send_Data(0x00); // XS7 - XS0  (Start column low byte = 0)
    Send_Data(0x00); // XE15 - XE8 (End column high byte = 0)
    Send_Data(0x7F); // XE7 - XE0  (End column low byte = 127) 

    Send_Command(0x2B); // RASET
    Send_Data(0x00); // YS15 - YS8 (Start row high byte = 0)
    Send_Data(0x00); // YS7 - YS0  (Start row low byte = 0)
    Send_Data(0x00); // YE15 - YE8 (End row high byte = 0)
    Send_Data(0x7F); // YE7 - YE0  (End row low byte = 127)

    Send_Command(0x2C); // RAMWR
    for (int i = 0; i < 128 * 128; i++) { 
        Send_Data(0xFF);
        Send_Data(0xFF);
    }

    char col_starts[] = {0x01, 0x13, 0x25, 0x37, 0x49, 0x5B, 0x6D}; // 1, 19, 37, 55, 73, 91, 109 
    char col_ends[] =   {0x12, 0x24, 0x36, 0x48, 0x5A, 0x6C, 0x7E}; // 18, 36, 54, 72, 90, 108, 126
    char row_starts[] = {0x01, 0x16, 0x2B, 0x40, 0x55, 0x6A}; // 1, 22, 43, 64, 85, 106
    char row_ends[] =   {0x15, 0x2A, 0x3F, 0x54, 0x69, 0x7E};       // 21, 42, 63, 84, 105, 126
    int m = -1;
    int n = -1;

    for (int i = 6; i >= 0; --i) { // Iterate backwards because screen is physically flipped
      m++;
      n = -1;
      for (int j = 5; j >= 0; --j) {
        n++;
        Send_Command(0x2A); // CASET
        Send_Data(0x00); // XS15 - XS8 (Start column high byte)
        Send_Data(col_starts[i]); // XS7 - XS0  (Start column low byte)
        Send_Data(0x00); // XE15 - XE8 (End column high byte)
        Send_Data(col_ends[i]); // XE7 - XE0  (End column low byte)

        Send_Command(0x2B); // RASET
        Send_Data(0x00); // YS15 - YS8 (Start row high byte)
        Send_Data(row_starts[j]); // YS7 - YS0  (Start row low byte)
        Send_Data(0x00); // YE15 - YE8 (End row high byte)
        Send_Data(row_ends[j]); // YE7 - YE0  (End row low byte)

        Send_Command(0x2C); // RAMWR
        for (int i = 0; i < 18; i++) {    // For each column (18)
          for (int j = 0; j < 21; j++) {  // For each row (21)
              if (board[n][m] == 0) {
                Send_Data(0xFF);
                Send_Data(0xFF);
              }
              else if (board[n][m] == 1) {
                Send_Data(0x00);
                Send_Data(0x1F);
              }
              else if (board[n][m] == 2) {
                Send_Data(0x07);
                Send_Data(0xFF);
              }
              else {
                Send_Data(0xFF);
                Send_Data(0xFF);
              }
          }
        }
      }
    }
}

int vals[16] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A
  0b01111100, // B
  0b00111001, // C
  0b01011110, // D
  0b01111001, // E
  0b01110001  // F
};

void shiftOut(int val) {
  for (int i = 0; i < 8; ++i) {
    PORTC = SetBit(PORTC, 0, GetBit(val, 7 - i)); // data
    PORTC = SetBit(PORTC, 2, 1); // clk
    PORTC = SetBit(PORTC, 2, 0); // clk
  }
  PORTC = SetBit(PORTC, 1, 1); // latch
  PORTC = SetBit(PORTC, 1, 0); // latch
}

void resetGame(int board[6][7]) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            board[i][j] = 0; 
        }
    }
}

int directions[4][2] = {
    {0, 1}, // Horizontal check
    {1, 0}, // Vertical check
    {1, 1}, // Right diagonal check
    {1, -1} // Left diagonal check
};

int checkGame(int board[6][7]) {
  for (int i = 0; i < 7; ++i) {
    if (board[0][i] == 0) {
      break;
    }
    else {
      if (i == 6) {
        if (board[0][i] != 0) {
          return -1;
        }
      }
    }
  }
  for (int i = 0; i < 4; ++i) {
    int currRow = directions[i][0];
    int currCol = directions[i][1];

    for (int j = 0; j < 6; ++j) {
      for (int k = 0; k < 7; ++k) {
        int prevRow = j + (currRow * 3);
        int prevCol = k + (currCol * 3);

        if ((prevRow >= 0) && (prevRow < 6) && (prevCol >= 0) && (prevCol < 7)) {
          int type = board[j][k];
          if (type != 0) {
            if ((type == board[j + currRow][k + currCol]) && (type == board[j + (currRow * 2)][k +(currCol * 2)]) && (type == board[prevRow][prevCol])) {
              return type;
            }
          }
        }
      }
    }
  }
  return 0;
}

void placePiece(int board[6][7], int select, int numPlayer) {
  if ((select >= 0) && (select < 7)) {
    if (board[0][select] == 0) {
      for (int i = 5; i >= 0; i--) {
        if (board[i][select] == 0) {
          board[i][select] = numPlayer;
          break;
        }
      }
    }
  }
}

int TickFct_Display(int state) {
  switch (state) {
    case displayOn:
      state = displayOn;
      break;
  }

  switch (state) {
    case displayOn:
      ST7735_write();
      break;
  }
  return state;
}

int TickFct_LCD(int state) {
  switch (state) {
    case lcdIdle:
      if (gameIdle) {
        state = lcdIdle;
      }
      else if (!gameIdle) {
        if (player1Turn) {
          state = lcdGamePlayer1;
        }
        else if (player2Turn) {
          state = lcdGamePlayer2;
        }
      }
      break;

    case lcdGamePlayer1:
      if (gameEnd) {
        if (player1Won) {
          state = lcdEndPlayer1;
        }
        else if (player2Won) {
          state = lcdEndPlayer2;
        }
        else if (gameWasTied) {
          state = lcdEndTied;
        }
      }
      else {
        if (gameIdle) {
          state = lcdIdle;
        }
        else if (!gameIdle) {
          if (player1Turn) {
            state = lcdGamePlayer1;
          }
          else if (player2Turn) {
            state = lcdGamePlayer2;
          }
        }
      }
      break;

    case lcdGamePlayer2:
      if (gameEnd) {
        if (player1Won) {
          state = lcdEndPlayer1;
        }
        else if (player2Won) {
          state = lcdEndPlayer2;
        }
        else if (gameWasTied) {
          state = lcdEndTied;
        }
      }
      else {
        if (gameIdle) {
          state = lcdIdle;
        }
        else if (!gameIdle) {
          if (player1Turn) {
            state = lcdGamePlayer1;
          }
          else if (player2Turn) {
            state = lcdGamePlayer2;
          }
        }
      }
      break;

    case lcdEndPlayer1:
      state = lcdEndPlayer1;
      break;

    case lcdEndPlayer2:
      state = lcdEndPlayer2;
      break;

    case lcdEndTied:
      state = lcdEndTied;
      break;
  }

  switch (state) {
    case lcdIdle:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message0);
      lcd_goto_xy(1, 0);
      break;

    case lcdGamePlayer1:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message1);
      lcd_goto_xy(1, 0);
      lcd_write_str(message3);
      lcd_goto_xy(1, 10);
      lcd_write_str(message8);
      lcd_write_character(selection + '0');
      break;

    case lcdGamePlayer2:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message1);
      lcd_goto_xy(1, 0);
      lcd_write_str(message4);
      lcd_goto_xy(1, 10);
      lcd_write_str(message8);
      lcd_write_character(selection + '0');
      break;

    case lcdEndPlayer1:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message2);
      lcd_goto_xy(1, 0);
      lcd_write_str(message5);
      break;

    case lcdEndPlayer2:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message2);
      lcd_goto_xy(1, 0);
      lcd_write_str(message6);
      break;

    case lcdEndTied:
      lcd_clear();
      lcd_goto_xy(0, 0);
      lcd_write_str(message2);
      lcd_goto_xy(1, 0);
      lcd_write_str(message7);
      break;
  }

  return state;
}

int TickFct_TimerDisplay(int state) {
  switch (state) {
    case timerdisplayOn:
      state = timerdisplayOn;
      break;
  }

  switch (state) {
    case timerdisplayOn:
      shiftOut(vals[time]);
      break;
  }

  return state;
}

int TickFct_LeftButton(int state) {
  switch (state) {
    case leftbuttonIdle:
      if (GetBit(PINC, 3)) {
        state = leftbuttonPress;
      }
      else {
        state = leftbuttonIdle;
      }
      break;

    case leftbuttonPress:
      if (!GetBit(PINC, 3)) {
        if (!gameEnd) {
          gameIdle = !gameIdle;
        }
        state = leftbuttonIdle;
      }
      else {
        state = leftbuttonPress;
      }
      break;
  }

  switch (state) {
    case leftbuttonIdle:
      break;

    case leftbuttonPress:
      break;
  }

  return state;
}

int TickFct_Game(int state) {
  switch (state) {
    case gameStart:
      if (gameEnd) {
        state = gameOver;
      }
      else {
        if (gameIdle) {
          state = gameStart;
        }
        else if (!gameIdle) {
          time = 15;
          player1Turn = true;
          player2Turn = false;
          state = gamePlayer1;
        }
      }
      break;

    case gamePlayer1:
      if (gameIdle) {
        time = 15;
        state = gameStart;
        player1Turn = false;
        player2Turn = false;
        resetGame(board);
      }
      else {
        time--;
        if (time < 0) {
          time = 15;
          placePiece(board, (selection - 1), 1);
          if (checkGame(board) == 1) {
            player1Won = true;
            player2Won = false;
            player1Turn = false;
            player2Turn = false;
            gameWasTied = false;
            state = gameOver;
          }
          else if (checkGame(board) == 2) {
            player1Won = false;
            player2Won = true;
            player1Turn = false;
            player2Turn = false;
            gameWasTied = false;
            state = gameOver;
          }
          else if (checkGame(board) == -1) {
            player1Won = false;
            player2Won = false;
            player1Turn = false;
            player2Turn = false;
            gameWasTied = true;
            state = gameOver;
          }
          else {
            player1Turn = false;
            player2Turn = true;
            state = gamePlayer2;
          }
        }
        else {
          state = gamePlayer1;
        }
      }
      break;
      
    case gamePlayer2:
      if (gameIdle) {
        time = 15;
        state = gameStart;
        player1Turn = false;
        player2Turn = false;
        resetGame(board);
      }
      else {
        time--;
        if (time < 0) {
          time = 15;
          placePiece(board, (selection - 1), 2);
          if (checkGame(board) == 1) {
            player1Won = true;
            player2Won = false;
            player1Turn = false;
            player2Turn = false;
            state = gameOver;
          }
          else if (checkGame(board) == 2) {
            player1Won = false;
            player2Won = true;
            player1Turn = false;
            player2Turn = false;
            state = gameOver;
          }
          else if (checkGame(board) == -1) {
            player1Won = false;
            player2Won = false;
            player1Turn = false;
            player2Turn = false;
            gameWasTied = true;
            state = gameOver;
          }
          else {
            player1Turn = true;
            player2Turn = false;
            state = gamePlayer1;
          }
        }
        else {
          state = gamePlayer2;
        }
      }
      break;

    case gameOver:
      state = gameOver;
      break;
  }

  switch (state) {
    case gameStart:
      break;

    case gamePlayer1:
      break;

    case gamePlayer2:
      break;

    case gameOver:
      gameEnd = true;
      break;
  }

  return state;
}

int TickFct_RightButton(int state) {
  switch (state) {
    case rightbuttonIdle:
      if (GetBit(PINC, 4)) {
        state = rightbuttonPress;
      }
      else {
        state = rightbuttonIdle;
      }
      break;

    case rightbuttonPress:
      if (!GetBit(PINC, 4)) {
        selection++;
        if (selection > 7) {
          selection = 1;
        }
        state = rightbuttonIdle;
      }
      else {
        state = rightbuttonPress;
      }
      break;
  }

  switch (state) {
    case rightbuttonIdle:
      break;

    case rightbuttonPress:
      break;
  }

  return state;
}

int TickFct_LED(int state) {
  switch (state) {
    case ledOn:
      state = ledOn;
      break;
  }

  switch (state) {
    case ledOn:
      if (gameEnd) {
        PORTB = SetBit(PORTB, 4, 0);
      }
      else if (!gameEnd) {
        if (!gameIdle) {
          PORTB = SetBit(PORTB, 4, 1);
        }
        else {
          PORTB = SetBit(PORTB, 4, 0);
        }
      }
      break;
  }

  return state;
}

int TickFct_Transmit(int state) {
  switch (state) {
    case transmitOn:
      state = transmitOn;
      break;
  }

  switch (state) {
    case transmitOn:
      if (USART_IsSendReady()) {
        USART_Send(gameIdle ? '1' : '0');
      }
      break;
  }

  return state;
}

int main(void) {
    //TODO: initialize all your inputs and ouputs

    // ADC_init();   // initializes ADC

    DDRB = 0xFF;
    PORTB = 0x00;
    
    DDRC = 0xE7; // 0b11100111
    PORTC = 0x19; // 0b00011001

    DDRD = 0xFF;
    PORTD = 0x00;

    SPI_INIT();
    PORTB = SetBit(PORTB, 2, 0);
    ST7735_init();
    lcd_init();
    initUSART();
    // serial_init(9600);

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

    tasks[0].period = displayPeriod;
    tasks[0].state = displayOn;
    tasks[0].elapsedTime = tasks[0].period;
    tasks[0].TickFct = &TickFct_Display;

    tasks[1].period = lcdPeriod;
    tasks[1].state = lcdIdle;
    tasks[1].elapsedTime = tasks[1].period;
    tasks[1].TickFct = &TickFct_LCD;

    tasks[2].period = timerdisplayPeriod;
    tasks[2].state = timerdisplayOn;
    tasks[2].elapsedTime = tasks[2].period;
    tasks[2].TickFct = &TickFct_TimerDisplay;

    tasks[3].period = leftbuttonPeriod;
    tasks[3].state = leftbuttonIdle;
    tasks[3].elapsedTime = tasks[3].period;
    tasks[3].TickFct = &TickFct_LeftButton;

    tasks[4].period = gamePeriod;
    tasks[4].state = gameStart;
    tasks[4].elapsedTime = tasks[4].period;
    tasks[4].TickFct = &TickFct_Game;

    tasks[5].period = rightbuttonPeriod;
    tasks[5].state = rightbuttonIdle;
    tasks[5].elapsedTime = tasks[5].period;
    tasks[5].TickFct = &TickFct_RightButton;

    tasks[6].period = ledPeriod;
    tasks[6].state = ledOn;
    tasks[6].elapsedTime = tasks[6].period;
    tasks[6].TickFct = &TickFct_LED;

    tasks[7].period = transmitPeriod;
    tasks[7].state = transmitOn;
    tasks[7].elapsedTime = tasks[7].period;
    tasks[7].TickFct = &TickFct_Transmit;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}