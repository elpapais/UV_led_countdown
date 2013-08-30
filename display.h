/*
 * display.h
 *
 *  Created on: 30/08/2013
 *      Author: João Sousa
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define TWO_POINT_PIN			BIT7


void binary_leds(unsigned char i);
void drive_display(unsigned int n, char point_up, char point_down);
void select_digit(char i);
unsigned int sec2sec_display(unsigned int s);
unsigned int sec2min_display(unsigned int s);
void drive_display(unsigned short l, unsigned short r, char point_up, char point_down);


#endif /* DISPLAY_H_ */
