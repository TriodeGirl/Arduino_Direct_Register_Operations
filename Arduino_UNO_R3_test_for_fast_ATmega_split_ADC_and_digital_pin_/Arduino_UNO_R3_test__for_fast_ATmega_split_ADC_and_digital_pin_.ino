/*  Arduino UNO R3 test code for fast ATmega split ADC and digital pin operation
 *  Susan Parker - 10th June 2023.
 *
 * This code is "AS IS" without warranty or liability. 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
/* For external aref - ADR4540 - Ultralow Noise, High Accuracy Voltage Reference
   Using an aref gives stable 10bit values on USB power
https://www.analog.com/media/en/technical-documentation/data-sheets/adr4520_4525_4530_4533_4540_4550.pdf
*/


// Direct register bit set/clear/test/wait macros (found code):
#define _BV(bit) (1 << (bit))                                             // Convert a bit number into a byte value.
#define  clr_bit(sfr, bit)  (_SFR_BYTE(sfr) &= ~_BV(bit))
#define  set_bit(sfr, bit)  (_SFR_BYTE(sfr) |=  _BV(bit))
#define  is_bit_set(sfr, bit)    (_SFR_BYTE(sfr) & _BV(bit))              // bit_is_set ?
#define  is_bit_clr(sfr, bit)  (!(_SFR_BYTE(sfr) & _BV(bit)))             // bit_is_clear ?
#define  loop_until_bit_set(sfr, bit)  do{}while(is_bit_clr(sfr, bit))    // loop_until_bit_is_set
#define  loop_until_bit_clr(sfr, bit)  do{}while(is_bit_set(sfr, bit))    // loop_until_bit_is_clear

#include <avr/wdt.h>          // Enable WatchDog timer - use minimum safe timeout to stop runaway charge cycle
#include <stdlib.h>
#include <Arduino.h>
#include "avr/pgmspace.h"

#define SERIAL_BAUD    (115200)      // Serial COM port speed

static unsigned int analogAdcValue = 0;

void setup(void)
  {
  byte temp_reg = 0;
  Serial.begin(SERIAL_BAUD);      // connect to the serial port
  set_bit(DDRD, 4); // set Port D Pin 4 as output = Arduino Pin 4
  setup_adc();
  }

void loop()  // Background tasks 
  {
  run_adc();
  Serial.println(analogAdcValue);
  delay(100);
  }

void run_adc(void)  // This code would go inside an interrupt call e.g. PWM timer
  {
  set_bit(PORTD, 4);                 // set Digital Pin 4 high - to observe timing with a scope
  analogAdcValue = (int)ADCL + ((int)ADCH << 8);  // read result from previous ADC conversion
  ADCSRA |= (1 << ADSC);                          // start new ADC measurement (single shot)
  // Other code would go here
  clr_bit(PORTD, 4);                 // clr Digital Pin 4 - place at last line of interupt

  set_bit(PORTD, 4);    // Extra flick to show fast port bit operation on 'scope.
  clr_bit(PORTD, 4);    //   "     "   outside of ADC operation
  }
  
void setup_adc(void)  // ADC conversion takes 13 (prescaled) clock cycles
  {
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX = 0;              // set A0 analog input pin
  DIDR0 = 0x01;           // Digital Input Disable Register 0 for Analog pin 0 
  ADMUX |= (1 << REFS0);  // Internal reference - Comment out with external Aref 
  // ADCSRA |= (1 << ADPS1) | (1 << ADPS0);                   //   8 prescaler (equiv 153.8 KHz)
  // ADCSRA |= (1 << ADPS2);                                  //  16 prescaler (equiv  76.9 KHz)
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0);                   // 32 prescaler (equiv  38.5 KHz)
  // ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescaler (equiv 8 KHz)
  ADCSRA |= (1 << ADEN);  // enable ADC
  ADCSRA |= (1 << ADSC);  // start ADC measurements
  }

/* End Code */
