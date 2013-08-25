/*
 * main.cpp
 *
 *  Created on: 18/08/2013
 *      Author: Jo�o
 */

#include <msp430.h>
#include "msp430g2231-calibration.h"
#include "flash.h"

#define BUTTON_DEBOUNCE			100
#define BUTTON_SHORT_PRESS		1000
#define BUTTON_MEDIUM_PRESS		2000
#define BUTTON_LONG_PRESS		5000

// LEDs debug
#define LED_RED_ON				P1OUT |= BIT0;
#define LED_RED_BLINK			P1OUT ^= BIT0;
#define LED_RED_OFF				P1OUT &= ~BIT0;
#define LED_GREEN_ON			P1OUT |= BIT6;
#define LED_GREEN_OFF			P1OUT &= ~BIT6;

unsigned long int ms = 0;
unsigned char s = 0, m = 0;
unsigned int button_millis = 0;
char button_pressed = 0;
unsigned char mem_flash[10];
unsigned long int last_digit_millis = 0;
char refresh_display = 0;

void binary_leds(unsigned char i);
void init_io(void);
void init_timerA(void);
unsigned long int millis(void);
void drive_display(unsigned int n);
void select_digit(char i);
void init_wdt(void);

int main()
{
	init_wdt();

	init_timerA();

	flash_init();

	init_io();

	for(int i = 0; i < 10; i++) mem_flash[i] = 0;

	// interrupts enabled
	__bis_SR_register(GIE);

	while(1)
	{
		if(refresh_display)
		{
			drive_display(millis()/ 1000);
			refresh_display = 0;
		}
	}
}

void init_io(void)
{
	// config outputs
	P1DIR |= 0x47;
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

void binary_leds(unsigned char i)
{
	if(i >= 0 && i < 10)
	{
		P2OUT &= ~0x0F;
		P2OUT |= i & 0x0F;
	}
}

unsigned long int millis(void)
{
	return ms + (s * 1000) + (60000 * m);
}

void drive_display(unsigned int n)
{
	static char digit = 0;
	unsigned int temp = n;

	select_digit(digit);
	switch(digit++)
	{

	case 1:
		temp /= 1000;
		binary_leds(temp);
		break;
	case 2:
		temp /= 100;
		binary_leds(temp % 10);

		break;
	case 3:
		temp /= 10;
		binary_leds(temp % 10);
		break;
	case 4:
		binary_leds(temp % 10);
		digit = 0;
		break;
	}

}

void select_digit(char i)
{
	switch(i)
	{
	case 1:
		P2OUT |= 0xF0;
		P2OUT &= ~BIT4;
		break;

	case 2:
		P2OUT |= 0xF0;
		P2OUT &= ~BIT5;
		break;

	case 3:
		P2OUT |= 0xF0;
		P2OUT &= ~BIT6;
		break;

	case 4:
		P2OUT |= 0xF0;
		P2OUT &= ~BIT7;
		break;
	}
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

//	if(button_pressed)
//	{
//		if((P1IN & (BIT3 + BIT4)) == (BIT3 + BIT4))
//		{
//			if(button_pressed == 1)
//			{
//				if(mem_flash[0] < 9)
//				mem_flash[0]++;
//			}
//			else if(button_pressed == 2)
//			{
//				if(mem_flash[0] > 0)
//				mem_flash[0]--;
//			}
//			button_pressed = 0;
//			binary_leds(mem_flash[0]);
//		}
//	}

}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(!(P1IN & BIT3))
	{
		button_millis = millis();
		button_pressed = 1;
	}
	if(!(P1IN & BIT4))
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
