/*  Arduino UNO R4 test code for fast ADC and digital pin operation
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

// RA4M1 User’s Manual: Hardware
// This doc has all the register discriptions I use:
// https://www.renesas.com/us/en/document/mah/renesas-ra4m1-group-users-manual-hardware

/* For external aref - ADR4540 - Ultralow Noise, High Accuracy Voltage Reference
   Using an aref gives c. +- 1 to 2 value in 14 bit reads on USB power
https://www.analog.com/media/en/technical-documentation/data-sheets/adr4520_4525_4530_4533_4540_4550.pdf
*/
/*
A/D Control Extended Register (ADCER)
b5 ACE A/D Data Register Automatic Clearing Enable
  0: Automatic clearing disabled
  1: Automatic clearing enabled.  << This is set by IDE startup code.
*/


#include "Arduino.h"

// ARM-developer - Accessing memory-mapped peripherals
// https://developer.arm.com/documentation/102618/0100

// =========== ADC14 ============
// 35.2 Register Descriptions

#define ADCBASE 0x40050000 /* ADC Base */

#define ADC140_ADCSR   ((volatile unsigned short *)(ADCBASE + 0xC000)) // A/D Control Register
#define ADC140_ADANSA0 ((volatile unsigned short *)(ADCBASE + 0xC004)) // A/D Channel Select Register A0
#define ADC140_ADANSA1 ((volatile unsigned short *)(ADCBASE + 0xC006)) // A/D Channel Select Register A1
#define ADC140_ADADS0  ((volatile unsigned short *)(ADCBASE + 0xC008)) // A/D-Converted Value Addition/Average Channel Select Register 0
#define ADC140_ADADS1  ((volatile unsigned short *)(ADCBASE + 0xC00A)) // A/D-Converted Value Addition/Average Channel Select Register 1
#define ADC140_ADCER   ((volatile unsigned short *)(ADCBASE + 0xC00E)) // A/D Control Extended Register 
#define ADC140_ADSTRGR ((volatile unsigned short *)(ADCBASE + 0xC010)) // A/D Conversion Start Trigger Select Register
#define ADC140_ADEXICR ((volatile unsigned short *)(ADCBASE + 0xC012)) // A/D Conversion Extended Input Control Register
#define ADC140_ADANSB0 ((volatile unsigned short *)(ADCBASE + 0xC014)) // A/D Channel Select Register B0
#define ADC140_ADANSB1 ((volatile unsigned short *)(ADCBASE + 0xC016)) // A/D Channel Select Register B1
#define ADC140_ADTSDR  ((volatile unsigned short *)(ADCBASE + 0xC01A)) // A/D conversion result of temperature sensor output
#define ADC140_ADOCDR  ((volatile unsigned short *)(ADCBASE + 0xC01C)) // A/D result of internal reference voltage
#define ADC140_ADRD    ((volatile unsigned short *)(ADCBASE + 0xC01E)) // A/D Self-Diagnosis Data Register

#define ADC140_ADDR00 ((volatile unsigned short *)(ADCBASE + 0xC020))      // A1 (P000 AN00 AMP+)
#define ADC140_ADDR01 ((volatile unsigned short *)(ADCBASE + 0xC020 +  2)) // A2 (P001 AN01 AMP-) 
#define ADC140_ADDR02 ((volatile unsigned short *)(ADCBASE + 0xC020 +  4)) // A3 (P002 AN02 AMPO) 
#define ADC140_ADDR05 ((volatile unsigned short *)(ADCBASE + 0xC020 + 10)) // Aref (P010 AN05 VrefH0)
#define ADC140_ADDR09 ((volatile unsigned short *)(ADCBASE + 0xC020 + 18)) // A0 (P014 AN09 DAC)
#define ADC140_ADDR21 ((volatile unsigned short *)(ADCBASE + 0xC040 + 10)) // A4 (P101 AN21 SDA) 
#define ADC140_ADDR22 ((volatile unsigned short *)(ADCBASE + 0xC040 + 12)) // A5 (P100 AN20 SCL) 


