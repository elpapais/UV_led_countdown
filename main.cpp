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

unsigned long int ms = 0;
unsigned char s = 0, m = 0;
unsigned int button_millis = 0;
char button_pressed = 0;


void binary_leds(unsigned char i);
void init_io(void);
void init_timerA(void);
unsigned long int millis(void);

int main()
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

	init_timerA();

	flash_init();

	init_io();


	// interrupts enabled
	__bis_SR_register(GIE);
}

void init_io(void)
{
	// config outputs
	P1DIR |= 0x07;

	// config inputs
	P1OUT |=  BIT3;                            // P1.3 set, else reset
	P1REN |= BIT3;                            // P1.3 pullup
	P1IE |= BIT3;                             // P1.3 interrupt enabled
	P1IES |= BIT3;                            // P1.3 Hi/lo edge
	P1IFG &= ~BIT3;                           // P1.3 IFG cleared
}

void init_timerA(void)
{
	BCSCTL1 = CALBC1_8MHZ;            // Set DCO to 8MHz
	DCOCTL = CALDCO_8MHZ;
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 8000;
	TACTL = TASSEL_2 + MC_2;                  // SMCLK, count mode
}

void binary_leds(unsigned char i)
{
	if(i >= 0 && i < 16)
	{
		P1OUT &= ~0x07;
		P1OUT |= i & 0x07;
	}
}

unsigned long int millis(void)
{
	return ms + (s * 1000) + (60000 * m);
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	CCR0 += 8000;
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

}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(P1IN & BIT3)
	{
		button_millis = millis();
	}

	P1IFG &= ~BIT3;                           // P1.4 IFG cleared
}

