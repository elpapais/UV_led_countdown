/*
 * main.cpp
 *
 *  Created on: 18/08/2013
 *      Author: Jo�o
 */

#include <msp430.h>
#include "msp430g2231-calibration.h"
#include "flash.h"
#include "display.h"


#define BUTTON_DEBOUNCE			200
#define BUTTON_SHORT_PRESS		1000
#define BUTTON_MEDIUM_PRESS		2000
#define BUTTON_LONG_PRESS		5000
#define GOAL_TIME_STEP			60
#define MAX_TIME_UV				3600
#define DEFAULT_PROG_INDEX		5
// LEDs debug
#define LED_RED_ON				P1OUT |= BIT0;
#define LED_RED_BLINK			P1OUT ^= BIT0;
#define LED_RED_OFF				P1OUT &= ~BIT0;
#define LED_GREEN_ON			P1OUT |= BIT6;
#define LED_GREEN_OFF			P1OUT &= ~BIT6;

unsigned long int ms = 0;
unsigned long int s = 0, m = 0;
unsigned long int button_millis = 0;
char button_pressed = 0;
unsigned char mem_flash[10]={5,7,9,10,0,3,0,0,0,0};
unsigned long int last_digit_millis = 0;
unsigned long int goal = 0;
unsigned long int init_time = 0;
char refresh_display = 0;
char op_mode = 0;
char heart_beat = 0;
unsigned char prog = 0;


void init_io(void);
void init_timerA(void);
unsigned long int millis(void);
void init_wdt(void);


int main()
{

	init_wdt();

	init_timerA();

	flash_init();
	//write_SegC(mem_flash,10);
	read_SegC(mem_flash, 10, 0);

	prog = mem_flash[DEFAULT_PROG_INDEX];

	init_io();

	goal = mem_flash[prog] * 60;

	// interrupts enabled
	__bis_SR_register(GIE);

	while(1)
	{

		if(refresh_display)
		{
			if(op_mode == 0)
			{
				drive_display(goal, 1, 1);
			}
			else if(op_mode == 1)
			{
				long int disp = goal - ((millis() / 1000) - init_time);
				if(disp >=  0)
				{
					drive_display(goal - ((millis() / 1000) - init_time), heart_beat, heart_beat);
				}
				else
				{
					op_mode = 0;
				}
			}
			else if(op_mode == 2 || op_mode == 3)
			{
				if(heart_beat)
				{
					drive_display(0xF, prog, mem_flash[prog] / 10, mem_flash[prog] % 10, 1, 0);
				}
				else
				{
					drive_display(0xF, 0xF, mem_flash[prog] / 10, mem_flash[prog] % 10, 1, 0);
				}
			}
			else if(op_mode == 4)
			{
				if(heart_beat)
				{
					drive_display(0xF, prog, mem_flash[prog] / 10, mem_flash[prog] % 10, 0, 1);
				}
				else
				{
					drive_display(0xF, prog, 0xF, 0xF, 0, 1);
				}
			}
			refresh_display = 0;
		}


		if(button_pressed == 3)
		{
			if((P1IN & BIT3) && (P1IN & BIT4))
			{
				if(op_mode == 0)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(op_mode != 1)
						{
							op_mode = 1;
							init_time = millis() / 1000;
						}
						else
						{
							op_mode = 0;
						}
					}
					else if(millis() - button_millis > BUTTON_SHORT_PRESS &&
							millis() - button_millis <= BUTTON_MEDIUM_PRESS)
					{
						button_pressed = 0;
					}
					else if(millis() - button_millis > BUTTON_MEDIUM_PRESS)
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 2)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						op_mode = 1;
						init_time = millis() / 1000;
						goal = mem_flash[prog] * 60;
					}
					else
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 3)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						op_mode = 4;
					}
					else
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 4)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;
						mem_flash[DEFAULT_PROG_INDEX] = prog;
						write_SegC(mem_flash, 10);
						op_mode = 0;
					}
					else
					{
						button_pressed = 0;
					}
				}

			}
		}
		else if(button_pressed == 2)
		{
			if((P1IN & BIT4))
			{
				if(op_mode == 0)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(goal >= GOAL_TIME_STEP)
						{
							goal -= GOAL_TIME_STEP;
						}
					}
					else if(millis() - button_millis > BUTTON_SHORT_PRESS &&
							millis() - button_millis <= BUTTON_MEDIUM_PRESS)
					{
						button_millis = 0;

						op_mode = 3;
					}
					else if(millis() - button_millis > BUTTON_MEDIUM_PRESS)
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 2 || op_mode == 3)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(prog > 0)
						{
							prog--;
						}
					}
					else
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 4)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(mem_flash[prog] >= GOAL_TIME_STEP/60)
						{
							mem_flash[prog] -= GOAL_TIME_STEP/60;
						}
					}
					else
					{
						button_pressed = 0;
					}
				}
			}
		}
		else if(button_pressed == 1)
		{
			if((P1IN & BIT3))
			{
				if(op_mode == 0)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(goal < MAX_TIME_UV)
						{
							goal += GOAL_TIME_STEP;
						}
					}
					else if(millis() - button_millis > BUTTON_SHORT_PRESS &&
							millis() - button_millis <= BUTTON_MEDIUM_PRESS)
					{
						button_millis = 0;
						op_mode = 2;
					}
					else if(millis() - button_millis > BUTTON_MEDIUM_PRESS)
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 2 || op_mode == 3)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(prog < 5)
						{
							prog++;
						}
					}
					else
					{
						button_pressed = 0;
					}
				}
				else if(op_mode == 4)
				{
					if(millis() - button_millis > BUTTON_DEBOUNCE &&
							millis() - button_millis <= BUTTON_SHORT_PRESS)
					{
						button_pressed = 0;

						if(mem_flash[prog] <= MAX_TIME_UV)
						{
							mem_flash[prog] += GOAL_TIME_STEP/60;
						}
					}
					else
					{
						button_pressed = 0;
					}
				}
			}
		}

		if(op_mode)
		{
			LED_GREEN_ON;
		}
		else if(op_mode == 0)
		{
			LED_GREEN_OFF;
		}
	}
}

