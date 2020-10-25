/*
 * watchdog.c
 *
 *  Created on: 26 mars 2020
 *      Author: Christophe
 */
#include "Arduino.h"

#include <avr/wdt.h>        // Supplied Watch Dog Timer Macros
#include <avr/sleep.h>      // Supplied AVR Sleep Macros
#include <avr/interrupt.h>

#include "watchdog.h"

#ifdef __AVR_ATtiny85__
#define WDTCSR WDTCR
#endif

/**
 * Set system into the sleep state...
 * System wakes up when watchdog is timed out.
 */
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System actually sleeps here

  // System continues execution here when watchdog timed out
  sleep_disable();
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
}

/**
 * Setup the WDT (8 seconde)
 */
void setup_watchdog() {
  // Clear the reset flag
  MCUSR &= ~(1 << WDRF);

  // In order to change WDE or the prescaler, we need to
  // set WDCE (This will allow updates for 4 clock cycles).
  WDTCSR |= (1 << WDCE) | (1 << WDE);

  // set new watchdog timeout prescaler value
  WDTCSR = 1 << WDP0 | 1 << WDP3; /* 8.0 seconds */

  // Enable the WD interrupt (note no reset)
  WDTCSR |= _BV(WDIE);
}