// =========== Ports ============
// 19.2.5 Port mn Pin Function Select Register (PmnPFS/PmnPFS_HA/PmnPFS_BY) (m = 0 to 9; n = 00 to 15)

#define PORTBASE 0x40040000 /* Port Base */
#define PFS_P100PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843))   // 8 bits - A5
#define PFS_P101PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 1 * 4))) // A4
#define PFS_P102PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 2 * 4))) // D5
#define PFS_P103PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 3 * 4))) // D4
#define PFS_P104PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 4 * 4))) // D3
#define PFS_P105PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 5 * 4))) // D2
#define PFS_P106PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 6 * 4))) // D6
#define PFS_P107PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 7 * 4))) // D7
#define PFS_P108PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 8 * 4))) // SWDIO
#define PFS_P109PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + ( 9 * 4))) // D11 / MOSI
#define PFS_P110PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (10 * 4))) // D12 / MISO
#define PFS_P111PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (11 * 4))) // D13 / SCLK
#define PFS_P112PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x0843 + (12 * 4))) // D10 / CS
#define PFS_P300PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3))            // SWCLK (P300)
#define PFS_P301PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (01 * 4))) // D0 / RxD (P301)
#define PFS_P302PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (02 * 4))) // D1 / TxD (P302) 
#define PFS_P303PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (03 * 4))) // D9
#define PFS_P304PFS_BY ((volatile unsigned char  *)(PORTBASE + 0x08C3 + (04 * 4))) // D8


int analogPin   = A0;    // analog input pin - just for setup

unsigned int int_val;
unsigned short short_val;
unsigned char char_val; 


void setup()
  {
  Serial.begin(115200);

 // analogReference(AR_EXTERNAL);    // Note the unexpected AR_ prefix for declaration.
  
 // analogReadResolution(10);        // IDE default - Note always read in 14 bit mode - this just scaled returned value
  analogReadResolution(12);          // With external Aref the default conversion is 1 bit accurate
 // analogReadResolution(14);        // This code reads ADC result directly, so this setup not needed
 
  uint16_t value = analogRead(analogPin);  // Do a read to get everthing set-up
  
// Update registers - do NOT use analogRead() after this

  *ADC140_ADCER = 0x06;              // 14 bit mode (already set), clear ACE bit 5

  *ADC140_ADCSR |= (0x01 << 15);     // Start a conversion
  }
  
void loop()
  {
  uint16_t adc_val_16;

  *PFS_P103PFS_BY = 0x05;      // Pulse on D4 to trigger scope 
  *PFS_P103PFS_BY = 0x04;      //  

  *PFS_P107PFS_BY = 0x05;         // digitalWrite(monitorPin, HIGH);   // Digital Pin D7

  adc_val_16 = *ADC140_ADDR09;   // adcValue = analogRead(analogPin); // Internal 16bit register read = c. 123nS 

  *ADC140_ADCSR |= (0x01 << 15);  // Next ADC conversion = write to register c. 300nS

  *PFS_P107PFS_BY = 0x04;         // digitalWrite(monitorPin, LOW);  

   // Set High, read ADC register, set Low = c. 207nS
   // Set High, read ADC register, trigger Conversion, set Low = c. 500nS

  *PFS_P107PFS_BY = 0x05;      // Each Port Output bit clear to set takes c. 82nS 
  *PFS_P107PFS_BY = 0x04;      // Each Port Output bit set to clear takes c. 85nS 
  *PFS_P107PFS_BY = 0x05;      // Set HIGH
  char_val = *PFS_P107PFS_BY;  // Port State Input read - takes about 165nS
  *PFS_P107PFS_BY = 0x04;      // Read plus Set LOW = c. 250nS
    
  Serial.println(adc_val_16);
  
  delay(100);
  }
