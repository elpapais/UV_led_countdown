/*
 * display.cpp
 *
 *  Created on: 30/08/2013
 *      Author: João Sousa
 */

#include "display.h"
#include <msp430.h>

void drive_display(unsigned int n, char point_up, char point_down)
{
	static char digit = 1;

	binary_leds(15);
	P1OUT |= TWO_POINT_PIN;
	select_digit(digit);
	unsigned int temp;

	switch(digit)
	{

	case 1:
		temp = sec2min_display(n);
		temp /= 10;
		binary_leds(temp);
		break;
	case 2:
		temp = sec2min_display(n);
		binary_leds(temp % 10);

		if(point_up)
		{
			P1OUT &= ~TWO_POINT_PIN;
		}
		break;
	case 3:

		temp = sec2sec_display(n);
		temp /= 10;
		binary_leds(temp % 10);

		if(point_down)
		{
			P1OUT &= ~TWO_POINT_PIN;
		}
		break;
	case 4:
		temp = sec2sec_display(n);
		binary_leds(temp % 10);
		break;
	}
	digit++;
	if(digit > 4) digit = 1;

}

void drive_display(unsigned short l, unsigned short r, char point_up, char point_down)
{
	static char digit = 1;

	binary_leds(15);
	P1OUT |= TWO_POINT_PIN;
	select_digit(digit);
	unsigned int temp;

	switch(digit)
	{

	case 1:
		binary_leds((l>>8));
		break;
	case 2:
		binary_leds(l & 0x0F);

		if(point_up)
		{
			P1OUT &= ~TWO_POINT_PIN;
		}
		break;
	case 3:

		binary_leds(r >> 8);

		if(point_down)
		{
			P1OUT &= ~TWO_POINT_PIN;
		}
		break;
	case 4:
		binary_leds(r & 0x0F);
		break;
	}
	digit++;
	if(digit > 4) digit = 1;

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

unsigned int sec2sec_display(unsigned int s)
{
	return s % 60;
}

unsigned int sec2min_display(unsigned int s)
{
	return s / 60;
}

void binary_leds(unsigned char i)
{
	if(i >= 0 && i < 16)
	{
		P2OUT &= ~0x0F;
		P2OUT |= i & 0x0F;
	}
}