void init_io(void)
{
	// config outputs
	P1DIR |= 0x41 + BIT7;
	P1OUT = 0;
	P2DIR |= 0xFF;
	P2SEL = 0;
	P2OUT = 0;
	// config inputs
	P1OUT |=  BIT3 + BIT4;                           // P1.3 set, else reset
	P1REN |= BIT3 + BIT4;                            // P1.3 pullup
	P1IE |= BIT3 + BIT4;                             // P1.3 interrupt enabled
	P1IES |= BIT3 + BIT4;                            // P1.3 Hi/lo edge
	P1IFG &= ~(BIT3 + BIT4);                           // P1.3 IFG cleared
}

void init_timerA(void)
{
	BCSCTL1 = CALBC1_8MHZ;            			// Set DCO to 8MHz
	DCOCTL = CALDCO_8MHZ;
	CCR0 = 8000;
	TACTL = TASSEL_2 + MC_1;
	CCTL0 = CCIE;                            // CCR0 interrupt enabled
}

void init_wdt(void)
{
	WDTCTL = WDT_MDLY_8;                     // Set Watchdog Timer interval to ~30ms
	IE1 |= WDTIE;                             // Enable WDT interrupt
}

unsigned long int millis(void)
{
	return ms + (s * 1000) + (60000 * m);
}

//===========================================================================
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	ms++;

	if(ms >= 1000)
	{
		ms = 0;
		s++;
	}
	if(s >= 60)
	{
		s = 0;
		m++;
	}

	if(ms > 500)
	{
		heart_beat = 0;
	}
	else
	{
		heart_beat = 1;
	}

}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(!(P1IN & (BIT3 + BIT4)))
	{
		button_millis = millis();
		button_pressed = 3;
		P1OUT ^= BIT0;
	}
	else if(!(P1IN & BIT3) && button_pressed != 3)
	{
		button_millis = millis();
		button_pressed = 1;
	}
	else if(!(P1IN & BIT4) && button_pressed != 3)
	{
		button_millis = millis();
		button_pressed = 2;
	}


	P1IFG &= ~(BIT3 + BIT4);                           // P1.3 IFG cleared
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
	refresh_display = 1;
}
