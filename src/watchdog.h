/*
 * watchdog.h
 *
 *  Created on: 26 mars 2020
 *      Author: Christophe
 */
#ifndef WATCHDOG_H_
#define WATCHDOG_H_

// Routines to clear and set bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup_watchdog();

void system_sleep();


#endif /* WATCHDOG_H_ */
